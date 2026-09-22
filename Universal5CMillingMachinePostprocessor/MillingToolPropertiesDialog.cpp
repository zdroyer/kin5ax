// MillingToolPropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "MillingToolPropertiesDialog.h"


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
	,m_nToolShapeIndex(0)
	,m_pOldTool(0)
	,m_pDoc(0)
{

}

void CMillingToolPropertiesDialog::UpdateMillingTool()
{
	assert(m_pDoc);
	m_wndMillingToolPreview.SetMillingTool(0);
	if(m_pDoc->m_pMillingTool != m_pOldTool)
		delete m_pDoc->m_pMillingTool;
	
	switch(m_nToolShapeIndex){
		//ball
		case 0:
			m_pDoc->m_pMillingTool = new CMillingToolBall(m_dRz,m_dR
				,m_dCuttingPartEnd,m_dReferencePointHeight);
		break;
		//cylinder
		case 1:
			m_pDoc->m_pMillingTool = new CMillingToolCylinder(m_dRz,m_dR
				,m_dCuttingPartEnd,m_dReferencePointHeight);
		break;
		default:
			m_editH.EnableWindow(TRUE);
	}

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
	if(m_pOldTool != m_pDoc->m_pMillingTool)
		delete m_pOldTool;
	m_pDoc->UpdateAllViews(NULL);
	OnOK();
}

void CMillingToolPropertiesDialog::SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc *pDoc)
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
			m_dH = 0;
			
			m_dR = m_pOldTool->m_dR;
			m_dReferencePointHeight = m_pOldTool->m_dReferencePointZ;
			m_dRz = m_pOldTool->m_dRz;
		}
	}

}
void CMillingToolPropertiesDialog::OnBnClickedCancel()
{
	if(m_pOldTool != m_pDoc->m_pMillingTool){
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
		//ball
		case 0:
			m_editH.EnableWindow(FALSE);
			m_editR.EnableWindow(TRUE);
			m_dReferencePointHeight = m_dR;
		break;
		//cylinder
		case 1:
			m_editH.EnableWindow(FALSE);
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
