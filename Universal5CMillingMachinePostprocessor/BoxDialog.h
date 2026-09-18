#pragma once
#include "r3box.h"

// CBoxDialog dialog

class CBoxDialog : public CDialog
{
	DECLARE_DYNAMIC(CBoxDialog)

public:
	CBoxDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBoxDialog();

// Dialog Data
	enum { IDD = ID_BOX_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_dX1;
	double m_dY1;
	double m_dZ1;
	double m_dX2;
	double m_dY2;
	double m_dZ2;
	double m_dD;
	CR3Box GetBox(void);
	void SetBox(const CR3Box &box);
	afx_msg void OnEnChangeEditD();
	afx_msg void OnEnKillfocusEdit();
	afx_msg void OnBnClickedButtonXPlus();
	afx_msg void OnBnClickedButtonXMinus();
	afx_msg void OnBnClickedButtonYPlus();
	afx_msg void OnBnClickedButtonYMinus();
	afx_msg void OnBnClickedButtonZPlus();
	afx_msg void OnBnClickedButtonZMinus();
	CWnd *m_pWindow;
	CR3Box *m_pBox;
	void SetWindowAndBoxForUpdates(CWnd * pWnd, CR3Box * pBox);
	void UpdateWindowAndBox(void);
	afx_msg void OnBnClickedButtonXEnlarge();
	afx_msg void OnBnClickedButtonXShrink();
	afx_msg void OnBnClickedButtonYEnlarge();
	afx_msg void OnBnClickedButtonYShrink();
	afx_msg void OnBnClickedButtonZEnlarge();
	afx_msg void OnBnClickedButtonZShrink();
	afx_msg void OnBnClickedButtonEnlarge();
	afx_msg void OnBnClickedButtonShrink();
	afx_msg void OnEnChangeEdit();
};
