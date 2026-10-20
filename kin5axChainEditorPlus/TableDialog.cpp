// TableDialog.cpp : the terminal of the workpiece branch and the holder standing on it
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "TableDialog.h"
#include "BoxDialog.h"

IMPLEMENT_DYNAMIC(CTableDialog, CDialog)

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
	,m_pSegment(0)
	,m_dUnitInMillimetres(1000.0)
{
}

CTableDialog::~CTableDialog()
{
}

void CTableDialog::SetTerminal(CChainSegment *pSegment, double dUnitInMillimetres)
{
	m_pSegment = pSegment;
	m_dUnitInMillimetres = dUnitInMillimetres;
	DialogFromTerminal();
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
	DDX_Control(pDX, IDC_COMBO_WORKPIECE_HOLDER, m_comboHolder);
	DDX_Control(pDX, IDC_LIST_WP_BOXES, m_listBoxes);
}

BEGIN_MESSAGE_MAP(CTableDialog, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_WORKPIECE_HOLDER, OnCbnSelchangeComboWorkpieceHolder)
	ON_BN_CLICKED(IDC_BUTTON_WP_ADD_BOX, OnBnClickedButtonWpAddBox)
	ON_BN_CLICKED(IDC_BUTTON_WP_DELETE_BOX, OnBnClickedButtonWpDeleteBox)
	ON_LBN_DBLCLK(IDC_LIST_WP_BOXES, OnLbnDblclkListWpBoxes)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CTableDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_comboHolder.ResetContent();
	for(size_t i=0;i<WorkpieceHolderCatalogueSize();i++)
		m_comboHolder.AddString(WorkpieceHolderFromCatalogue(i).GetModel().c_str());
	m_comboHolder.SetCurSel(m_nHolderIndex);
	UpdateTheBoxList();
	EnableFields();
	return TRUE;
}

//The holder of the dialog. Every entry but CUSTOM_HOLDER is the catalogue's, scaled into the unit of
//the chain because the catalogue is in metres; the custom one is the frame and the boxes typed here.
CWorkpieceHolder CTableDialog::HolderFromDialog() const
{
	const std::string sModel = (m_nHolderIndex>=0 && (size_t)m_nHolderIndex<WorkpieceHolderCatalogueSize())
		? WorkpieceHolderFromCatalogue((size_t)m_nHolderIndex).GetModel() : std::string(szNO_HOLDER);
	CWorkpieceHolder oHolder(sModel,CR3Vector(m_dTrX,m_dTrY,m_dTrZ)
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
	m_dTrX = oHolder.GetTranslation().GetX();
	m_dTrY = oHolder.GetTranslation().GetY();
	m_dTrZ = oHolder.GetTranslation().GetZ();
	m_dAxisX = oHolder.GetRotationAxis().GetX();
	m_dAxisY = oHolder.GetRotationAxis().GetY();
	m_dAxisZ = oHolder.GetRotationAxis().GetZ();
	m_dAngle = oHolder.GetRotationAngleDegrees();
	m_vBoxes = oHolder.GetBoxes();
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
		,IDC_BUTTON_WP_ADD_BOX,IDC_BUTTON_WP_DELETE_BOX};
	for(int i=0;i<9;i++)
		if(GetDlgItem(nFields[i]))
			GetDlgItem(nFields[i])->EnableWindow(bCustom);
}

void CTableDialog::OnCbnSelchangeComboWorkpieceHolder()
{
	UpdateData(TRUE);
	if(m_nHolderIndex!=1 && m_nHolderIndex>=0
		&& (size_t)m_nHolderIndex<WorkpieceHolderCatalogueSize()){
		//the frame and the solids of the picked holder, scaled into the unit of the chain
		const CWorkpieceHolder oCatalogue = WorkpieceHolderFromCatalogue((size_t)m_nHolderIndex);
		const double dFactor = 1000.0/m_dUnitInMillimetres;
		m_dTrX = oCatalogue.GetTranslation().GetX()*dFactor;
		m_dTrY = oCatalogue.GetTranslation().GetY()*dFactor;
		m_dTrZ = oCatalogue.GetTranslation().GetZ()*dFactor;
		m_dAxisX = oCatalogue.GetRotationAxis().GetX();
		m_dAxisY = oCatalogue.GetRotationAxis().GetY();
		m_dAxisZ = oCatalogue.GetRotationAxis().GetZ();
		m_dAngle = oCatalogue.GetRotationAngleDegrees();
		m_vBoxes.clear();
		for(size_t i=0;i<oCatalogue.GetBoxes().size();i++){
			const CR3Point p1 = oCatalogue.GetBoxes()[i].GetMinPoint();
			const CR3Point p2 = oCatalogue.GetBoxes()[i].GetMaxPoint();
			//the two corners, not six loose numbers: the six-double constructor takes them as
			//min and max per axis, which is not the order a corner comes in
			m_vBoxes.push_back(CR3Box(CR3Point(p1.GetX()*dFactor,p1.GetY()*dFactor,p1.GetZ()*dFactor)
				,CR3Point(p2.GetX()*dFactor,p2.GetY()*dFactor,p2.GetZ()*dFactor)));
		}
	}
	UpdateData(FALSE);
	UpdateTheBoxList();
	EnableFields();
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

void CTableDialog::OnBnClickedOk()
{
	UpdateData(TRUE);
	if(CR3Vector(m_dAxisX,m_dAxisY,m_dAxisZ).IsZero()){
		AfxMessageBox("The axis of the holder frame has no length, so it is no axis to turn about.");
		return;
	}
	if(m_pSegment){
		m_pSegment->SetName((LPCTSTR)m_sName);
		m_pSegment->SetWorkpieceHolder(HolderFromDialog());
	}
	OnOK();
}
