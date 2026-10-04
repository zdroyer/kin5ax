// SettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "SettingsDialog.h"
#include "double_tools.h"
#include ".\settingsdialog.h"


// CSettingsDialog dialog

extern CApplicationConfiguration oConfiguration;
extern CPostprocessorSettings oPostprocessorSettings;

IMPLEMENT_DYNAMIC(CSettingsDialog, CDialog)
CSettingsDialog::CSettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDialog::IDD, pParent)
	, m_nCalculationOrder((int)oPostprocessorSettings.m_nAnglesSolvingMethod)
	, m_nAlpha1Method((int)oPostprocessorSettings.m_nAlpha1Method)
	, m_nAlpha2Method((int)oPostprocessorSettings.m_nAlpha2Method)
	, m_dInternalPrecision(dEPSILON)
{
}

CSettingsDialog::~CSettingsDialog()
{
}

void CSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_CALCULATION_ORDER_1, m_nCalculationOrder);
	DDX_Radio(pDX, IDC_RADIO_ALPHA1_METHOD_1, m_nAlpha1Method);
	DDX_Radio(pDX, IDC_RADIO_ALPHA2_METHOD_1, m_nAlpha2Method);
	DDX_Text(pDX, IDC_EDIT_INTERNAL_PRECISION, m_dInternalPrecision);

}


BEGIN_MESSAGE_MAP(CSettingsDialog, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CSettingsDialog message handlers

void CSettingsDialog::OnBnClickedOk()
{
	UpdateData();
	//m_dInternalPrecision is shown for information: dEPSILON is a constant of the library (KnownBugs #23);
	//the former assignment only changed the application's private copy and never reached the solver
	oPostprocessorSettings.m_nAlpha1Method = (ALPHA1_METHOD)m_nAlpha1Method;
	oPostprocessorSettings.m_nAlpha2Method = (ALPHA2_METHOD)m_nAlpha2Method;
	oPostprocessorSettings.m_nAnglesSolvingMethod = (ANGLES_SOLVING_METHOD)m_nCalculationOrder;
	OnOK();
}
