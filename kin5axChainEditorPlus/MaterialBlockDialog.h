#pragma once
#include "afxwin.h"
#include "kin5axChainEditorPlusDoc.h"


// CMaterialBlockDialog dialog

class CMaterialBlockDialog : public CDialog
{
	DECLARE_DYNAMIC(CMaterialBlockDialog)

public:
	CMaterialBlockDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMaterialBlockDialog();

// Dialog Data
	enum { IDD = ID_MATERIAL_DEFINITION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CKin5axChainEditorPlusDoc *m_pDoc;
	DECLARE_MESSAGE_MAP()
public:
	void SetActiveDocument(CKin5axChainEditorPlusDoc *pDoc);
	double m_dVoxelSize;
	double m_dSizeX;
	double m_dSizeY;
	double m_dSizeZ;
	double m_dPositionX;
	double m_dPositionY;
	double m_dPositionZ;
	CEdit m_edtMemoryUsage;
	afx_msg void OnEnChangeEditSizeX();
	afx_msg void OnEnChangeEditSizeY();
	afx_msg void OnEnChangeEditSizeZ();
	afx_msg void OnEnChangeEditVoxelSize();
	afx_msg void OnBnClickedOk();
protected:
	void UpdateMemoryUsage(void);
};
