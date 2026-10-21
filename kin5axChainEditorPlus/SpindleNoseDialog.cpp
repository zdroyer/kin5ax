// SpindleNoseDialog.cpp : the default tool assembly of a machine - the holder hanging from the tool
// spindle and the tool in it, measured in millimetres or in inches
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "SpindleNoseDialog.h"

IMPLEMENT_DYNAMIC(CSpindleNoseDialog, CDialog)

namespace {

//the shapes of the list, in its own order: the first entry is no tool at all
const int nSHAPE_NONE = 0;
const int nSHAPE_BALL = 1;
const int nSHAPE_CYLINDER = 2;
const int nSHAPE_TOROIDAL = 3;

MILLING_TOOL_TYPE ShapeOfIndex(int nIndex)
{
	switch(nIndex){
	case nSHAPE_CYLINDER:	return MILLING_TOOL_CYLINDER;
	case nSHAPE_TOROIDAL:	return MILLING_TOOL_TORUS;
	default:				return MILLING_TOOL_BALL;
	}
}

int IndexOfShape(MILLING_TOOL_TYPE nType)
{
	switch(nType){
	case MILLING_TOOL_CYLINDER:	return nSHAPE_CYLINDER;
	case MILLING_TOOL_TORUS:	return nSHAPE_TOROIDAL;
	default:					return nSHAPE_BALL;
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
	,m_nUnit(CHAIN_UNIT_M)
	,m_dShown(1000.0)
{
	for(size_t i=0;i<nTOOL_HOLDER_SECTIONS_MAX;i++){
		m_dSectionLength[i] = 0.0;
		m_dSectionRadius[i] = 0.0;
	}
}

CSpindleNoseDialog::~CSpindleNoseDialog()
{
}

void CSpindleNoseDialog::SetTerminal(CChainSegment *pSegment, CHAIN_UNIT nUnit)
{
	m_pSegment = pSegment;
	m_nUnit = nUnit;
	//a chain in inches is shown in inches; everything else in millimetres, which is what a holder
	//and a tool are measured in whether the file says metres or millimetres
	m_dShown = (nUnit==CHAIN_UNIT_IN) ? 1.0 : ChainUnitInMillimetres(nUnit);
	DialogFromTerminal();
}

const char * CSpindleNoseDialog::ShownUnitName() const
{
	return (m_nUnit==CHAIN_UNIT_IN) ? "in" : "mm";
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
	m_comboHolder.AddString("Custom - user defined");
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

	//every length field says what it is measured in
	const int nUnits[] = {IDC_STATIC_UNIT_S1,IDC_STATIC_UNIT_S2,IDC_STATIC_UNIT_S3
		,IDC_STATIC_UNIT_RADIUS,IDC_STATIC_UNIT_CORNER,IDC_STATIC_UNIT_CUTTING
		,IDC_STATIC_UNIT_LENGTH,IDC_STATIC_UNIT_REFERENCE,IDC_STATIC_UNIT_ASSEMBLY
		,IDC_STATIC_UNIT_OFFSET};
	for(int i=0;i<10;i++)
		SetDlgItemText(nUnits[i],ShownUnitName());

	EnableFields();
	UpdateTheSums();
	return TRUE;
}

//the holder of the dialog, in the unit of the chain: the cylinders as typed, or the catalogue entry
//scaled out of metres, which is what the catalogue is in
CToolHolder CSpindleNoseDialog::HolderFromDialog() const
{
	if(m_nHolderIndex>0){
		const CToolHolder oCatalogue = ToolHolderFromCatalogue((size_t)(m_nHolderIndex-1));
		const double dFactor = 1000.0/ChainUnitInMillimetres(m_nUnit);
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
		vSections.push_back(CToolHolderSection(ToChain(m_dSectionLength[i])
			,ToChain(m_dSectionRadius[i])));
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
	oTool.m_dRadius = ToChain(m_dRadius);
	oTool.m_dCornerRadius = ToChain(m_dCornerRadius);
	oTool.m_dCuttingHeight = ToChain(m_dCuttingHeight);
	oTool.m_dLength = ToChain(m_dToolLength);
	oTool.m_nReference = (TOOL_REFERENCE_POINT)m_nReferenceIndex;
	oTool.m_dReferenceHeight = ToChain(m_dReferenceHeight);
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
		m_dSectionLength[i] = bHas ? FromChain(oHolder.GetSections()[i].GetLength()) : 0.0;
		m_dSectionRadius[i] = bHas ? FromChain(oHolder.GetSections()[i].GetRadius()) : 0.0;
	}
	const CToolDescription &oTool = m_pSegment->GetTool();
	m_nShapeIndex = oTool.IsGiven() ? IndexOfShape(oTool.m_nType) : nSHAPE_NONE;
	m_dRadius = FromChain(oTool.m_dRadius);
	m_dCornerRadius = FromChain(oTool.m_dCornerRadius);
	m_dCuttingHeight = FromChain(oTool.m_dCuttingHeight);
	m_dToolLength = FromChain(oTool.m_dLength);
	m_nReferenceIndex = (int)oTool.m_nReference;
	m_dReferenceHeight = FromChain(oTool.m_dReferenceHeight);
}

void CSpindleNoseDialog::SyncCornerRadius()
{
	//the ball of a ball nose tool is the tool: a corner radius of its own would be a different
	//shape. A flat end mill has none at all. Only a toroidal one is asked for it.
	if(m_nShapeIndex==nSHAPE_BALL)
		m_dCornerRadius = m_dRadius;
	else if(m_nShapeIndex==nSHAPE_CYLINDER)
		m_dCornerRadius = 0.0;
}

void CSpindleNoseDialog::EnableFields()
{
	//a holder off the list is shown, not measured: its cylinders are the catalogue's
	const BOOL bCustom = (m_nHolderIndex<=0) ? TRUE : FALSE;
	const int nSectionFields[] = {IDC_EDIT_SECTION1_LENGTH,IDC_EDIT_SECTION1_RADIUS
		,IDC_EDIT_SECTION2_LENGTH,IDC_EDIT_SECTION2_RADIUS
		,IDC_EDIT_SECTION3_LENGTH,IDC_EDIT_SECTION3_RADIUS
		,IDC_STATIC_SEC_LEN,IDC_STATIC_SEC_RAD,IDC_STATIC_SEC1,IDC_STATIC_SEC2,IDC_STATIC_SEC3
		,IDC_STATIC_UNIT_S1,IDC_STATIC_UNIT_S2,IDC_STATIC_UNIT_S3};
	for(int i=0;i<14;i++)
		if(GetDlgItem(nSectionFields[i]))
			GetDlgItem(nSectionFields[i])->EnableWindow(bCustom);

	//with nothing in the spindle there is nothing to measure about a tool
	const BOOL bTool = (m_nShapeIndex==nSHAPE_NONE) ? FALSE : TRUE;
	const int nToolFields[] = {IDC_EDIT_TOOL_RZ,IDC_STATIC_LBL_RADIUS,IDC_STATIC_UNIT_RADIUS
		,IDC_EDIT_TOOL_CUTTING_HEIGHT,IDC_STATIC_LBL_CUTTING,IDC_STATIC_UNIT_CUTTING
		,IDC_EDIT_TOOL_LENGTH,IDC_STATIC_LBL_LENGTH,IDC_STATIC_UNIT_LENGTH
		,IDC_STATIC_LBL_PROGRAMMED};
	for(int i=0;i<10;i++)
		if(GetDlgItem(nToolFields[i]))
			GetDlgItem(nToolFields[i])->EnableWindow(bTool);
	m_comboReference.EnableWindow(bTool);

	//the corner radius belongs to a toroidal tool alone: a ball nose one has the whole radius for a
	//ball and a flat end mill has no corner at all, so neither is asked
	const BOOL bCorner = (m_nShapeIndex==nSHAPE_TOROIDAL) ? TRUE : FALSE;
	const int nCornerFields[] = {IDC_EDIT_TOOL_R,IDC_STATIC_LBL_CORNER,IDC_STATIC_UNIT_CORNER};
	for(int i=0;i<3;i++)
		if(GetDlgItem(nCornerFields[i]))
			GetDlgItem(nCornerFields[i])->EnableWindow(bCorner);

	//and a height above the tip is only asked for when the point is given that way
	const BOOL bHeight = (bTool && m_nReferenceIndex==TOOL_REFERENCE_CUSTOM) ? TRUE : FALSE;
	const int nHeightFields[] = {IDC_EDIT_TOOL_REFERENCE_HEIGHT,IDC_STATIC_UNIT_REFERENCE};
	for(int i=0;i<2;i++)
		if(GetDlgItem(nHeightFields[i]))
			GetDlgItem(nHeightFields[i])->EnableWindow(bHeight);
}

void CSpindleNoseDialog::UpdateTheSums()
{
	SyncCornerRadius();
	const CToolAssembly oAssembly(HolderFromDialog(),ToolFromDialog());
	m_sAssemblyLength = double_to_string(FromChain(oAssembly.GetLength())).c_str();
	m_sToolOffset = double_to_string(FromChain(oAssembly.GetToolOffset())).c_str();
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
		m_dSectionLength[i] = bHas ? FromChain(oHolder.GetSections()[i].GetLength()) : 0.0;
		m_dSectionRadius[i] = bHas ? FromChain(oHolder.GetSections()[i].GetRadius()) : 0.0;
	}
	EnableFields();
	UpdateTheSums();
}

void CSpindleNoseDialog::OnCbnSelchangeComboToolShape()
{
	UpdateData(TRUE);
	//the ball of a ball nose tool is its radius, and a flat end mill has no corner: a shape that
	//was just picked says so itself rather than leaving a number nobody can make sense of
	SyncCornerRadius();
	if(m_nShapeIndex==nSHAPE_TOROIDAL && m_dCornerRadius<=0.0)
		m_dCornerRadius = 0.25*m_dRadius;		//a corner to start from, which is a usual one
	//the centre of a ball is a place on a ball nose tool and nowhere else
	if(m_nShapeIndex!=nSHAPE_BALL && m_nReferenceIndex==TOOL_REFERENCE_BALL_CENTRE)
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
	//the fields are read on every keystroke, so a half typed dimension is normal: the sums, the
	//picture and the complaint follow, and nothing is written to the chain until OK
	UpdateTheSums();
}

void CSpindleNoseDialog::OnBnClickedOk()
{
	UpdateData(TRUE);
	SyncCornerRadius();
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
