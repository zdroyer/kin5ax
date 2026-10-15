// MillingToolPropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "MillingToolPropertiesDialog.h"
#include "MainFrm.h"


#include "millingtoolball.h"
#include "millingtoolcylinder.h"
#include ".\millingtoolpropertiesdialog.h"



// CMillingToolPropertiesDialog dialog

IMPLEMENT_DYNAMIC(CMillingToolPropertiesDialog, CDialog)
CMillingToolPropertiesDialog::CMillingToolPropertiesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMillingToolPropertiesDialog::IDD, pParent)
	,m_dH(0.0)
	,m_dRz(0.012)
	,m_dR(0.012)
	,m_dCuttingPartEnd(0.040)
	,m_dReferencePointHeight(0.012)
	,m_dNonCuttingLength(0.0)
	,m_dHolderLength(0.0)
	,m_dHolderRadius(0.0)
	,m_nToolShapeIndex(0)
	,m_pOldTool(0)
	,m_pDoc(0)
{

}

//The shape the combo box stands for, and the R that shape uses (a flat end mill has none).
MILLING_TOOL_TYPE CMillingToolPropertiesDialog::ToolType() const
{
	return (m_nToolShapeIndex==0) ? MILLING_TOOL_BALL : MILLING_TOOL_CYLINDER;
}

std::string CMillingToolPropertiesDialog::ToolDimensionProblem() const
{
	if(m_nToolShapeIndex!=0 && m_nToolShapeIndex!=1)
		return "";		//no shape to build: the dimensions are nobody's business yet
	double dR = (ToolType()==MILLING_TOOL_BALL) ? m_dR : 0.0;
	return CMillingTool::DimensionProblem(ToolType(),m_dRz,dR,m_dCuttingPartEnd
		,m_dReferencePointHeight,m_dNonCuttingLength);
}

//The holder the fields describe. A name the catalogue knows is the whole holder - three cylinders
//and all - because a host that reads the file later resolves the same name to the same solid. Any
//other name is a holder of this workshop, and then its shape is the single cylinder of the two
//fields, with the name carried along so that the file says what it was.
CToolHolder CMillingToolPropertiesDialog::HolderFromDialog() const
{
	const std::string sModel((LPCTSTR)m_sHolderModel);
	const CToolHolder oCatalogue = ToolHolderByModel(sModel);
	if(!oCatalogue.IsEmpty())
		return oCatalogue;
	CToolHolder oHolder(m_dHolderLength,m_dHolderRadius);
	oHolder.SetModel(sModel);
	return oHolder;
}

void CMillingToolPropertiesDialog::UpdateHolderFields()
{
	const bool bFromCatalogue = !ToolHolderByModel(std::string((LPCTSTR)m_sHolderModel)).IsEmpty();
	if(bFromCatalogue){
		//the sizes are the catalogue's: shown, because they are what collides, and not typed over
		const CToolHolder oHolder = ToolHolderByModel(std::string((LPCTSTR)m_sHolderModel));
		m_dHolderLength = oHolder.GetLength();
		m_dHolderRadius = oHolder.GetRadius();
	}
	if(m_editHolderLength.GetSafeHwnd())
		m_editHolderLength.SetReadOnly(bFromCatalogue);
	if(m_editHolderRadius.GetSafeHwnd())
		m_editHolderRadius.SetReadOnly(bFromCatalogue);
}

void CMillingToolPropertiesDialog::UpdateMillingTool()
{
	assert(m_pDoc);
	//the programmed point follows the shape of the tool: the centre of the ball, or the tip (KnownBugs #66)
	m_dReferencePointHeight = (m_nToolShapeIndex==0) ? fabs(m_dR) : 0.0;
	//the fields are read on every keystroke, so a half typed dimension is normal and the tools refuse
	//dimensions that are not a tool (KnownBugs #50): the document keeps the tool it has until the fields
	//make one again, and Apply says what is wrong with them
	if(ToolDimensionProblem().size()){
		UpdateData(FALSE);
		return;
	}
	((CMainFrame*)AfxGetMainWnd())->StopAndResetSimulation();	//the simulation refers to the tool being replaced
	m_wndMillingToolPreview.SetMillingTool(0);
	if(m_pDoc->m_pMillingTool != m_pOldTool)
		delete m_pDoc->m_pMillingTool;
	
	UpdateHolderFields();
	CToolHolder oHolder = HolderFromDialog();
	//m_dH is the gauge length: from the tip to the tool mount of the machine, which is where the end
	//of the tool branch of every chain is. 0 means nobody measured the tool, and then it stands right
	//up against its holder and the tool works the length out.
	switch(m_nToolShapeIndex){
		//ball
		case 0:
			m_pDoc->m_pMillingTool = new CMillingToolBall(m_dRz,m_dR
				,m_dCuttingPartEnd,m_dReferencePointHeight,m_dNonCuttingLength,oHolder,m_dH);
		break;
		//cylinder
		case 1:
			m_pDoc->m_pMillingTool = new CMillingToolCylinder(m_dRz,m_dR
				,m_dCuttingPartEnd,m_dReferencePointHeight,m_dNonCuttingLength,oHolder,m_dH);
		break;
		default:
			m_editH.EnableWindow(TRUE);
	}
	if(m_pDoc->m_pMillingTool){
		//the tool point hangs the gauge length below the tool mount, less the height of the
		//programmed point above the tip; a 0 in the field comes back as the length it stands for
		m_dH = m_pDoc->m_pMillingTool->GetGaugeLength();
		m_pDoc->m_oChain.SetToolAssembly(*m_pDoc->m_pMillingTool);
	}
	UpdateData(FALSE);

	m_wndMillingToolPreview.SetMillingTool(m_pDoc->m_pMillingTool);
}

CMillingToolPropertiesDialog::~CMillingToolPropertiesDialog()
{
}

void CMillingToolPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICTURE_TOOL_PREVIEW, m_wndMillingToolPreview);

	DDX_Text(pDX, IDC_EDIT_H, m_dH);
	DDX_Text(pDX, IDC_EDIT_R, m_dR);
	DDX_Text(pDX, IDC_EDIT_RZ, m_dRz);
	DDX_Text(pDX, IDC_EDIT_CUTTING_PART_HEIGHT, m_dCuttingPartEnd);
	DDX_Text(pDX, IDC_EDIT_REFERENCE_POINT_HEIGHT,m_dReferencePointHeight);
	DDX_Text(pDX, IDC_EDIT_NON_CUTTING_LENGTH, m_dNonCuttingLength);
	DDX_Text(pDX, IDC_EDIT_HOLDER_LENGTH, m_dHolderLength);
	DDX_Text(pDX, IDC_EDIT_HOLDER_RADIUS, m_dHolderRadius);
	DDX_Control(pDX, IDC_EDIT_HOLDER_LENGTH, m_editHolderLength);
	DDX_Control(pDX, IDC_EDIT_HOLDER_RADIUS, m_editHolderRadius);
	DDX_Control(pDX, IDC_COMBO_HOLDER_MODEL, m_comboHolderModel);
	DDX_CBString(pDX, IDC_COMBO_HOLDER_MODEL, m_sHolderModel);
	DDX_CBIndex(pDX, IDC_COMBO_SHAPE, m_nToolShapeIndex);
	DDX_Control(pDX, IDC_EDIT_CUTTING_PART_HEIGHT, m_editCuttingPartHeight);
	DDX_Control(pDX, IDC_EDIT_H, m_editH);
	DDX_Control(pDX, IDC_EDIT_R, m_editR);
}


BEGIN_MESSAGE_MAP(CMillingToolPropertiesDialog, CDialog)
	ON_EN_CHANGE(IDC_EDIT_RZ, OnEnChangeEditRz)
	ON_EN_CHANGE(IDC_EDIT_R, OnEnChangeEditR)
	ON_EN_CHANGE(IDC_EDIT_H, OnEnChangeEditH)
	ON_EN_CHANGE(IDC_EDIT_CUTTING_PART_HEIGHT, OnEnChangeEditCuttingPartHeight)
	ON_EN_CHANGE(IDC_EDIT_REFERENCE_POINT_HEIGHT, OnEnChangeEditReferencePointHeight)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnBnClickedButtonApply)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO_SHAPE, OnCbnSelchangeComboShape)
	ON_CBN_SELCHANGE(IDC_COMBO_HOLDER_MODEL, OnCbnSelchangeComboHolderModel)
	ON_CBN_EDITCHANGE(IDC_COMBO_HOLDER_MODEL, OnCbnEditchangeComboHolderModel)
	ON_EN_CHANGE(IDC_EDIT_HOLDER_LENGTH, OnEnChangeEditHolderLength)
	ON_EN_CHANGE(IDC_EDIT_HOLDER_RADIUS, OnEnChangeEditHolderRadius)
END_MESSAGE_MAP()


// CMillingToolPropertiesDialog message handlers

void CMillingToolPropertiesDialog::OnEnChangeEditRz()
{
	UpdateData(TRUE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
}

void CMillingToolPropertiesDialog::OnEnChangeEditR()
{
	UpdateData(TRUE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
}

void CMillingToolPropertiesDialog::OnEnChangeEditH()
{
	UpdateData(TRUE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
}

void CMillingToolPropertiesDialog::OnEnChangeEditCuttingPartHeight()
{
	UpdateData(TRUE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);

}

void CMillingToolPropertiesDialog::OnEnChangeEditReferencePointHeight()
{
	UpdateData(TRUE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
}

void CMillingToolPropertiesDialog::OnBnClickedButtonApply()
{
	std::string sProblem = ToolDimensionProblem();
	if(sProblem.size()){
		AfxMessageBox((std::string("The dimensions in the dialog do not make a milling tool:\n\n")
			+sProblem+".\n\nThe tool of the document is unchanged.").c_str());
		return;
	}
	if(m_pOldTool != m_pDoc->m_pMillingTool)
		delete m_pOldTool;
	m_pDoc->UpdateAllViews(NULL);
	OnOK();
}

void CMillingToolPropertiesDialog::SetActiveDocument(CKin5axChainEditorPlusDoc *pDoc)
{
	assert(pDoc);
	m_pDoc = pDoc;
	if(m_pDoc){
		m_pOldTool = m_pDoc->m_pMillingTool;
		if(m_pOldTool){
			if(m_pOldTool->m_type==MILLING_TOOL_BALL)
				m_nToolShapeIndex=0;
			else
				m_nToolShapeIndex=1;
			m_dCuttingPartEnd = m_pOldTool->m_dCuttingHeight;
			m_dH = m_pOldTool->GetAssemblyLength();
			
			m_dR = m_pOldTool->m_dR;
			m_dReferencePointHeight = m_pOldTool->m_dReferencePointZ;
			m_dRz = m_pOldTool->m_dRz;
			m_dNonCuttingLength = m_pOldTool->m_dNonCuttingLength;
			//a tool without a holder of its own takes the one the chain file declares
			CToolHolder oHolder = m_pOldTool->m_oHolder;
			if(!oHolder.HasBody() && m_pDoc->m_oChain.HasDefaultToolHolder())
				oHolder = m_pDoc->m_oChain.GetDefaultToolHolder();
			m_dHolderLength = oHolder.GetLength();
			m_dHolderRadius = oHolder.GetRadius();
			m_sHolderModel = oHolder.GetModel().c_str();
			m_dH = m_pOldTool->GetGaugeLength();
		}
	}

}
void CMillingToolPropertiesDialog::OnBnClickedCancel()
{
	if(m_pOldTool != m_pDoc->m_pMillingTool){
		((CMainFrame*)AfxGetMainWnd())->StopAndResetSimulation();
		delete m_pDoc->m_pMillingTool;
		m_pDoc->m_pMillingTool = m_pOldTool;
	}
	OnCancel();
}

INT_PTR CMillingToolPropertiesDialog::DoModal()
{
	if(!m_pDoc)
		MY_THROW(CGeneratorException,"Document not set:\n CMillingToolPropertiesDialog cannot be displayed");
	UpdateMillingTool();
	return CDialog::DoModal();
}

BOOL CMillingToolPropertiesDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	//the holders a five-axis machine usually carries, to be picked instead of measured; a workshop
	//with holders of its own types their designation in, and then gives the sizes itself
	for(size_t i=0;i<ToolHolderCatalogueSize();i++)
		m_comboHolderModel.AddString(ToolHolderFromCatalogue(i).GetModel().c_str());
	UpdateData(FALSE);		//the list is filled, so the model of the tool goes back into it
	UpdateHolderFields();
	return TRUE;
}

void CMillingToolPropertiesDialog::OnCbnSelchangeComboHolderModel()
{
	//the text of the combo box is not exchanged yet when the selection changes, so take the item
	const int nIndex = m_comboHolderModel.GetCurSel();
	if(nIndex!=CB_ERR)
		m_comboHolderModel.GetLBText(nIndex,m_sHolderModel);
	UpdateHolderFields();
	UpdateData(FALSE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
}

void CMillingToolPropertiesDialog::OnCbnEditchangeComboHolderModel()
{
	UpdateData(TRUE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
}

void CMillingToolPropertiesDialog::OnEnChangeEditHolderLength()
{
	UpdateData(TRUE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
}

void CMillingToolPropertiesDialog::OnEnChangeEditHolderRadius()
{
	UpdateData(TRUE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
}

void CMillingToolPropertiesDialog::OnCbnSelchangeComboShape()
{
	UpdateData(TRUE);
	switch(m_nToolShapeIndex){
		//ball
		case 0:
			m_editR.EnableWindow(TRUE);
			m_dReferencePointHeight = m_dR;
		break;
		//cylinder
		case 1:
			m_editR.EnableWindow(FALSE);
			m_dReferencePointHeight = 0.0;
		break;
		default:
			m_editH.EnableWindow(TRUE);
	}
	UpdateData(FALSE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);

}
