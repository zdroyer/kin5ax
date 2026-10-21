// TableDialog.cpp : the workpiece holder of a machine, and the solids it puts on the table
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "TableDialog.h"
#include "BoxDialog.h"

IMPLEMENT_DYNAMIC(CTableDialog, CDialog)

namespace {

const char *const szX_VICE = "X_VICE";
const char *const szY_VICE = "Y_VICE";

} // namespace

CTableDialog::CTableDialog(CWnd *pParent /*=NULL*/)
	: CDialog(CTableDialog::IDD, pParent)
	,m_nHolderIndex(0)
	,m_dTrX(0.0)
	,m_dTrY(0.0)
	,m_dTrZ(0.0)
	,m_dAxisX(0.0)
	,m_dAxisY(0.0)
	,m_dAxisZ(1.0)
	,m_dAngle(0.0)
	,m_dOpening(0.0)
	,m_dJawLength(0.0)
	,m_pSegment(0)
	,m_nUnit(CHAIN_UNIT_M)
	,m_dShown(1000.0)
{
}

CTableDialog::~CTableDialog()
{
}

void CTableDialog::SetTerminal(CChainSegment *pSegment, CHAIN_UNIT nUnit)
{
	m_pSegment = pSegment;
	m_nUnit = nUnit;
	m_dShown = (nUnit==CHAIN_UNIT_IN) ? 1.0 : ChainUnitInMillimetres(nUnit);
	DialogFromTerminal();
}

const char * CTableDialog::ShownUnitName() const
{
	return (m_nUnit==CHAIN_UNIT_IN) ? "in" : "mm";
}

double CTableDialog::FromMetres(double d) const
{
	return d*1000.0/ChainUnitInMillimetres(m_nUnit);
}

void CTableDialog::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_TERMINAL_NAME, m_sName);
	DDX_CBIndex(pDX, IDC_COMBO_WORKPIECE_HOLDER, m_nHolderIndex);
	DDX_Text(pDX, IDC_EDIT_WP_TR_X, m_dTrX);
	DDX_Text(pDX, IDC_EDIT_WP_TR_Y, m_dTrY);
	DDX_Text(pDX, IDC_EDIT_WP_TR_Z, m_dTrZ);
	DDX_Text(pDX, IDC_EDIT_WP_AXIS_X, m_dAxisX);
	DDX_Text(pDX, IDC_EDIT_WP_AXIS_Y, m_dAxisY);
	DDX_Text(pDX, IDC_EDIT_WP_AXIS_Z, m_dAxisZ);
	DDX_Text(pDX, IDC_EDIT_WP_ANGLE, m_dAngle);
	DDX_Text(pDX, IDC_EDIT_VICE_OPENING, m_dOpening);
	DDX_Text(pDX, IDC_EDIT_VICE_JAW_LENGTH, m_dJawLength);
	DDX_Control(pDX, IDC_COMBO_WORKPIECE_HOLDER, m_comboHolder);
	DDX_Control(pDX, IDC_LIST_WP_BOXES, m_listBoxes);
}

BEGIN_MESSAGE_MAP(CTableDialog, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_WORKPIECE_HOLDER, OnCbnSelchangeComboWorkpieceHolder)
	ON_EN_CHANGE(IDC_EDIT_VICE_OPENING, OnEnChangeViceSize)
	ON_EN_CHANGE(IDC_EDIT_VICE_JAW_LENGTH, OnEnChangeViceSize)
	ON_BN_CLICKED(IDC_BUTTON_WP_ADD_BOX, OnBnClickedButtonWpAddBox)
	ON_BN_CLICKED(IDC_BUTTON_WP_DELETE_BOX, OnBnClickedButtonWpDeleteBox)
	ON_LBN_DBLCLK(IDC_LIST_WP_BOXES, OnLbnDblclkListWpBoxes)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CTableDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_comboHolder.ResetContent();
	for(size_t i=0;i<WorkpieceHolderCatalogueSize();i++){
		const std::string sModel = WorkpieceHolderFromCatalogue(i).GetModel();
		//the one that is nobody's holder but this user's says so in words
		m_comboHolder.AddString((sModel==szCUSTOM_HOLDER) ? "Custom - user defined" : sModel.c_str());
	}
	m_comboHolder.SetCurSel(m_nHolderIndex);
	SetDlgItemText(IDC_STATIC_UNIT_OPENING,ShownUnitName());
	SetDlgItemText(IDC_STATIC_UNIT_JAW,ShownUnitName());
	SetDlgItemText(IDC_STATIC_UNIT_TR,ShownUnitName());
	UpdateTheBoxList();
	EnableFields();
	return TRUE;
}

bool CTableDialog::IsVice(R3_AXIS &nOpeningAxis) const
{
	if(m_nHolderIndex<0 || (size_t)m_nHolderIndex>=WorkpieceHolderCatalogueSize())
		return false;
	const std::string sModel = WorkpieceHolderFromCatalogue((size_t)m_nHolderIndex).GetModel();
	if(sModel==szX_VICE){
		nOpeningAxis = R3_AXIS_X;
		return true;
	}
	if(sModel==szY_VICE){
		nOpeningAxis = R3_AXIS_Y;
		return true;
	}
	return false;
}

//The holder of the dialog, in the unit of the chain. A vice is built from its two numbers; every
//other entry keeps the frame and the boxes that are in the fields and in the list.
CWorkpieceHolder CTableDialog::HolderFromDialog() const
{
	R3_AXIS nAxis = R3_AXIS_X;
	const std::string sModel = (m_nHolderIndex>=0
		&& (size_t)m_nHolderIndex<WorkpieceHolderCatalogueSize())
		? WorkpieceHolderFromCatalogue((size_t)m_nHolderIndex).GetModel() : std::string(szNO_HOLDER);
	if(IsVice(nAxis))
		return MakeVice(sModel,nAxis,ToChain(m_dOpening),ToChain(m_dJawLength)
			,FromMetres(dVICE_JAW_THICKNESS),FromMetres(dVICE_JAW_HEIGHT));
	CWorkpieceHolder oHolder(sModel,CR3Vector(ToChain(m_dTrX),ToChain(m_dTrY),ToChain(m_dTrZ))
		,CR3Vector(m_dAxisX,m_dAxisY,m_dAxisZ),m_dAngle);
	oHolder.SetBoxes(m_vBoxes);
	return oHolder;
}

void CTableDialog::DialogFromTerminal()
{
	if(!m_pSegment)
		return;
	m_sName = m_pSegment->GetName().c_str();
	const CWorkpieceHolder &oHolder = m_pSegment->GetWorkpieceHolder();
	m_nHolderIndex = 0;
	for(size_t i=0;i<WorkpieceHolderCatalogueSize();i++)
		if(oHolder.GetModel()==WorkpieceHolderFromCatalogue(i).GetModel())
			m_nHolderIndex = (int)i;
	m_dTrX = FromChain(oHolder.GetTranslation().GetX());
	m_dTrY = FromChain(oHolder.GetTranslation().GetY());
	m_dTrZ = FromChain(oHolder.GetTranslation().GetZ());
	m_dAxisX = oHolder.GetRotationAxis().GetX();
	m_dAxisY = oHolder.GetRotationAxis().GetY();
	m_dAxisZ = oHolder.GetRotationAxis().GetZ();
	m_dAngle = oHolder.GetRotationAngleDegrees();
	m_vBoxes = oHolder.GetBoxes();

	//a vice says how far apart its jaws are and how long they are, read off the jaws themselves;
	//one that has no jaws yet takes the sizes of the catalogue
	R3_AXIS nAxis = R3_AXIS_X, nFound = R3_AXIS_X;
	double dOpening = 0.0, dJawLength = 0.0;
	if(!IsVice(nAxis))
		return;
	if(ViceParameters(oHolder,nFound,dOpening,dJawLength)){
		m_dOpening = FromChain(dOpening);
		m_dJawLength = FromChain(dJawLength);
		return;
	}
	CWorkpieceHolder oCatalogue = WorkpieceHolderFromCatalogue((size_t)m_nHolderIndex);
	if(ViceParameters(oCatalogue,nFound,dOpening,dJawLength)){
		m_dOpening = FromChain(FromMetres(dOpening));		//the catalogue is metres
		m_dJawLength = FromChain(FromMetres(dJawLength));
	}
}

void CTableDialog::UpdateTheBoxList()
{
	const int nSelected = m_listBoxes.GetCurSel();
	m_listBoxes.ResetContent();
	for(size_t i=0;i<m_vBoxes.size();i++){
		std::string s("box_");
		s += size_to_string(i+1);
		m_listBoxes.AddString(s.c_str());
	}
	if(nSelected>=0 && nSelected<(int)m_vBoxes.size())
		m_listBoxes.SetCurSel(nSelected);
}

void CTableDialog::EnableFields()
{
	//only the custom holder is measured here: the rest are the catalogue's own sizes
	const BOOL bCustom = (m_nHolderIndex==1) ? TRUE : FALSE;	//NO_HOLDER, then CUSTOM_HOLDER
	const int nFields[] = {IDC_EDIT_WP_TR_X,IDC_EDIT_WP_TR_Y,IDC_EDIT_WP_TR_Z
		,IDC_EDIT_WP_AXIS_X,IDC_EDIT_WP_AXIS_Y,IDC_EDIT_WP_AXIS_Z,IDC_EDIT_WP_ANGLE
		,IDC_BUTTON_WP_ADD_BOX,IDC_BUTTON_WP_DELETE_BOX,IDC_STATIC_UNIT_TR};
	for(int i=0;i<10;i++)
		if(GetDlgItem(nFields[i]))
			GetDlgItem(nFields[i])->EnableWindow(bCustom);

	//and the two numbers of a vice belong to a vice
	R3_AXIS nAxis = R3_AXIS_X;
	const BOOL bVice = IsVice(nAxis) ? TRUE : FALSE;
	const int nViceFields[] = {IDC_EDIT_VICE_OPENING,IDC_STATIC_LBL_OPENING,IDC_STATIC_UNIT_OPENING
		,IDC_EDIT_VICE_JAW_LENGTH,IDC_STATIC_LBL_JAW,IDC_STATIC_UNIT_JAW};
	for(int i=0;i<6;i++)
		if(GetDlgItem(nViceFields[i]))
			GetDlgItem(nViceFields[i])->EnableWindow(bVice);
}

void CTableDialog::OnCbnSelchangeComboWorkpieceHolder()
{
	UpdateData(TRUE);
	R3_AXIS nAxis = R3_AXIS_X;
	if(IsVice(nAxis)){
		//the sizes of the vice of the catalogue to start from, scaled out of its metres
		R3_AXIS nFound = R3_AXIS_X;
		double dOpening = 0.0, dJawLength = 0.0;
		CWorkpieceHolder oCatalogue = WorkpieceHolderFromCatalogue((size_t)m_nHolderIndex);
		if(ViceParameters(oCatalogue,nFound,dOpening,dJawLength)){
			m_dOpening = FromChain(FromMetres(dOpening));
			m_dJawLength = FromChain(FromMetres(dJawLength));
		}
		//and the jaws and the frame follow from those two numbers
		const CWorkpieceHolder oVice = HolderFromDialog();
		m_vBoxes = oVice.GetBoxes();
		m_dTrX = FromChain(oVice.GetTranslation().GetX());
		m_dTrY = FromChain(oVice.GetTranslation().GetY());
		m_dTrZ = FromChain(oVice.GetTranslation().GetZ());
		m_dAxisX = oVice.GetRotationAxis().GetX();
		m_dAxisY = oVice.GetRotationAxis().GetY();
		m_dAxisZ = oVice.GetRotationAxis().GetZ();
		m_dAngle = oVice.GetRotationAngleDegrees();
	} else if(m_nHolderIndex!=1 && m_nHolderIndex>=0
		&& (size_t)m_nHolderIndex<WorkpieceHolderCatalogueSize()){
		//the frame and the solids of the picked holder, scaled into the unit of the chain
		const CWorkpieceHolder oCatalogue = WorkpieceHolderFromCatalogue((size_t)m_nHolderIndex);
		m_dTrX = FromChain(FromMetres(oCatalogue.GetTranslation().GetX()));
		m_dTrY = FromChain(FromMetres(oCatalogue.GetTranslation().GetY()));
		m_dTrZ = FromChain(FromMetres(oCatalogue.GetTranslation().GetZ()));
		m_dAxisX = oCatalogue.GetRotationAxis().GetX();
		m_dAxisY = oCatalogue.GetRotationAxis().GetY();
		m_dAxisZ = oCatalogue.GetRotationAxis().GetZ();
		m_dAngle = oCatalogue.GetRotationAngleDegrees();
		m_vBoxes.clear();
		for(size_t i=0;i<oCatalogue.GetBoxes().size();i++){
			const CR3Point p1 = oCatalogue.GetBoxes()[i].GetMinPoint();
			const CR3Point p2 = oCatalogue.GetBoxes()[i].GetMaxPoint();
			m_vBoxes.push_back(CR3Box(
				CR3Point(FromMetres(p1.GetX()),FromMetres(p1.GetY()),FromMetres(p1.GetZ()))
				,CR3Point(FromMetres(p2.GetX()),FromMetres(p2.GetY()),FromMetres(p2.GetZ()))));
		}
	}
	UpdateData(FALSE);
	UpdateTheBoxList();
	EnableFields();
}

void CTableDialog::OnEnChangeViceSize()
{
	R3_AXIS nAxis = R3_AXIS_X;
	if(!IsVice(nAxis))
		return;
	UpdateData(TRUE);
	//the jaws follow the two numbers on every keystroke, so the list shows what will go on the
	//table; a half typed number makes no vice at all, and OK says so rather than storing one
	const CWorkpieceHolder oVice = HolderFromDialog();
	m_vBoxes = oVice.GetBoxes();
	m_dTrZ = FromChain(oVice.GetTranslation().GetZ());
	UpdateData(FALSE);
	UpdateTheBoxList();
}

void CTableDialog::OnBnClickedButtonWpAddBox()
{
	UpdateData(TRUE);
	m_vBoxes.push_back(CR3Box());
	CBoxDialog dlg;
	dlg.SetBox(m_vBoxes.back());
	if(dlg.DoModal()==IDOK)
		m_vBoxes.back() = dlg.GetBox();
	else
		m_vBoxes.pop_back();
	UpdateTheBoxList();
}

void CTableDialog::OnBnClickedButtonWpDeleteBox()
{
	const int nSelected = m_listBoxes.GetCurSel();
	if(nSelected<0 || nSelected>=(int)m_vBoxes.size())
		return;
	m_vBoxes.erase(m_vBoxes.begin()+nSelected);
	UpdateTheBoxList();
}

void CTableDialog::OnLbnDblclkListWpBoxes()
{
	const int nSelected = m_listBoxes.GetCurSel();
	if(nSelected<0 || nSelected>=(int)m_vBoxes.size() || m_nHolderIndex!=1)
		return;
	CBoxDialog dlg;
	dlg.SetBox(m_vBoxes[nSelected]);
	if(dlg.DoModal()==IDOK)
		m_vBoxes[nSelected] = dlg.GetBox();
	UpdateTheBoxList();
}

//The solids of a holder are solids of the table: the ones the old holder put there are taken away
//and the ones of the new holder are put in their place, so that picking a vice puts a vice on the
//machine - drawn in the 3D view, and in the way of the tool like everything else it is made of.
void CTableDialog::PutTheSolidsOnTheTable(const CWorkpieceHolder &oNew)
{
	if(!m_pSegment)
		return;
	const std::vector<CR3Box> vOld = m_pSegment->GetWorkpieceHolder().GetBoxes();
	std::vector<CSolid> vSolids;
	for(size_t i=0;i<m_pSegment->GetPartVector().size();i++){
		const CSolid &oSolid = m_pSegment->GetPartVector()[i];
		bool bOfTheOldHolder = false;
		for(size_t k=0;k<vOld.size();k++)
			if(oSolid.GetType()==SOLID_BOX && oSolid.GetBox()==vOld[k])
				bOfTheOldHolder = true;
		if(!bOfTheOldHolder)
			vSolids.push_back(oSolid);			//a solid of the machine itself stays
	}
	for(size_t i=0;i<oNew.GetBoxes().size();i++)
		vSolids.push_back(CSolid(oNew.GetBoxes()[i]));
	m_pSegment->SetPartVector(vSolids);
}

void CTableDialog::OnBnClickedOk()
{
	UpdateData(TRUE);
	if(CR3Vector(m_dAxisX,m_dAxisY,m_dAxisZ).IsZero()){
		AfxMessageBox("The axis of the holder frame has no length, so it is no axis to turn about.");
		return;
	}
	const CWorkpieceHolder oHolder = HolderFromDialog();
	R3_AXIS nAxis = R3_AXIS_X;
	if(IsVice(nAxis) && oHolder.GetBoxes().size()!=2){
		AfxMessageBox("A vice is two jaws: give it an opening and a jaw length greater than zero.");
		return;
	}
	if(m_pSegment){
		m_pSegment->SetName((LPCTSTR)m_sName);
		PutTheSolidsOnTheTable(oHolder);		//while the segment still carries the old holder
		m_pSegment->SetWorkpieceHolder(oHolder);
	}
	OnOK();
}
