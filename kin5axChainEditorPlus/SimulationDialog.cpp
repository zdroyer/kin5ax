// SimulationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include ".\simulationdialog.h"
#include "mainfrm.h"


extern CApplicationConfiguration oConfiguration;

// CSimulationDialog dialog

IMPLEMENT_DYNAMIC(CSimulationDialog, CDialog)
CSimulationDialog::CSimulationDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSimulationDialog::IDD, pParent)
	, m_bSimulationStarted(false)
	, m_nRefreshPeriod((UINT)oConfiguration.m_nRefreshPeriod)
	, m_pDoc(0)
	, m_nInterval(0)
{
}

CSimulationDialog::~CSimulationDialog()
{
}

void CSimulationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_REFRESH_PERIOD, m_nRefreshPeriod);
	DDV_MinMaxUInt(pDX, m_nRefreshPeriod, 0, 2000000000);
	DDX_Control(pDX, IDC_BUTTON_PLAY_PAUSE, m_buttonPlayPause);
	DDX_Control(pDX, IDC_SPIN_REFRESH_PERIOD, m_spinRefreshPeriod);
	DDX_Text(pDX, IDC_EDIT_INTERVAL, m_nInterval);
}


BEGIN_MESSAGE_MAP(CSimulationDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_PAUSE, OnBnClickedButtonPlayPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, OnBnClickedButtonStop)
	ON_EN_CHANGE(IDC_EDIT_REFRESH_PERIOD, OnEnChangeEditRefreshPeriod)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_REFRESH_NOW, OnBnClickedButtonRefreshNow)
	ON_EN_CHANGE(IDC_EDIT_INTERVAL, OnEnChangeEditInterval)
END_MESSAGE_MAP()


// CSimulationDialog message handlers

void CSimulationDialog::OnBnClickedButtonPlayPause()
{
	if(m_bSimulationStarted){
		m_bSimulationStarted =false;
		m_buttonPlayPause.SetWindowText(">");
		((CMainFrame*)GetParent())->PauseSimulation();

	} else {
		m_bSimulationStarted = ((CMainFrame*)GetParent())->StartSimulation();
		if(m_bSimulationStarted)
			m_buttonPlayPause.SetWindowText("||");
		
	}
}

void CSimulationDialog::OnBnClickedButtonStop()
{
	((CMainFrame*)GetParent())->StopAndResetSimulation();
	m_buttonPlayPause.SetWindowText(">");
	m_bSimulationStarted=false;
}

void CSimulationDialog::SetActiveDocument(CKin5axChainEditorPlusDoc * pDocument)
{
	m_pDoc = pDocument;
}

void CSimulationDialog::OnEnChangeEditRefreshPeriod()
{
	if(m_pDoc){
		UpdateData(TRUE);
		oConfiguration.m_nRefreshPeriod = m_nRefreshPeriod;
	}
}

void CSimulationDialog::OnClose()
{
	((CMainFrame*)GetParent())->OnSimulationDialog();
}

void CSimulationDialog::OnBnClickedButtonRefreshNow()
{
	GetParent()->Invalidate();

}

BOOL CSimulationDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_spinRefreshPeriod.SetRange32(0,INT_MAX);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CSimulationDialog::OnEnChangeEditInterval()
{
	UpdateData(TRUE);
	oConfiguration.m_nSimulationInterval = abs((int)m_nInterval);
}
