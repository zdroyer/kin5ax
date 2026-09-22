	#pragma once
#include "afxwin.h"
#include "Universal5CMillingMachinePostprocessorDoc.h"

// CSingleInputSolverDialog dialog

class CSingleInputSolverDialog : public CDialog
{
	DECLARE_DYNAMIC(CSingleInputSolverDialog)

public:
	CSingleInputSolverDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSingleInputSolverDialog();

// Dialog Data
	enum { IDD = ID_SINGLE_INPUT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CUniversal5CMillingMachinePostprocessorDoc * m_pDoc;
	int m_nSolutionIndex;
	CListBox m_listboxSolutions;
	afx_msg void OnBnClickedBtnSolve();
	void SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc * pDocument);
protected:
	std::vector<tChainConfigurationWithResult> m_vConfigurations;
	double m_dX;
	double m_dY;
	double m_dZ;
	double m_dI;
	double m_dJ;
	double m_dK;

	double m_dXComputed;
	double m_dYComputed;
	double m_dZComputed;
	double m_dIComputed;
	double m_dJComputed;
	double m_dKComputed;

	void UpdateVirtualToolPosition();
public:
	afx_msg void OnLbnSelchangeListSolutions();
	afx_msg void OnLbnDblclkListSolutions();

	double m_dPositionError;
	double m_dOrientationError;
	afx_msg void OnBnClickedBtnSolve2();
	afx_msg void OnEnChangeEditZ();
	afx_msg void OnEnChangeEditY();
	afx_msg void OnEnChangeEditX();
	afx_msg void OnEnChangeEditI();
	afx_msg void OnEnChangeEditJ();
	afx_msg void OnEnChangeEditK();
};
