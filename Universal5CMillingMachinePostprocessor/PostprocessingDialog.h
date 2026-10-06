#pragma once

#include "Universal5CMillingMachinePostprocessorDoc.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "5cpostprocessor.h"

// CPostprocessingDialog dialog

class CPostprocessingDialog : public CDialog
{
	DECLARE_DYNAMIC(CPostprocessingDialog)

public:
	CPostprocessingDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPostprocessingDialog();

	void SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc *pDoc);


// Dialog Data
	enum { IDD = ID_POSTPROCESSING_DIALOG };

protected:
	std::string GenerateReportHead() const;
	std::vector<std::string> m_vsProblemsLogs;
	std::vector<tChainConfigurationVector> m_vSolutions; 

	CTime m_timeBegin, m_timeEnd;
	void UpdateProblemsCountControl();
	void SetText(CWnd & window, double d);
	void SetText(CWnd & window, size_t n);
	std::string m_sReport;
	CUniversal5CMillingMachinePostprocessorDoc *m_pDoc;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	std::vector<size_t> m_vTranslationsIndexes;
	std::vector<size_t> m_vRotationsIndexes;

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editLocationsCount;
	CEdit m_editTrajectoryLength;
	CEdit m_editTotalOrientationChange;
	CEdit m_editProblemsCount;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	double m_dPositionDifference;
	double m_dOrientationDifference;
	double m_dMaxPositionError;
	double m_dMaxOrientationErrorVectorLength;
	CR3AnchoredVector m_cvMaxPositionErrorInput;
	CR3AnchoredVector m_cvMaxOrientationErrorInput;
	//what the shortest path started from, for the report: a start is a convention of the control
	CTrajectoryStart m_oTrajectoryStart;
	CChainConfiguration m_oMaxPositionErrorSolution;
	CChainConfiguration m_oMaxOrientationErrorSolution;
	afx_msg void OnEnChangeEditOrientationDifference();
	afx_msg void OnEnChangeEditPositionDifference();
	afx_msg void OnBnClickedStart();
	CProgressCtrl m_progressPostprocessing;
	CEdit m_editReport;
	BOOL m_bDetailedReport;
	double m_dActualMaxPositionDifference;
	double m_dActualMaxOrientationDifferenceDegrees;
	size_t m_nTotalProblemsCount;
	std::vector<size_t> m_vProblemsByStatus;	//indexed by kin5ax::SOLVE_STATUS
	std::string m_sReportFile;
    
	void GenerateAndShowReport(void);
	CButton m_buttonStart;
	afx_msg void OnBnClickedButtonProgramSettings();
	CComboBox m_comboXName;
	CComboBox m_comboYName;
	CComboBox m_comboZName;
	virtual BOOL OnInitDialog();

	CComboBox m_comboAngle1Name;
	CComboBox m_comboAngle2Name;
	int m_nXNameIndex;
	int m_nYNameIndex;
	int m_nZNameIndex;
	int m_nAngle1NameIndex;
	int m_nAngle2NameIndex;
	afx_msg void OnBnClickedButtonSaveGcode();
	BOOL m_bRadians;
	CString m_sAngle1GCodeName;
	CString m_sAngle2GCodeName;
	CButton m_buttonSaveGCode;
	CComboBox m_comboAngle1Char;
	CComboBox m_comboAngle2Char;
	CButton m_radioDetailedReportContents0;
	CButton m_radioDetailedReportContents1;
	CButton m_radioDetailedReportContents2;
	int m_nDetailedReportContents;
	afx_msg void OnBnClickedCheckDetailedReport();
	CEdit m_editDetailedReportFileName;
	CButton m_buttonBrowseForDetailedReportFileName;
	afx_msg void OnBnClickedButtonBrowseForDetailedReportFileName();
};
