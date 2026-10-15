// KinematicChainPropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "string_tools.h"
#include "kin5axChainEditorPlus.h"
#include "KinematicChainPropertiesDialog.h"
#include ".\kinematicchainpropertiesdialog.h"
#include "boxdialog.h"


// CKinematicChainPropertiesDialog dialog

IMPLEMENT_DYNAMIC(CKinematicChainPropertiesDialog, CDialog)
CKinematicChainPropertiesDialog::CKinematicChainPropertiesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CKinematicChainPropertiesDialog::IDD, pParent)
	, m_dTrX(0.2)
	, m_dTrY(0.2)
	, m_dTrZ(0.2)
	, m_dAxisX(1.0)
	, m_dAxisY(0)
	, m_dAxisZ(0)
	, m_dAngle(0)
	, m_nSegmentTypeIndex(0)
	, m_pDoc(0)
	, m_pOldParentNode(0)
	, m_nOldIndex(-1)
	, m_bParameterIsConstrained(FALSE)
	, m_dParameterMaximum(0)
	, m_dParameterMinimum(0)
	, m_nSliderValue(0)
	, m_dParameterValue(0)
	, m_dParameterStart(0)
	, m_dParameterEnd(0)
	, m_sSegmentName(_T(""))
	, m_sChainHolderModel(_T(""))
	, m_dChainHolderLength(0.0)
	, m_dChainHolderRadius(0.0)
	, m_pActiveSegment(0)
	, m_nActivePart(-1)
{
}

CKinematicChainPropertiesDialog::~CKinematicChainPropertiesDialog()
{
}

void CKinematicChainPropertiesDialog::SetActiveDocument(CKin5axChainEditorPlusDoc *pDoc)
{
	assert(pDoc);
	// the segment vectors may have changed: the pointer is taken again from the tree selection
	m_pActiveSegment = 0;
	if(m_pDoc){
		while(1){
			HTREEITEM item = m_treeSegments.GetChildItem(m_hToolPart);
			if(item==NULL)
				break;
			m_treeSegments.DeleteItem(item);
		}
		while(1){
			HTREEITEM item = m_treeSegments.GetChildItem(m_hMaterialHolderPart);
			if(item==NULL)
				break;
			m_treeSegments.DeleteItem(item);
		}
	}
	m_pDoc = pDoc;
	if(m_comboUnit.GetSafeHwnd())
		m_comboUnit.SetCurSel((int)m_pDoc->m_oChain.GetUnit());
	if(m_pOldParentNode==0)
		m_pOldParentNode = &m_hToolPart;
	if(m_nOldIndex<0)
		m_treeSegments.SelectItem(*m_pOldParentNode);
	for(size_t i=0;i<m_pDoc->m_oChain.m_vToolPart.size();i++){
		HTREEITEM item = m_treeSegments.InsertItem(m_pDoc->m_oChain.m_vToolPart[i].GetTypeName().c_str(),m_hToolPart);
		m_treeSegments.SetItemData(item,(DWORD_PTR)i);
		if(m_nOldIndex==i && m_pOldParentNode == &m_hToolPart)
			m_treeSegments.SelectItem(item);
	}
	for(size_t i=0;i<m_pDoc->m_oChain.m_vMaterialHolderPart.size();i++){
		HTREEITEM item = m_treeSegments.InsertItem(m_pDoc->m_oChain.m_vMaterialHolderPart[i].GetTypeName().c_str(),m_hMaterialHolderPart);
		m_treeSegments.SetItemData(item,(DWORD_PTR)i);
		if(m_nOldIndex==i && m_pOldParentNode == &m_hMaterialHolderPart)
			m_treeSegments.SelectItem(item);

	}
	m_treeSegments.Invalidate(true);
}

void CKinematicChainPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SEGMENT_TREE, m_treeSegments);
	DDX_Control(pDX, IDC_COMBO_SEGMENT_TYPE, m_comboSegmentType);
	DDX_Control(pDX, IDC_COMBO_UNIT, m_comboUnit);
	DDX_Control(pDX, IDC_COMBO_CHAIN_HOLDER_MODEL, m_comboChainHolderModel);
	DDX_CBString(pDX, IDC_COMBO_CHAIN_HOLDER_MODEL, m_sChainHolderModel);
	DDX_Control(pDX, IDC_EDIT_CHAIN_HOLDER_LENGTH, m_editChainHolderLength);
	DDX_Control(pDX, IDC_EDIT_CHAIN_HOLDER_RADIUS, m_editChainHolderRadius);
	DDX_Text(pDX, IDC_EDIT_CHAIN_HOLDER_LENGTH, m_dChainHolderLength);
	DDX_Text(pDX, IDC_EDIT_CHAIN_HOLDER_RADIUS, m_dChainHolderRadius);
	DDX_Text(pDX, IDC_EDIT_TR_X, m_dTrX);
	DDX_Text(pDX, IDC_EDIT_TR_Y, m_dTrY);
	DDX_Text(pDX, IDC_EDIT_TR_Z, m_dTrZ);
	DDX_Control(pDX, IDC_COMBO_PREDEFINED_AXIS, m_comboPredefinedAxes);
	DDX_Control(pDX, IDC_COMBO_PREDEFINED_ANGLES, m_comboPredefinedAngles);
	DDX_Text(pDX, IDC_EDIT_AXIS_X, m_dAxisX);
	DDX_Text(pDX, IDC_EDIT_AXIS_Y, m_dAxisY);
	DDX_Text(pDX, IDC_EDIT_AXIS_Z, m_dAxisZ);

	DDX_Control(pDX, ID_BTN_ADD_SEGMENT, m_buttonAddSegment);
	DDX_Control(pDX, ID_BTN_DELETE_SEGMENT, m_buttonDeleteSegment);
	DDX_Control(pDX, IDC_EDIT_TR_X, m_editTrX);
	DDX_Control(pDX, IDC_EDIT_TR_Y, m_editTrY);
	DDX_Control(pDX, IDC_EDIT_TR_Z, m_editTrZ);
	DDX_Control(pDX, IDC_EDIT_AXIS_X, m_editAxisX);
	DDX_Control(pDX, IDC_EDIT_AXIS_Y, m_editAxisY);
	DDX_Control(pDX, IDC_EDIT_AXIS_Z, m_editAxisZ);
	DDX_Control(pDX, IDC_EDIT_ANGLE, m_editAngle);
	DDX_Text(pDX, IDC_EDIT_ANGLE, m_dAngle);
	DDX_CBIndex(pDX, IDC_COMBO_SEGMENT_TYPE, m_nSegmentTypeIndex);
	DDX_Control(pDX, IDC_CHECK_CONSTRAINT, m_checkParameterIsConstrained);
	DDX_Control(pDX, IDC_EDIT_PARAMETER_MINIMUM, m_editParameterMinimum);
	DDX_Control(pDX, IDC_EDIT_PARAMETER_MAXIMUM, m_editParameterMaximum);
	DDX_Check(pDX, IDC_CHECK_CONSTRAINT, m_bParameterIsConstrained);
	DDX_Text(pDX, IDC_EDIT_PARAMETER_MAXIMUM, m_dParameterMaximum);
	DDX_Text(pDX, IDC_EDIT_PARAMETER_MINIMUM, m_dParameterMinimum);
	DDX_Control(pDX, IDC_EDIT_SEGMENT_NAME, m_editSegmentName);
	DDX_Slider(pDX, IDC_PARAMETER_SLIDER, m_nSliderValue);
	DDX_Control(pDX, IDC_EDIT_PARAMETER_VALUE, m_edtParameterValue);
	DDX_Text(pDX, IDC_EDIT_PARAMETER_VALUE, m_dParameterValue);
	DDX_Text(pDX, IDC_EDIT_PARAMETER_START, m_dParameterStart);
	DDX_Text(pDX, IDC_EDIT_PARAMETER_END, m_dParameterEnd);
	DDX_Control(pDX, IDC_PARAMETER_SLIDER, m_sldParameter);
	DDX_Control(pDX, ID_BTN_ZERO_PARAMETER, m_btnZeroParameter);
	DDX_Text(pDX, IDC_EDIT_SEGMENT_NAME, m_sSegmentName);
	DDX_Control(pDX, IDC_LIST_PARTS, m_listboxSegmentParts);
	DDX_Control(pDX, IDC_BUTTON_DELETE_PART, m_buttonDeletePart);
	DDX_Control(pDX, IDC_BUTTON_ADD_PART, m_buttonAddPart);
}


BEGIN_MESSAGE_MAP(CKinematicChainPropertiesDialog, CDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_SEGMENT_TREE, OnTvnSelchangedSegmentTree)
	ON_BN_CLICKED(ID_BTN_ADD_SEGMENT, OnBnClickedAddSegment)
	ON_BN_CLICKED(ID_BTN_DELETE_SEGMENT, OnBnClickedDeleteSegment)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO_PREDEFINED_AXIS, OnCbnSelchangeComboPredefinedAxis)
	ON_CBN_SELCHANGE(IDC_COMBO_SEGMENT_TYPE, OnCbnSelchangeComboSegmentType)
	ON_CBN_SELCHANGE(IDC_COMBO_UNIT, OnCbnSelchangeComboUnit)
	ON_CBN_SELCHANGE(IDC_COMBO_CHAIN_HOLDER_MODEL, OnCbnSelchangeComboChainHolderModel)
	ON_CBN_EDITCHANGE(IDC_COMBO_CHAIN_HOLDER_MODEL, OnCbnEditchangeComboChainHolderModel)
	ON_EN_CHANGE(IDC_EDIT_CHAIN_HOLDER_LENGTH, OnEnChangeEditChainHolderLength)
	ON_EN_CHANGE(IDC_EDIT_CHAIN_HOLDER_RADIUS, OnEnChangeEditChainHolderRadius)
	ON_EN_CHANGE(IDC_EDIT_TR_X, OnEnChangeEditTrX)
	ON_EN_CHANGE(IDC_EDIT_TR_Y, OnEnChangeEditTrY)
	ON_EN_CHANGE(IDC_EDIT_TR_Z, OnEnChangeEditTrZ)
	ON_EN_CHANGE(IDC_EDIT_AXIS_X, OnEnChangeEditAxisX)
	ON_EN_CHANGE(IDC_EDIT_AXIS_Y, OnEnChangeEditAxisY)
	ON_EN_CHANGE(IDC_EDIT_AXIS_Z, OnEnChangeEditAxisZ)
	ON_EN_CHANGE(IDC_EDIT_ANGLE, OnEnChangeEditAngle)
	ON_CBN_SELCHANGE(IDC_COMBO_PREDEFINED_ANGLES, OnCbnSelchangeComboPredefinedAngles)
	ON_EN_KILLFOCUS(IDC_EDIT_TR_Z, OnEnKillfocusEditTrZ)
	ON_EN_KILLFOCUS(IDC_EDIT_TR_Y, OnEnKillfocusEditTrY)
	ON_EN_KILLFOCUS(IDC_EDIT_TR_X, OnEnKillfocusEditTrX)
	ON_EN_KILLFOCUS(IDC_EDIT_AXIS_X, OnEnKillfocusEditAxisX)
	ON_EN_KILLFOCUS(IDC_EDIT_AXIS_Y, OnEnKillfocusEditAxisY)
	ON_EN_KILLFOCUS(IDC_EDIT_AXIS_Z, OnEnKillfocusEditAxisZ)
	ON_EN_KILLFOCUS(IDC_EDIT_ANGLE, OnEnKillfocusEditAngle)
	ON_BN_CLICKED(ID_BTN_ZERO_TR_X, OnBnClickedBtnZeroTrX)
	ON_BN_CLICKED(ID_BTN_ZERO_TR_Y, OnBnClickedBtnZeroTrY)
	ON_BN_CLICKED(ID_BTN_ZERO_TR_Z, OnBnClickedBtnZeroTrZ)
	ON_BN_CLICKED(ID_BTN_ZERO_TR, OnBnClickedBtnZeroTr)
	ON_EN_CHANGE(IDC_EDIT_PARAMETER_MINIMUM, OnEnChangeEditParameterMinimum)
	ON_EN_CHANGE(IDC_EDIT_PARAMETER_MAXIMUM, OnEnChangeEditParameterMaximum)
	ON_BN_CLICKED(IDC_CHECK_CONSTRAINT, OnBnClickedCheckConstraint)
	ON_WM_SHOWWINDOW()
	ON_EN_CHANGE(IDC_EDIT_SEGMENT_NAME, OnEnChangeSegmentName)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(ID_BTN_ZERO_PARAMETER, OnBnClickedBtnZeroParameter)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(ID_BTN_RESET_ALL_PARAMS, OnBnClickedBtnResetAllParams)
	ON_EN_CHANGE(IDC_EDIT_PARAMETER_VALUE, OnEnChangeEditParameterValue)
	ON_WM_CLOSE()

//	ON_EN_KILLFOCUS(IDC_EDIT_SEGMENT_NAME, m_)
ON_EN_KILLFOCUS(IDC_EDIT_SEGMENT_NAME, OnEnKillfocusEditSegmentName)
ON_LBN_SELCHANGE(IDC_LIST_PARTS, OnLbnSelchangeListParts)
ON_LBN_DBLCLK(IDC_LIST_PARTS, OnLbnDblclkListParts)
ON_BN_CLICKED(IDC_BUTTON_ADD_PART, OnBnClickedButtonAddPart)
ON_BN_CLICKED(IDC_BUTTON_DELETE_PART, OnBnClickedButtonDeletePart)
END_MESSAGE_MAP()


// CKinematicChainPropertiesDialog message handlers

BOOL CKinematicChainPropertiesDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	// TODO:  Add extra initialization here


	m_hToolPart = m_treeSegments.InsertItem("Tool part");
	m_treeSegments.SetItemData(m_hToolPart,(DWORD_PTR)-1);
	m_hMaterialHolderPart = m_treeSegments.InsertItem("Material holder part");
	m_treeSegments.SetItemData(m_hMaterialHolderPart,(DWORD_PTR)-1);
	m_sldParameter.SetRange(0,100,TRUE);
	m_sldParameter.SetPos(50);
	//the three units in the order of CHAIN_UNIT, so that the index of the combo is the unit
	for(int i=0;i<CHAIN_UNIT_COUNT;i++)
		m_comboUnit.AddString(ChainUnitName((CHAIN_UNIT)i));
	//the holders a five-axis machine usually carries; any other designation can be typed in, and
	//then the length and the radius of that holder are given here too
	for(size_t i=0;i<ToolHolderCatalogueSize();i++)
		m_comboChainHolderModel.AddString(ToolHolderFromCatalogue(i).GetModel().c_str());

	//*/
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CKinematicChainPropertiesDialog::OnTvnSelchangedSegmentTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	if(!m_pDoc){
		DisableSegmentProperties();
		return;
	}
	if(m_pActiveSegment)
		m_pActiveSegment->HighlightPart(-1);
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	std::vector<CChainSegment> *pvVector;
	int nIndex;
	bool bIsToolPartActive;
	if(GetSegmentVectorFromTreeItem(pNMTreeView->itemNew.hItem,&pvVector,nIndex) && nIndex<(int)pvVector->size()){
		
		m_pActiveSegment = &(*pvVector)[nIndex];
		EnableSegmentProperties((*pvVector)[nIndex]);
		if(m_pActiveSegment)
			m_pActiveSegment->HighlightPart(-1);
		if(m_hMaterialHolderPart == m_treeSegments.GetParentItem(pNMTreeView->itemNew.hItem)){
			m_pOldParentNode =	&m_hMaterialHolderPart;
			bIsToolPartActive = false;
		} else{
			m_pOldParentNode = &m_hToolPart;
			bIsToolPartActive=true;
		}
	} else {
		m_pActiveSegment=0;
		DisableSegmentProperties();
		if(pNMTreeView->itemNew.hItem==m_hMaterialHolderPart){
			bIsToolPartActive = false;
			m_pOldParentNode =	&m_hMaterialHolderPart;
		} else {
			m_pOldParentNode = &m_hToolPart;
			bIsToolPartActive = true;
		}
	}
	m_pDoc->m_oChain.SetActiveSegment(bIsToolPartActive ? CHAIN_PART_TOOL : CHAIN_PART_MATERIAL_HOLDER,nIndex);
	m_pDoc->UpdateAllViews(NULL);
	UpdateParameterSliderUnits();
	*pResult = 0;
}

void CKinematicChainPropertiesDialog::OnBnClickedAddSegment()
{
	HTREEITEM hItem = m_treeSegments.GetSelectedItem();
	UpdateData(TRUE);
	std::vector<CChainSegment> *pvVector;
	int nIndex;
	GetSegmentVectorFromTreeItem(hItem,&pvVector,nIndex);
	assert(m_pDoc);
	m_nOldIndex=nIndex+1;
	CChainSegment oDefSegment(CHAIN_SEGMENT_REVOLUTE_LINK,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.2,0.2,0.2));
	size_t n = m_pDoc->m_oChain.GetNumberOfPrismaticLinks()+m_pDoc->m_oChain.GetNumberOfRevoluteLinks();
	CString s;
	s.Format("S%zu",n);
	oDefSegment.SetName(s.GetBuffer(0));
	ForgetActiveSegment();
	m_pDoc->m_oChain.AddSegmentAfterActiveSegment(oDefSegment);
	SetActiveDocument(m_pDoc);
	UpdateParameterSliderUnits();
	m_pDoc->UpdateAllViews(NULL);
}

// Before the segment vector changes: m_pActiveSegment would dangle
void CKinematicChainPropertiesDialog::ForgetActiveSegment()
{
	if(m_pActiveSegment)
		m_pActiveSegment->HighlightPart(-1);
	m_pActiveSegment = 0;
}

void CKinematicChainPropertiesDialog::OnBnClickedDeleteSegment()
{
	assert(m_pDoc);
	ForgetActiveSegment();
	m_pDoc->m_oChain.DeleteActiveSegment();
	SetActiveDocument(m_pDoc);
	UpdateParameterSliderUnits();
	m_pDoc->UpdateAllViews(NULL);

}

void CKinematicChainPropertiesDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//OnOK();
	ShowWindow(SW_HIDE);
}

// makes gray all controls concerned with chain segment
void CKinematicChainPropertiesDialog::DisableSegmentProperties(void)
{
	m_comboPredefinedAngles.EnableWindow(FALSE);
	m_comboPredefinedAxes.EnableWindow(FALSE);
	m_comboSegmentType.EnableWindow(FALSE);

	m_editAngle.EnableWindow(FALSE);
	m_editAxisX.EnableWindow(FALSE);
	m_editAxisY.EnableWindow(FALSE);
	m_editAxisZ.EnableWindow(FALSE);
	m_editTrX.EnableWindow(FALSE);
	m_editTrY.EnableWindow(FALSE);
	m_editTrZ.EnableWindow(FALSE);

	m_editParameterMaximum.EnableWindow(FALSE);
	m_editParameterMinimum.EnableWindow(FALSE);
	m_checkParameterIsConstrained.EnableWindow(FALSE);
	m_editSegmentName.EnableWindow(FALSE);
	m_edtParameterValue.EnableWindow(FALSE);
	m_buttonAddPart.EnableWindow(FALSE);
	m_buttonDeletePart.EnableWindow(FALSE);
	UpdateChainHolderFields(0);

	

}

// enables and fills controls concerned with passed chain segment
void CKinematicChainPropertiesDialog::EnableSegmentProperties(const CChainSegment & segment)
{
	m_comboPredefinedAngles.EnableWindow(TRUE);
	m_comboPredefinedAxes.EnableWindow(TRUE);
	BOOL bIsRegularJoint = (BOOL) segment.IsRegularJoint();
	m_comboSegmentType.EnableWindow(bIsRegularJoint);
	m_buttonDeleteSegment.EnableWindow(bIsRegularJoint);
	m_editSegmentName.EnableWindow(bIsRegularJoint);


	m_editAngle.EnableWindow(TRUE);
	m_editAxisX.EnableWindow(TRUE);
	m_editAxisY.EnableWindow(TRUE);
	m_editAxisZ.EnableWindow(TRUE);
	m_editTrX.EnableWindow(TRUE);
	m_editTrY.EnableWindow(TRUE);
	m_editTrZ.EnableWindow(TRUE);
	
	m_checkParameterIsConstrained.EnableWindow(TRUE);


	m_editParameterMaximum.EnableWindow((BOOL)segment.IsParameterConstrained());
	m_editParameterMinimum.EnableWindow((BOOL)segment.IsParameterConstrained());
	
	m_sldParameter.EnableWindow((BOOL)segment.IsRegularJoint());
	m_edtParameterValue.EnableWindow((BOOL)segment.IsRegularJoint());
	m_btnZeroParameter.EnableWindow((BOOL)segment.IsRegularJoint());
	

	SetSegmentTypeComboValue(segment.GetType());
	m_dAngle = segment.m_dConstRevoluteAngle;
	segment.m_r3vConstRevoluteAxis.CopyCoordinates(m_dAxisX,m_dAxisY,m_dAxisZ);
	segment.m_r3vConstTranslation.CopyCoordinates(m_dTrX,m_dTrY,m_dTrZ);

	m_bParameterIsConstrained = segment.m_bParameterIsConstrained;
	if(segment.IsParameterConstrained()){
		m_dParameterMinimum = segment.m_dParameterValueMinimum;
		m_dParameterMaximum = segment.m_dParameterValueMaximum;
		if(segment.GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
			m_dParameterMinimum = radians_to_degrees(m_dParameterMinimum);
			m_dParameterMaximum = radians_to_degrees(m_dParameterMaximum);
		}
	} else {
		if(segment.GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
			m_dParameterMinimum = -180;
			m_dParameterMaximum = 180;
		} else {
			m_dParameterMinimum = -1;
			m_dParameterMaximum = 1;
		}		
	}
	m_dParameterValue = segment.m_dActualParameterValue;
	if(segment.GetType()==CHAIN_SEGMENT_REVOLUTE_LINK)
		m_dParameterValue=radians_to_degrees(m_dParameterValue);
	m_sSegmentName = segment.GetName().c_str();
	UpdateData(FALSE);
	m_comboPredefinedAngles.SetCurSel(0);
	m_comboPredefinedAxes.SetCurSel(0);
	
	m_nActivePart = segment.GetHighlightedPartIndex();
	m_vBoxes = segment.GetPartVector();

	m_buttonAddPart.EnableWindow(TRUE);
	m_buttonDeletePart.EnableWindow(TRUE);
	UpdateChainHolderFields(&segment);
	UpdateSegmentPartsList();

}
bool CKinematicChainPropertiesDialog::GetSegmentVectorFromTreeItem(const HTREEITEM & hTreeItem,std::vector<CChainSegment> **ppvVectorSegment, int &nIndex)
{
	assert(m_pDoc);
	nIndex = (int)m_treeSegments.GetItemData(hTreeItem);
	HTREEITEM item;
	if(nIndex>=0)
		item = m_treeSegments.GetParentItem(hTreeItem);
	else
		item = hTreeItem;
	if(item == m_hMaterialHolderPart){
		(*ppvVectorSegment) = &m_pDoc->m_oChain.m_vMaterialHolderPart;
	} else {
		(*ppvVectorSegment) = &m_pDoc->m_oChain.m_vToolPart;
	}
	if(nIndex>=0)
		return true;
	return false;
}

void CKinematicChainPropertiesDialog::OnCbnSelchangeComboPredefinedAxis()
{
	UpdateData(TRUE);
	int nSel = m_comboPredefinedAxes.GetCurSel();
	if(nSel<1)
		return;
	m_dAxisX=m_dAxisY=m_dAxisZ=0.0;
	switch (nSel){
		case 1:
			m_dAxisX=1.0;
			break;
		case 2:
			m_dAxisY=1.0;
			break;
		case 3:
			m_dAxisZ=1.0;
			break;
	}
	UpdateData(FALSE);
	UpdateActualSegment();
}

// translates index of selected valuea from SegmentTypeCombo into CHAIN_SEGMENT_TYPE
CHAIN_SEGMENT_TYPE CKinematicChainPropertiesDialog::GetSegmentTypeFromTypeIndex(int nIndex)
{
	
	if(nIndex<2)
		return (CHAIN_SEGMENT_TYPE)(nIndex+1);
	if(nIndex<5)
		return  CHAIN_SEGMENT_REVOLUTE_LINK;
	return CHAIN_SEGMENT_PRISMATIC_LINK;
}

// creates chain segment from data displayed on dialog
CChainSegment CKinematicChainPropertiesDialog::GetChainSegmentFromDialogData(void)
{
	UpdateData(TRUE);
	CHAIN_SEGMENT_TYPE type = GetSegmentTypeFromTypeIndex(m_nSegmentTypeIndex);
	
	CChainSegment segment(
				type,
				CR3Vector(m_dAxisX,m_dAxisY,m_dAxisZ),
				m_dAngle,
				CR3Vector(m_dTrX,m_dTrY,m_dTrZ),
				m_bParameterIsConstrained==TRUE,
				(type==CHAIN_SEGMENT_REVOLUTE_LINK ? degrees_to_radians(m_dParameterMinimum):m_dParameterMinimum),
				(type==CHAIN_SEGMENT_REVOLUTE_LINK ? degrees_to_radians(m_dParameterMaximum):m_dParameterMaximum));
	segment.SetPartVector(m_vBoxes);
	segment.HighlightPart(m_nActivePart);
	if(segment.GetType()==CHAIN_SEGMENT_REVOLUTE_LINK)
		segment.SetParameterValue(degrees_to_radians( m_dParameterValue));	//the dialog holds degrees, the segment radians
	else
		segment.SetParameterValue(m_dParameterValue);
	segment.SetName(m_sSegmentName.GetBuffer(0));
	//Only a TOOL terminal carries a holder, and a segment built without one would drop the holder of
	//the file the moment anything else on that segment was edited.
	if(type==CHAIN_SEGMENT_TOOL){
		const CToolHolder oHolder = ChainHolderFromDialog();
		if(!oHolder.IsEmpty())
			segment.SetToolHolder(oHolder);
	}
	return segment;
}

// changes actually selected (in the SegmentTree) segment with dialog data
void CKinematicChainPropertiesDialog::UpdateActualSegment(CWnd *pControlWnd,bool bRestoreFocus)
{
	static char str[51];
	bool bWasDot=false;
	if(pControlWnd){
		pControlWnd->GetWindowText(str,50);
		if(strlen(str)==0){
			pControlWnd->SetWindowText("0");
		}
}	

	HTREEITEM hItem = m_treeSegments.GetSelectedItem();
//	m_nOldIndex = (int)m_treeSegments.GetItemData(hItem);
	std::vector<CChainSegment> *pvVector;
	int nIndex;
	
	if(GetSegmentVectorFromTreeItem(hItem,&pvVector,nIndex)){
		assert(m_pDoc);
		CHAIN_SEGMENT_TYPE type = (*pvVector)[nIndex].GetType();
		if(!(*pvVector)[nIndex].IsRegularJoint()) {
			(*pvVector)[nIndex] = GetChainSegmentFromDialogData();	
			(*pvVector)[nIndex].SetType(type);
		} else {
			(*pvVector)[nIndex] = GetChainSegmentFromDialogData();
		}
	
		m_treeSegments.SetItemText(hItem,(*pvVector)[nIndex].GetTypeName().c_str());
		m_pDoc->UpdateAllViews(NULL);
	}
	assert(m_pDoc);
	if(pvVector == &m_pDoc->m_oChain.m_vMaterialHolderPart)
		m_pOldParentNode = &m_hMaterialHolderPart;
	else 
		m_pOldParentNode = &m_hToolPart;
	if(pControlWnd){
		if(bRestoreFocus)
			pControlWnd->SetFocus();
	}
}

void CKinematicChainPropertiesDialog::SetSegmentTypeComboValue(CHAIN_SEGMENT_TYPE type)
{
	m_nSegmentTypeIndex = (int) (type-1);
}

//The holder the three controls describe. A designation the catalogue knows brings its cylinders with
//it, so that this editor and a host reading the file later put the same solid on the machine; any
//other designation is this workshop's own holder, and then the length and the radius are the single
//cylinder somebody measured. Nothing typed at all is a chain that names no holder.
CToolHolder CKinematicChainPropertiesDialog::ChainHolderFromDialog() const
{
	const std::string sModel((LPCTSTR)m_sChainHolderModel);
	const CToolHolder oCatalogue = ToolHolderByModel(sModel);
	if(!oCatalogue.IsEmpty())
		return oCatalogue;
	CToolHolder oHolder(m_dChainHolderLength,m_dChainHolderRadius);
	oHolder.SetModel(sModel);
	return oHolder;
}

void CKinematicChainPropertiesDialog::UpdateChainHolderFields(const CChainSegment *pSegment)
{
	const bool bTool = pSegment && pSegment->GetType()==CHAIN_SEGMENT_TOOL;
	m_comboChainHolderModel.EnableWindow(bTool);
	m_editChainHolderLength.EnableWindow(bTool);
	m_editChainHolderRadius.EnableWindow(bTool);
	if(!bTool){
		m_sChainHolderModel = "";
		m_dChainHolderLength = 0.0;
		m_dChainHolderRadius = 0.0;
		UpdateData(FALSE);
		return;
	}
	const CToolHolder oHolder = pSegment->IsToolHolderDeclared()
		? pSegment->GetToolHolder() : CToolHolder();
	m_sChainHolderModel = oHolder.GetModel().c_str();
	m_dChainHolderLength = oHolder.GetLength();
	m_dChainHolderRadius = oHolder.GetRadius();
	//a holder of the catalogue is three cylinders, and its sizes are the catalogue's to give
	const bool bFromCatalogue = !ToolHolderByModel(oHolder.GetModel()).IsEmpty();
	m_editChainHolderLength.SetReadOnly(bFromCatalogue);
	m_editChainHolderRadius.SetReadOnly(bFromCatalogue);
	UpdateData(FALSE);
}

void CKinematicChainPropertiesDialog::OnCbnSelchangeComboChainHolderModel()
{
	//the text of a combo box is not exchanged yet when its selection changes, so take the item
	const int nIndex = m_comboChainHolderModel.GetCurSel();
	if(nIndex!=CB_ERR)
		m_comboChainHolderModel.GetLBText(nIndex,m_sChainHolderModel);
	const CToolHolder oHolder = ToolHolderByModel(std::string((LPCTSTR)m_sChainHolderModel));
	if(!oHolder.IsEmpty()){
		m_dChainHolderLength = oHolder.GetLength();
		m_dChainHolderRadius = oHolder.GetRadius();
	}
	m_editChainHolderLength.SetReadOnly(!oHolder.IsEmpty());
	m_editChainHolderRadius.SetReadOnly(!oHolder.IsEmpty());
	UpdateData(FALSE);
	UpdateActualSegment();
}

void CKinematicChainPropertiesDialog::OnCbnEditchangeComboChainHolderModel()
{
	UpdateData(TRUE);
	m_editChainHolderLength.SetReadOnly(FALSE);
	m_editChainHolderRadius.SetReadOnly(FALSE);
	UpdateActualSegment();
}

void CKinematicChainPropertiesDialog::OnEnChangeEditChainHolderLength()
{
	UpdateActualSegment(&m_editChainHolderLength);
}

void CKinematicChainPropertiesDialog::OnEnChangeEditChainHolderRadius()
{
	UpdateActualSegment(&m_editChainHolderRadius);
}

void CKinematicChainPropertiesDialog::OnCbnSelchangeComboUnit()
{
	//the unit says what every length of the chain is. Changing it changes none of the numbers, it
	//changes what they mean - which is what somebody who drew a machine in the wrong unit wants,
	//and the one thing a file cannot be saved without
	if(!m_pDoc)
		return;
	int n = m_comboUnit.GetCurSel();
	if(n<0 || n>=CHAIN_UNIT_COUNT)
		return;
	if(m_pDoc->m_oChain.GetUnit()==(CHAIN_UNIT)n)
		return;
	m_pDoc->m_oChain.SetUnit((CHAIN_UNIT)n);
	m_pDoc->SetModifiedFlag(TRUE);
	m_pDoc->UpdateAllViews(NULL);
}

void CKinematicChainPropertiesDialog::OnCbnSelchangeComboSegmentType()
{
	UpdateActualSegment();
	UpdateParameterSliderUnits();
	// TODO: Add your control notification handler code here
}

void CKinematicChainPropertiesDialog::OnEnChangeEditTrX()
{
	UpdateActualSegment(&m_editTrX);
}

void CKinematicChainPropertiesDialog::OnEnChangeEditTrY()
{
	UpdateActualSegment(&m_editTrY);
}

void CKinematicChainPropertiesDialog::OnEnChangeEditTrZ()
{
	UpdateActualSegment(&m_editTrZ);
}

void CKinematicChainPropertiesDialog::OnEnChangeEditAxisX()
{
	UpdateData(TRUE);
	if(CR3Vector(m_dAxisX,m_dAxisY,m_dAxisZ).IsZero()){
		m_editAxisX.SetWindowText("1");
	}
	UpdateActualSegment(&m_editAxisX);
}

void CKinematicChainPropertiesDialog::OnEnChangeEditAxisY()
{
	UpdateData(TRUE);
	if(CR3Vector(m_dAxisX,m_dAxisY,m_dAxisZ).IsZero()){
		m_editAxisY.SetWindowText("1");
	}
	UpdateActualSegment(&m_editAxisY);
}

void CKinematicChainPropertiesDialog::OnEnChangeEditAxisZ()
{
	UpdateData(TRUE);
	if(CR3Vector(m_dAxisX,m_dAxisY,m_dAxisZ).IsZero()){
		m_editAxisZ.SetWindowText("1");
	}
	UpdateActualSegment(&m_editAxisZ);	
}

void CKinematicChainPropertiesDialog::OnEnChangeEditAngle()
{
	UpdateActualSegment(&m_editAngle);
	
}

void CKinematicChainPropertiesDialog::OnCbnSelchangeComboPredefinedAngles()
{
	UpdateData(TRUE);
	int nSel = m_comboPredefinedAngles.GetCurSel();
	if(nSel<1)
		return;
	if(nSel<8){
		m_dAngle = ((double)(nSel-1)*15.0);
	} else {
		if(nSel==8)
			m_dAngle = (135.0);
		if(nSel==9)
			m_dAngle = (180.0);
		if(nSel==10)
			m_dAngle = (270.0);

	}
	UpdateData(FALSE);// TODO: Add your control notification handler code here
	UpdateActualSegment();
}

void CKinematicChainPropertiesDialog::OnEnKillfocusEditTrZ()
{
	UpdateActualSegment(&m_editTrZ,false);
}

void CKinematicChainPropertiesDialog::OnEnKillfocusEditTrY()
{
	UpdateActualSegment(&m_editTrY,false);
}

void CKinematicChainPropertiesDialog::OnEnKillfocusEditTrX()
{
	UpdateActualSegment(&m_editTrX,false);
}

void CKinematicChainPropertiesDialog::OnEnKillfocusEditAxisX()
{
	UpdateActualSegment(&m_editAxisX,false);
}

void CKinematicChainPropertiesDialog::OnEnKillfocusEditAxisY()
{
	UpdateActualSegment(&m_editAxisY,false);
}

void CKinematicChainPropertiesDialog::OnEnKillfocusEditAxisZ()
{
	UpdateActualSegment(&m_editAxisZ,false);
}

void CKinematicChainPropertiesDialog::OnEnKillfocusEditAngle()
{

	UpdateActualSegment(&m_editAngle,false);
}

void CKinematicChainPropertiesDialog::OnBnClickedBtnZeroTrX()
{
	m_editTrX.SetWindowText("0.0");// TODO: Add your control notification handler code here
}

void CKinematicChainPropertiesDialog::OnBnClickedBtnZeroTrY()
{
	m_editTrY.SetWindowText("0.0");// TODO: Add your control notification handler code here
}

void CKinematicChainPropertiesDialog::OnBnClickedBtnZeroTrZ()
{
	m_editTrZ.SetWindowText("0.0");
}

void CKinematicChainPropertiesDialog::OnBnClickedBtnZeroTr()
{
	OnBnClickedBtnZeroTrX();
	OnBnClickedBtnZeroTrY();
	OnBnClickedBtnZeroTrZ();
}


void CKinematicChainPropertiesDialog::OnEnChangeEditParameterMinimum()
{
	UpdateData();
	if(m_dParameterMinimum>m_dParameterMaximum)
		m_dParameterMaximum = m_dParameterMinimum ;
	UpdateData(FALSE);
	UpdateActualSegment(&m_editParameterMinimum,false);
	UpdateParameterSliderUnits();
}

void CKinematicChainPropertiesDialog::OnEnChangeEditParameterMaximum()
{
	UpdateData();
	if(m_dParameterMinimum>m_dParameterMaximum)
		 m_dParameterMinimum = m_dParameterMaximum;
	UpdateData(FALSE);
	UpdateActualSegment(&m_editParameterMaximum,false);
	UpdateParameterSliderUnits();
}

void CKinematicChainPropertiesDialog::OnBnClickedCheckConstraint()
{
	UpdateActualSegment(&m_checkParameterIsConstrained,false);
	m_editParameterMaximum.EnableWindow(m_bParameterIsConstrained);
	m_editParameterMinimum.EnableWindow(m_bParameterIsConstrained);
	UpdateParameterSliderUnits();

}

void CKinematicChainPropertiesDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	SetActiveDocument(m_pDoc);	
}

void CKinematicChainPropertiesDialog::OnEnChangeSegmentName()
{
	UpdateActualSegment(NULL,false);

}
void CKinematicChainPropertiesDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SetParameterValue(m_dParameterStart+( ((double)(m_sldParameter.GetPos())/100.0)*(m_dParameterEnd-m_dParameterStart)),m_sldParameter.GetPos());
}

void CKinematicChainPropertiesDialog::OnBnClickedBtnZeroParameter()
{
	SetParameterValue(( m_dParameterEnd + m_dParameterStart ) * 0.5 , 50 );
}

void CKinematicChainPropertiesDialog::SetParameterValue(double dNewValue , int nSliderPosition)
{
	CChainSegment * pSegment = m_pDoc->m_oChain.GetActiveSegment();
	if(pSegment && pSegment->IsRegularJoint()){
		m_dParameterValue = dNewValue;
		if(pSegment->GetType()==CHAIN_SEGMENT_REVOLUTE_LINK)
			pSegment->SetParameterValue(degrees_to_radians(m_dParameterValue));
		else
			pSegment->SetParameterValue(m_dParameterValue);
		m_sldParameter.SetPos(nSliderPosition);
		m_nSliderValue = nSliderPosition;
		UpdateData(FALSE);
		m_pDoc->UpdateAllViews(NULL);
	}  


}


void CKinematicChainPropertiesDialog::UpdateParameterSliderUnits(void)
{
	CChainSegment *pSegment = m_pDoc->m_oChain.GetActiveSegment();
	if(pSegment && pSegment->IsRegularJoint()){
		if(pSegment->IsParameterConstrained()){
			if(pSegment->GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				m_dParameterStart = radians_to_degrees( pSegment->m_dParameterValueMinimum);
				m_dParameterEnd = radians_to_degrees( pSegment->m_dParameterValueMaximum);
			} else{
				m_dParameterStart = pSegment->m_dParameterValueMinimum;
				m_dParameterEnd = pSegment->m_dParameterValueMaximum;
			}

		} else {
			if(pSegment->GetType()==CHAIN_SEGMENT_PRISMATIC_LINK){
				m_dParameterStart = -1;
				m_dParameterEnd = 1;
			} 
			if(pSegment->GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				m_dParameterStart = -180;
				m_dParameterEnd = 180;
			}
		}
		if(m_dParameterValue<m_dParameterEnd 
			&& m_dParameterValue>m_dParameterStart){
				double d = m_dParameterEnd - m_dParameterStart;
				int nSliderPosition = (int)((100.0*(m_dParameterValue-m_dParameterStart))/d);
				SetParameterValue(m_dParameterValue,nSliderPosition); 
		} else {
			if(m_dParameterValue<m_dParameterStart)
				//m_sldParameter.SetPos(0);			
				SetParameterValue(m_dParameterValue,0);
			if(m_dParameterValue>m_dParameterEnd)
				//m_sldParameter.SetPos(100);			
				SetParameterValue(m_dParameterValue,100);
				//*/
		}
		
		UpdateData(FALSE);
	}

}

void CKinematicChainPropertiesDialog::OnBnClickedBtnResetAllParams()
{
	assert(m_pDoc);
	if(!m_pDoc)
		return;
	m_pDoc->m_oChain.ZeroAllParameters();
	SetActiveDocument(m_pDoc);
	m_pDoc->UpdateAllViews(NULL);
}

void CKinematicChainPropertiesDialog::OnEnChangeEditParameterValue()
{
	UpdateActualSegment(&m_edtParameterValue,true);
	UpdateParameterSliderUnits();
	m_pDoc->UpdateAllViews(NULL);

}


void CKinematicChainPropertiesDialog::OnBnClickedCancel()
{
	CDialog::OnCancel();
}

void CKinematicChainPropertiesDialog::OnEnKillfocusEditSegmentName()
{
	UpdateData(TRUE);
	if(m_sSegmentName.GetLength()==0){
		MessageBox("You have to input non-empty segment name");
		m_editSegmentName.SetFocus();
	}
}

void CKinematicChainPropertiesDialog::OnLbnSelchangeListParts()
{
	m_nActivePart = m_listboxSegmentParts.GetCurSel();	//LB_ERR (-1) when nothing is selected
	if(m_pActiveSegment)
		m_pActiveSegment->HighlightPart(m_nActivePart);
	m_pDoc->UpdateAllViews(NULL);
}

void CKinematicChainPropertiesDialog::OnLbnDblclkListParts()
{
	OnLbnSelchangeListParts();
	if(m_pActiveSegment && m_nActivePart>=0 && m_nActivePart<(int)m_vBoxes.size()){
		CBoxDialog dlg;
		dlg.SetBox(m_vBoxes[m_nActivePart]);
		dlg.SetWindowAndBoxForUpdates(GetParent(),m_pActiveSegment->GetActivePartPointer());
		if(dlg.DoModal()==IDOK){
			m_vBoxes[m_nActivePart] = dlg.GetBox();
		}
		UpdateSegmentPartsList();

	}
}

void CKinematicChainPropertiesDialog::OnBnClickedButtonAddPart()
{
	if(m_pActiveSegment){
		m_vBoxes.push_back(CR3Box());
		m_pActiveSegment->SetPartVector(m_vBoxes);
		m_pActiveSegment->HighlightPart((int)m_vBoxes.size()-1);
		CBoxDialog dlg;
		dlg.SetWindowAndBoxForUpdates(GetParent(),m_pActiveSegment->GetActivePartPointer());
		if(dlg.DoModal()==IDOK){
			m_nActivePart = (int)m_vBoxes.size()-1;
			m_vBoxes = m_pActiveSegment->GetPartVector();
		} else {
			m_vBoxes.pop_back();
			m_pActiveSegment->SetPartVector(m_vBoxes);
			m_pActiveSegment->HighlightPart(m_nActivePart);
		}
		UpdateSegmentPartsList();
	}
}

void CKinematicChainPropertiesDialog::UpdateSegmentPartsList(void)
{
	m_listboxSegmentParts.ResetContent();
	if(m_pActiveSegment){
		for(size_t i=0;i<m_vBoxes.size();i++){
			std::string s("box_");
			s+=size_to_string(i+1);
			m_listboxSegmentParts.AddString(s.c_str());
		}
		m_listboxSegmentParts.SetCurSel(m_nActivePart);
		m_pActiveSegment->SetPartVector(m_vBoxes);
		m_pActiveSegment->HighlightPart(m_nActivePart);
		m_pDoc->UpdateAllViews(NULL);
	} 
}
void CKinematicChainPropertiesDialog::OnBnClickedButtonDeletePart()
{
	if(m_pActiveSegment){
		if(m_nActivePart>=0 && m_nActivePart<(int)m_vBoxes.size()){
			m_vBoxes.erase(m_vBoxes.begin()+m_nActivePart);
			//the selection stays at the same place, or on the new last box, or nowhere
			if(m_nActivePart>=(int)m_vBoxes.size())
				m_nActivePart = (int)m_vBoxes.size()-1;
			UpdateSegmentPartsList();
		}
	}
}
