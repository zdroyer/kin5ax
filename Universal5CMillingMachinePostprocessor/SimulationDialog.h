#pragma once
#include "afxwin.h"
#include "Universal5CMillingMachinePostprocessorDoc.h"
#include "afxcmn.h"


// CSimulationDialog dialog

class CSimulationDialog : public CDialog
{
	DECLARE_DYNAMIC(CSimulationDialog)

public:
	CSimulationDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSimulationDialog();

// Dialog Data
	enum { IDD = ID_SIMULATION_DIALOG };

protected:
	bool m_bSimulationStarted;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc * pDocument);
	CUniversal5CMillingMachinePostprocessorDoc * m_pDoc;
	afx_msg void OnBnClickedButtonPlayPause();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnEnChangeEditRefreshPeriod();
	UINT m_nRefreshPeriod;
	afx_msg void OnClose();
	CButton m_buttonPlayPause;
	afx_msg void OnBnClickedButtonRefreshNow();
	CSpinButtonCtrl m_spinRefreshPeriod;
	virtual BOOL OnInitDialog();
	UINT m_nInterval;
	afx_msg void OnEnChangeEditInterval();
};
