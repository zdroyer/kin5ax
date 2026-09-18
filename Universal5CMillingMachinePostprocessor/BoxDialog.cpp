// BoxDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "BoxDialog.h"
#include ".\boxdialog.h"


// CBoxDialog dialog

IMPLEMENT_DYNAMIC(CBoxDialog, CDialog)
CBoxDialog::CBoxDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBoxDialog::IDD, pParent)
	, m_dX1(0.0)
	, m_dX2(0.1)
	, m_dY1(0.0)
	, m_dY2(0.1)
	, m_dZ1(0.0)
	, m_dZ2(0.1)
	, m_dD(0.1)
	, m_pBox(0)
	, m_pWindow(0)
{
}

CBoxDialog::~CBoxDialog()
{
}

void CBoxDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_X1, m_dX1);
	DDX_Text(pDX, IDC_EDIT_X2, m_dX2);
	DDX_Text(pDX, IDC_EDIT_Y1, m_dY1);
	DDX_Text(pDX, IDC_EDIT_Y2, m_dY2);
	DDX_Text(pDX, IDC_EDIT_Z1, m_dZ1);
	DDX_Text(pDX, IDC_EDIT_Z2, m_dZ2);
	DDX_Text(pDX, IDC_EDIT_D, m_dD);
}


BEGIN_MESSAGE_MAP(CBoxDialog, CDialog)
	ON_EN_CHANGE(IDC_EDIT_D, OnEnChangeEditD)
	ON_EN_KILLFOCUS(IDC_EDIT_X1, OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_X2, OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_Y1, OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_Y2, OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_Z1, OnEnKillfocusEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_Z2, OnEnKillfocusEdit)
	ON_BN_CLICKED(IDC_BUTTON_X_PLUS, OnBnClickedButtonXPlus)
	ON_BN_CLICKED(IDC_BUTTON_X_MINUS, OnBnClickedButtonXMinus)
	ON_BN_CLICKED(IDC_BUTTON_Y_PLUS, OnBnClickedButtonYPlus)
	ON_BN_CLICKED(IDC_BUTTON_Y_MINUS, OnBnClickedButtonYMinus)
	ON_BN_CLICKED(IDC_BUTTON_Z_PLUS, OnBnClickedButtonZPlus)
	ON_BN_CLICKED(IDC_BUTTON_Z_MINUS, OnBnClickedButtonZMinus)
	ON_BN_CLICKED(IDC_BUTTON_X_ENLARGE, OnBnClickedButtonXEnlarge)
	ON_BN_CLICKED(IDC_BUTTON_X_SHRINK, OnBnClickedButtonXShrink)
	ON_BN_CLICKED(IDC_BUTTON_Y_ENLARGE, OnBnClickedButtonYEnlarge)
	ON_BN_CLICKED(IDC_BUTTON_Y_SHRINK, OnBnClickedButtonYShrink)
	ON_BN_CLICKED(IDC_BUTTON_Z_ENLARGE, OnBnClickedButtonZEnlarge)
	ON_BN_CLICKED(IDC_BUTTON_Z_SHRINK, OnBnClickedButtonZShrink)
	ON_BN_CLICKED(IDC_BUTTON_ENLARGE, OnBnClickedButtonEnlarge)
	ON_BN_CLICKED(IDC_BUTTON_SHRINK, OnBnClickedButtonShrink)
	ON_EN_CHANGE(IDC_EDIT_X1, OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_X2, OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_Y1, OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_Y2, OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_Z1, OnEnChangeEdit)
	ON_EN_CHANGE(IDC_EDIT_Z2, OnEnChangeEdit)

END_MESSAGE_MAP()


// CBoxDialog message handlers


void CBoxDialog::SetBox(const CR3Box &box)
{
	m_dX1= box.GetMinPoint().GetX();
	m_dY1= box.GetMinPoint().GetY();
	m_dZ1= box.GetMinPoint().GetZ();
	m_dX2= box.GetMaxPoint().GetX();
	m_dY2= box.GetMaxPoint().GetY();
	m_dZ2= box.GetMaxPoint().GetZ();
}

CR3Box CBoxDialog::GetBox(void)
{
	return CR3Box(m_dX1,m_dX2,m_dY1,m_dY2,m_dZ1,m_dZ2);
}

void CBoxDialog::OnEnChangeEditD()
{
	UpdateData(TRUE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnEnKillfocusEdit()
{
	UpdateData(TRUE);

}

void CBoxDialog::OnBnClickedButtonYPlus()
{
	UpdateData(TRUE);
	m_dY1+=m_dD;
	m_dY2+=m_dD;
	UpdateData(FALSE);
	UpdateWindowAndBox();
}
void CBoxDialog::OnBnClickedButtonYMinus()
{
	UpdateData(TRUE);
	m_dY1-=m_dD;
	m_dY2-=m_dD;
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnBnClickedButtonXPlus()
{
	UpdateData(TRUE);
	m_dX1+=m_dD;
	m_dX2+=m_dD;
	UpdateData(FALSE);
	UpdateWindowAndBox();
}
void CBoxDialog::OnBnClickedButtonXMinus()
{
	UpdateData(TRUE);
	m_dX1-=m_dD;
	m_dX2-=m_dD;
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnBnClickedButtonZPlus()
{
	UpdateData(TRUE);
	m_dZ1+=m_dD;
	m_dZ2+=m_dD;
	UpdateData(FALSE);
	UpdateWindowAndBox();
}
void CBoxDialog::OnBnClickedButtonZMinus()
{
	UpdateData(TRUE);
	m_dZ1-=m_dD;
	m_dZ2-=m_dD;
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::SetWindowAndBoxForUpdates(CWnd * pWnd, CR3Box * pBox)
{
	m_pBox = pBox;
	m_pWindow = pWnd;
	UpdateWindowAndBox();
}

void CBoxDialog::UpdateWindowAndBox(void)
{
	if(m_pBox)
		m_pBox->SetBox(CR3Box(m_dX1,m_dX2,m_dY1,m_dY2,m_dZ1,m_dZ2));
	if(m_pWindow)
		m_pWindow->Invalidate();
}

void CBoxDialog::OnBnClickedButtonXEnlarge()
{
	if(m_dX1<=m_dX2){
		m_dX1-=m_dD;
		m_dX2+=m_dD;
	} else {
		m_dX1-=m_dD;
		m_dX2+=m_dD;
	}
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnBnClickedButtonXShrink()
{
	if(m_dX1<=m_dX2){
		m_dX1+=m_dD;
		m_dX2-=m_dD;
	} else {
		m_dX1+=m_dD;
		m_dX2-=m_dD;
	}
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnBnClickedButtonYEnlarge()
{
	if(m_dY1<=m_dY2){
		m_dY1-=m_dD;
		m_dY2+=m_dD;
	} else {
		m_dY1-=m_dD;
		m_dY2+=m_dD;
	}
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnBnClickedButtonYShrink()
{
	if(m_dY1<=m_dY2){
		m_dY1+=m_dD;
		m_dY2-=m_dD;
	} else {
		m_dY1+=m_dD;
		m_dY2-=m_dD;
	}
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnBnClickedButtonZEnlarge()
{
	if(m_dZ1<=m_dZ2){
		m_dZ1-=m_dD;
		m_dZ2+=m_dD;
	} else {
		m_dZ1-=m_dD;
		m_dZ2+=m_dD;
	}
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnBnClickedButtonZShrink()
{
	if(m_dZ1<=m_dZ2){
		m_dZ1+=m_dD;
		m_dZ2-=m_dD;
	} else {
		m_dZ1+=m_dD;
		m_dZ2-=m_dD;
	}
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnBnClickedButtonEnlarge()
{
	if(m_dZ1<=m_dZ2){
		m_dZ1-=m_dD;
		m_dZ2+=m_dD;
	} else {
		m_dZ1-=m_dD;
		m_dZ2+=m_dD;
	}
	if(m_dY1<=m_dY2){
		m_dY1-=m_dD;
		m_dY2+=m_dD;
	} else {
		m_dY1-=m_dD;
		m_dY2+=m_dD;
	}
	if(m_dX1<=m_dX2){
		m_dX1-=m_dD;
		m_dX2+=m_dD;
	} else {
		m_dX1-=m_dD;
		m_dX2+=m_dD;
	}
	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnBnClickedButtonShrink()
{
	if(m_dZ1<=m_dZ2){
		m_dZ1+=m_dD;
		m_dZ2-=m_dD;
	} else {
		m_dZ1+=m_dD;
		m_dZ2-=m_dD;
	}

	if(m_dY1<=m_dY2){
		m_dY1+=m_dD;
		m_dY2-=m_dD;
	} else {
		m_dY1+=m_dD;
		m_dY2-=m_dD;
	}
	if(m_dX1<=m_dX2){
		m_dX1+=m_dD;
		m_dX2-=m_dD;
	} else {
		m_dX1+=m_dD;
		m_dX2-=m_dD;
	}

	UpdateData(FALSE);
	UpdateWindowAndBox();
}

void CBoxDialog::OnEnChangeEdit()
{
	UpdateData(TRUE);
	UpdateWindowAndBox();
}
