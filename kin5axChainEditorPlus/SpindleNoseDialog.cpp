// SpindleNoseDialog.cpp : the terminal of the tool branch, its holder and its default tool
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "SpindleNoseDialog.h"

IMPLEMENT_DYNAMIC(CSpindleNoseDialog, CDialog)

namespace {

//the shapes of the list, in its own order: the first entry is no tool at all
const int nSHAPE_NONE = 0;
MILLING_TOOL_TYPE ShapeOfIndex(int nIndex)
{
	switch(nIndex){
	case 2:		return MILLING_TOOL_CYLINDER;
	case 3:		return MILLING_TOOL_TORUS;
	default:	return MILLING_TOOL_BALL;
	}
}

int IndexOfShape(MILLING_TOOL_TYPE nType)
{
	switch(nType){
	case MILLING_TOOL_CYLINDER:	return 2;
	case MILLING_TOOL_TORUS:	return 3;
	default:					return 1;
	}
}

} // namespace

CSpindleNoseDialog::CSpindleNoseDialog(CWnd *pParent /*=NULL*/)
	: CDialog(CSpindleNoseDialog::IDD, pParent)
	,m_nHolderIndex(0)
	,m_nShapeIndex(nSHAPE_NONE)
	,m_dRadius(0.0)
	,m_dCornerRadius(0.0)
	,m_dCuttingHeight(0.0)
	,m_dToolLength(0.0)
	,m_nReferenceIndex(TOOL_REFERENCE_TIP)
	,m_dReferenceHeight(0.0)
	,m_pSegment(0)
	,m_dUnitInMillimetres(1000.0)
{
	for(size_t i=0;i<nTOOL_HOLDER_SECTIONS_MAX;i++){
		m_dSectionLength[i] = 0.0;
		m_dSectionRadius[i] = 0.0;
	}
}

CSpindleNoseDialog::~CSpindleNoseDialog()
{
}

void CSpindleNoseDialog::SetTerminal(CChainSegment *pSegment, double dUnitInMillimetres)
{
	m_pSegment = pSegment;
	m_dUnitInMillimetres = dUnitInMillimetres;
	DialogFromTerminal();
}

void CSpindleNoseDialog::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TERMINAL_NAME, m_sName);
	DDX_CBIndex(pDX, IDC_COMBO_NOSE_HOLDER, m_nHolderIndex);
	DDX_Text(pDX, IDC_EDIT_SECTION1_LENGTH, m_dSectionLength[0]);
	DDX_Text(pDX, IDC_EDIT_SECTION1_RADIUS, m_dSectionRadius[0]);
	DDX_Text(pDX, IDC_EDIT_SECTION2_LENGTH, m_dSectionLength[1]);
	DDX_Text(pDX, IDC_EDIT_SECTION2_RADIUS, m_dSectionRadius[1]);
	DDX_Text(pDX, IDC_EDIT_SECTION3_LENGTH, m_dSectionLength[2]);
	DDX_Text(pDX, IDC_EDIT_SECTION3_RADIUS, m_dSectionRadius[2]);
	DDX_CBIndex(pDX, IDC_COMBO_TOOL_SHAPE, m_nShapeIndex);
	DDX_Text(pDX, IDC_EDIT_TOOL_RZ, m_dRadius);
	DDX_Text(pDX, IDC_EDIT_TOOL_R, m_dCornerRadius);
	DDX_Text(pDX, IDC_EDIT_TOOL_CUTTING_HEIGHT, m_dCuttingHeight);
	DDX_Text(pDX, IDC_EDIT_TOOL_LENGTH, m_dToolLength);
	DDX_CBIndex(pDX, IDC_COMBO_TOOL_REFERENCE, m_nReferenceIndex);
	DDX_Text(pDX, IDC_EDIT_TOOL_REFERENCE_HEIGHT, m_dReferenceHeight);
	DDX_Text(pDX, IDC_EDIT_ASSEMBLY_LENGTH, m_sAssemblyLength);
	DDX_Text(pDX, IDC_EDIT_TOOL_OFFSET, m_sToolOffset);
	DDX_Text(pDX, IDC_STATIC_ASSEMBLY_PROBLEM, m_sProblem);
	DDX_Control(pDX, IDC_COMBO_NOSE_HOLDER, m_comboHolder);
	DDX_Control(pDX, IDC_COMBO_TOOL_SHAPE, m_comboShape);
	DDX_Control(pDX, IDC_COMBO_TOOL_REFERENCE, m_comboReference);
	DDX_Control(pDX, IDC_EDIT_TOOL_REFERENCE_HEIGHT, m_editReferenceHeight);
	DDX_Control(pDX, IDC_PICTURE_TOOL_PREVIEW, m_wndPreview);
}

BEGIN_MESSAGE_MAP(CSpindleNoseDialog, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_NOSE_HOLDER, OnCbnSelchangeComboNoseHolder)
	ON_CBN_SELCHANGE(IDC_COMBO_TOOL_SHAPE, OnCbnSelchangeComboToolShape)
	ON_CBN_SELCHANGE(IDC_COMBO_TOOL_REFERENCE, OnCbnSelchangeComboToolReference)
	ON_EN_CHANGE(IDC_EDIT_SECTION1_LENGTH, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_SECTION1_RADIUS, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_SECTION2_LENGTH, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_SECTION2_RADIUS, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_SECTION3_LENGTH, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_SECTION3_RADIUS, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_TOOL_RZ, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_TOOL_R, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_TOOL_CUTTING_HEIGHT, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_TOOL_LENGTH, OnEnChangeAnyDimension)
	ON_EN_CHANGE(IDC_EDIT_TOOL_REFERENCE_HEIGHT, OnEnChangeAnyDimension)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CSpindleNoseDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	//the lists in their own order, which is what the indices above mean
	m_comboHolder.ResetContent();
	m_comboHolder.AddString("(the cylinders below)");
	for(size_t i=0;i<ToolHolderCatalogueSize();i++)
		m_comboHolder.AddString(ToolHolderFromCatalogue(i).GetModel().c_str());
	m_comboHolder.SetCurSel(m_nHolderIndex);

	m_comboShape.ResetContent();
	m_comboShape.AddString("(no tool)");
	m_comboShape.AddString(MillingToolTypeName(MILLING_TOOL_BALL));
	m_comboShape.AddString(MillingToolTypeName(MILLING_TOOL_CYLINDER));
	m_comboShape.AddString(MillingToolTypeName(MILLING_TOOL_TORUS));
	m_comboShape.SetCurSel(m_nShapeIndex);

	m_comboReference.ResetContent();
	m_comboReference.AddString(ToolReferencePointName(TOOL_REFERENCE_TIP));
	m_comboReference.AddString(ToolReferencePointName(TOOL_REFERENCE_BALL_CENTRE));
	m_comboReference.AddString(ToolReferencePointName(TOOL_REFERENCE_CUSTOM));
	m_comboReference.SetCurSel(m_nReferenceIndex);

	EnableFields();
	UpdateTheSums();
	return TRUE;
}

//the holder of the dialog: the cylinders as typed, or the catalogue entry scaled into the unit of
//the chain, because the catalogue is in metres
CToolHolder CSpindleNoseDialog::HolderFromDialog() const
{
	if(m_nHolderIndex>0){
		const CToolHolder oCatalogue = ToolHolderFromCatalogue((size_t)(m_nHolderIndex-1));
		const double dFactor = 1000.0/m_dUnitInMillimetres;
		if(dFactor==1.0)
			return oCatalogue;
		std::vector<CToolHolderSection> vScaled = oCatalogue.GetSections();
		for(size_t i=0;i<vScaled.size();i++){
			vScaled[i].SetLength(vScaled[i].GetLength()*dFactor);
			vScaled[i].SetRadius(vScaled[i].GetRadius()*dFactor);
		}
		return CToolHolder(oCatalogue.GetModel(),vScaled);
	}
	std::vector<CToolHolderSection> vSections;
	for(size_t i=0;i<nTOOL_HOLDER_SECTIONS_MAX;i++)
		vSections.push_back(CToolHolderSection(m_dSectionLength[i],m_dSectionRadius[i]));
	CToolHolder oHolder(szNO_HOLDER,vSections);
	if(!oHolder.GetSections().empty() && m_pSegment)
		oHolder.SetModel(m_pSegment->GetToolHolder().GetModel());	//whatever the file called it
	return oHolder;
}

CToolDescription CSpindleNoseDialog::ToolFromDialog() const
{
	CToolDescription oTool;
	if(m_nShapeIndex==nSHAPE_NONE)
		return oTool;							//nothing in this spindle, which is NO_TOOL
	oTool.m_nType = ShapeOfIndex(m_nShapeIndex);
	oTool.m_dRadius = m_dRadius;
	oTool.m_dCornerRadius = m_dCornerRadius;
	oTool.m_dCuttingHeight = m_dCuttingHeight;
	oTool.m_dLength = m_dToolLength;
	oTool.m_nReference = (TOOL_REFERENCE_POINT)m_nReferenceIndex;
	oTool.m_dReferenceHeight = m_dReferenceHeight;
	oTool.SetGiven(true);
	return oTool;
}

void CSpindleNoseDialog::DialogFromTerminal()
{
	if(!m_pSegment)
		return;
	m_sName = m_pSegment->GetName().c_str();
	const CToolHolder &oHolder = m_pSegment->GetToolHolder();
	//a holder the catalogue knows by name is shown as that entry, any other as its cylinders
	m_nHolderIndex = 0;
	for(size_t i=0;i<ToolHolderCatalogueSize();i++)
		if(!oHolder.GetModel().empty() && oHolder.GetModel()==ToolHolderFromCatalogue(i).GetModel())
			m_nHolderIndex = (int)i+1;
	for(size_t i=0;i<nTOOL_HOLDER_SECTIONS_MAX;i++){
		const bool bHas = i<oHolder.GetSections().size();
		m_dSectionLength[i] = bHas ? oHolder.GetSections()[i].GetLength() : 0.0;
		m_dSectionRadius[i] = bHas ? oHolder.GetSections()[i].GetRadius() : 0.0;
	}
	const CToolDescription &oTool = m_pSegment->GetTool();
	m_nShapeIndex = oTool.IsGiven() ? IndexOfShape(oTool.m_nType) : nSHAPE_NONE;
	m_dRadius = oTool.m_dRadius;
	m_dCornerRadius = oTool.m_dCornerRadius;
	m_dCuttingHeight = oTool.m_dCuttingHeight;
	m_dToolLength = oTool.m_dLength;
	m_nReferenceIndex = (int)oTool.m_nReference;
	m_dReferenceHeight = oTool.m_dReferenceHeight;
}

void CSpindleNoseDialog::EnableFields()
{
	//a holder off the list is shown, not measured: its cylinders are the catalogue's
	const BOOL bCustom = (m_nHolderIndex<=0) ? TRUE : FALSE;
	const int nSectionFields[] = {IDC_EDIT_SECTION1_LENGTH,IDC_EDIT_SECTION1_RADIUS
		,IDC_EDIT_SECTION2_LENGTH,IDC_EDIT_SECTION2_RADIUS
		,IDC_EDIT_SECTION3_LENGTH,IDC_EDIT_SECTION3_RADIUS};
	for(int i=0;i<6;i++)
		if(GetDlgItem(nSectionFields[i]))
			GetDlgItem(nSectionFields[i])->EnableWindow(bCustom);

	const BOOL bTool = (m_nShapeIndex==nSHAPE_NONE) ? FALSE : TRUE;
	const int nToolFields[] = {IDC_EDIT_TOOL_RZ,IDC_EDIT_TOOL_R,IDC_EDIT_TOOL_CUTTING_HEIGHT
		,IDC_EDIT_TOOL_LENGTH};
	for(int i=0;i<4;i++)
		if(GetDlgItem(nToolFields[i]))
			GetDlgItem(nToolFields[i])->EnableWindow(bTool);
	m_comboReference.EnableWindow(bTool);
	//a height above the tip is only asked for when the reference point is given that way
	m_editReferenceHeight.EnableWindow(bTool && m_nReferenceIndex==TOOL_REFERENCE_CUSTOM);
}

void CSpindleNoseDialog::UpdateTheSums()
{
	const CToolAssembly oAssembly(HolderFromDialog(),ToolFromDialog());
	m_sAssemblyLength = double_to_string(oAssembly.GetLength()).c_str();
	m_sToolOffset = double_to_string(oAssembly.GetToolOffset()).c_str();
	const std::string sProblem = oAssembly.Problem();
	m_sProblem = sProblem.empty() ? CString("") : CString(sProblem.c_str());
	UpdateData(FALSE);
	//and the picture of it, which is empty while the fields are not an assembly yet
	m_wndPreview.SetMillingTool(0);
	m_pTool.reset(sProblem.empty() ? oAssembly.MakeTool() : 0);
	m_wndPreview.SetMillingTool(m_pTool.get());
	if(m_wndPreview.GetSafeHwnd())
		m_wndPreview.Invalidate(TRUE);
}

void CSpindleNoseDialog::OnCbnSelchangeComboNoseHolder()
{
	UpdateData(TRUE);
	//the cylinders of the picked holder are shown, so that its sizes can be read off the fields
	const CToolHolder oHolder = HolderFromDialog();
	for(size_t i=0;i<nTOOL_HOLDER_SECTIONS_MAX;i++){
		const bool bHas = i<oHolder.GetSections().size();
		m_dSectionLength[i] = bHas ? oHolder.GetSections()[i].GetLength() : 0.0;
		m_dSectionRadius[i] = bHas ? oHolder.GetSections()[i].GetRadius() : 0.0;
	}
	EnableFields();
	UpdateTheSums();
}

void CSpindleNoseDialog::OnCbnSelchangeComboToolShape()
{
	UpdateData(TRUE);
	//the ball of a ball nose tool is its corner radius, and the centre of it is a place to program
	if(m_nShapeIndex!=nSHAPE_NONE && ShapeOfIndex(m_nShapeIndex)!=MILLING_TOOL_BALL
		&& m_nReferenceIndex==TOOL_REFERENCE_BALL_CENTRE)
		m_nReferenceIndex = TOOL_REFERENCE_TIP;
	m_comboReference.SetCurSel(m_nReferenceIndex);
	EnableFields();
	UpdateTheSums();
}

void CSpindleNoseDialog::OnCbnSelchangeComboToolReference()
{
	UpdateData(TRUE);
	EnableFields();
	UpdateTheSums();
}

void CSpindleNoseDialog::OnEnChangeAnyDimension()
{
	UpdateData(TRUE);
	//the fields are read on every keystroke, so a half typed dimension is normal: the sums and the
	//complaint follow, and nothing is written to the chain until OK
	UpdateTheSums();
}

void CSpindleNoseDialog::OnBnClickedOk()
{
	UpdateData(TRUE);
	const CToolAssembly oAssembly(HolderFromDialog(),ToolFromDialog());
	const std::string sProblem = oAssembly.Problem();
	if(sProblem.size()){
		AfxMessageBox((std::string("The fields do not make a tool assembly:\n\n")+sProblem
			+".\n\nThe spindle nose is unchanged.").c_str());
		return;
	}
	if(m_pSegment){
		m_pSegment->SetName((LPCTSTR)m_sName);
		m_pSegment->SetToolHolder(oAssembly.GetHolder());
		m_pSegment->SetTool(oAssembly.GetTool());
	}
	OnOK();
}
