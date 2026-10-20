// MillingToolPropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "MillingToolPropertiesDialog.h"
#include "MainFrm.h"


#include "millingtoolball.h"
#include "millingtoolcylinder.h"
#include "MillingToolToroidal.h"
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
	,m_nHolderIndex(0)
	,m_pOldTool(0)
	,m_pDoc(0)
{

}

//The shape the combo box stands for, and what R means for it: the ball of a ball nose tool, the
//rounding of the bottom edge of a toroidal one, nothing at all for a flat end mill.
MILLING_TOOL_TYPE CMillingToolPropertiesDialog::ToolType() const
{
	switch(m_nToolShapeIndex){
	case 1:		return MILLING_TOOL_CYLINDER;
	case 2:		return MILLING_TOOL_TORUS;
	default:	return MILLING_TOOL_BALL;
	}
}

std::string CMillingToolPropertiesDialog::ToolDimensionProblem() const
{
	if(m_nToolShapeIndex<0 || m_nToolShapeIndex>2)
		return "";		//no shape to build: the dimensions are nobody's business yet
	double dR = (ToolType()==MILLING_TOOL_CYLINDER) ? 0.0 : m_dR;
	return CMillingTool::DimensionProblem(ToolType(),m_dRz,dR,m_dCuttingPartEnd
		,m_dReferencePointHeight,m_dNonCuttingLength);
}

CToolHolder CMillingToolPropertiesDialog::HolderFromDialog() const
{
	if(m_nHolderIndex<=0)
		return CToolHolder(m_dHolderLength,m_dHolderRadius);	//the two fields, as one cylinder
	const CToolHolder oCatalogue = ToolHolderFromCatalogue((size_t)(m_nHolderIndex-1));
	//the catalogue is in metres and the dialog in the unit of the chain, so the sections are scaled
	const double dFactor = m_pDoc ? 1000.0/m_pDoc->m_oChain.GetUnitInMillimetres() : 1.0;
	if(dFactor==1.0)
		return oCatalogue;
	std::vector<CToolHolderSection> vSections = oCatalogue.GetSections();
	for(size_t i=0;i<vSections.size();i++){
		vSections[i].SetLength(vSections[i].GetLength()*dFactor);
		vSections[i].SetRadius(vSections[i].GetRadius()*dFactor);
	}
	return CToolHolder(oCatalogue.GetModel(),vSections);
}

void CMillingToolPropertiesDialog::FillTheLists()
{
	m_comboShape.ResetContent();
	m_comboShape.AddString("ball nose");		//the order of ToolType
	m_comboShape.AddString("flat end");
	m_comboShape.AddString("toroidal");
	m_comboShape.SetCurSel(m_nToolShapeIndex);
	m_comboHolder.ResetContent();
	m_comboHolder.AddString("(the sizes below)");
	for(size_t i=0;i<ToolHolderCatalogueSize();i++)
		m_comboHolder.AddString(ToolHolderFromCatalogue(i).GetModel().c_str());
	m_comboHolder.SetCurSel(m_nHolderIndex);
	EnableHolderFields();
}

void CMillingToolPropertiesDialog::EnableHolderFields()
{
	const BOOL bCustom = (m_nHolderIndex<=0) ? TRUE : FALSE;
	m_editHolderLength.EnableWindow(bCustom);
	m_editHolderRadius.EnableWindow(bCustom);
}

BOOL CMillingToolPropertiesDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	FillTheLists();
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
	return TRUE;
}

void CMillingToolPropertiesDialog::OnCbnSelchangeComboToolHolder()
{
	UpdateData(TRUE);
	//the picked holder is what the two fields now say, so that the numbers of the drawing can be read
	const CToolHolder oHolder = HolderFromDialog();
	m_dHolderLength = oHolder.GetLength();
	m_dHolderRadius = oHolder.GetRadius();
	EnableHolderFields();
	UpdateData(FALSE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);
}

void CMillingToolPropertiesDialog::UpdateMillingTool()
{
	assert(m_pDoc);
	//the programmed point follows the shape of the tool: the centre of the ball, or the tip (KnownBugs #66)
	m_dReferencePointHeight = (ToolType()==MILLING_TOOL_BALL) ? fabs(m_dR) : 0.0;
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
	
	const CToolHolder oHolder = HolderFromDialog();
	switch(m_nToolShapeIndex){
		//ball
		case 0:
			m_pDoc->m_pMillingTool = new CMillingToolBall(m_dRz,m_dR
				,m_dCuttingPartEnd,m_dReferencePointHeight,m_dNonCuttingLength,oHolder);
		break;
		//cylinder
		case 1:
			m_pDoc->m_pMillingTool = new CMillingToolCylinder(m_dRz,m_dR
				,m_dCuttingPartEnd,m_dReferencePointHeight,m_dNonCuttingLength,oHolder);
		break;
		//toroidal: R is the radius the bottom edge is rounded by
		case 2:
			m_pDoc->m_pMillingTool = new CMillingToolToroidal(m_dRz,m_dR
				,m_dCuttingPartEnd,m_dReferencePointHeight,m_dNonCuttingLength,oHolder);
		break;
		default:
			m_editH.EnableWindow(TRUE);
	}
	if(m_pDoc->m_pMillingTool){
		//the tool point hangs the length of the assembly below the tool mount of the machine
		m_dH = m_pDoc->m_pMillingTool->GetAssemblyLength();
		m_pDoc->m_oChain.SetToolOffsetFromTool(*m_pDoc->m_pMillingTool);
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
	DDX_CBIndex(pDX, IDC_COMBO_SHAPE, m_nToolShapeIndex);
	DDX_CBIndex(pDX, IDC_COMBO_TOOL_HOLDER, m_nHolderIndex);
	DDX_Control(pDX, IDC_EDIT_CUTTING_PART_HEIGHT, m_editCuttingPartHeight);
	DDX_Control(pDX, IDC_EDIT_H, m_editH);
	DDX_Control(pDX, IDC_EDIT_R, m_editR);
	DDX_Control(pDX, IDC_COMBO_SHAPE, m_comboShape);
	DDX_Control(pDX, IDC_COMBO_TOOL_HOLDER, m_comboHolder);
	DDX_Control(pDX, IDC_EDIT_HOLDER_LENGTH, m_editHolderLength);
	DDX_Control(pDX, IDC_EDIT_HOLDER_RADIUS, m_editHolderRadius);
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
	ON_CBN_SELCHANGE(IDC_COMBO_TOOL_HOLDER, OnCbnSelchangeComboToolHolder)
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
			//a holder the catalogue knows by name is shown as that entry, the rest as its sizes
			m_nHolderIndex = 0;
			for(size_t i=0;i<ToolHolderCatalogueSize();i++)
				if(!oHolder.GetModel().empty()
					&& oHolder.GetModel()==ToolHolderFromCatalogue(i).GetModel())
					m_nHolderIndex = (int)i+1;
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

void CMillingToolPropertiesDialog::OnCbnSelchangeComboShape()
{
	UpdateData(TRUE);
	switch(m_nToolShapeIndex){
		//ball: the programmed point is the centre of the ball
		case 0:
			m_editH.EnableWindow(FALSE);
			m_editR.EnableWindow(TRUE);
			m_dReferencePointHeight = m_dR;
		break;
		//cylinder: no R, and the tip is the programmed point
		case 1:
			m_editH.EnableWindow(FALSE);
			m_editR.EnableWindow(FALSE);
			m_dReferencePointHeight = 0.0;
		break;
		//toroidal: R rounds the bottom edge, and the tip is still the programmed point
		case 2:
			m_editH.EnableWindow(FALSE);
			m_editR.EnableWindow(TRUE);
			m_dReferencePointHeight = 0.0;
		break;
		default:
			m_editH.EnableWindow(TRUE);
	}
	UpdateData(FALSE);
	UpdateMillingTool();
	m_wndMillingToolPreview.Invalidate(TRUE);

}
