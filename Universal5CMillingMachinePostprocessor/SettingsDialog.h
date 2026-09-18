#pragma once


// CSettingsDialog dialog

class CSettingsDialog : public CDialog
{
	DECLARE_DYNAMIC(CSettingsDialog)

public:
	CSettingsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsDialog();

// Dialog Data
	enum { IDD = ID_SETTINGS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_nCalculationOrder;
	int m_nAlpha1Method;
	int m_nAlpha2Method;
	double m_dInternalPrecision;
	afx_msg void OnBnClickedOk();
};
