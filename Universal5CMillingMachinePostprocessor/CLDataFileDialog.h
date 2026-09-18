#pragma once

#include "Universal5CMillingMachinePostprocessorDoc.h"
#include "afxwin.h"

// CCLDataFileDialog dialog

class CCLDataFileDialog : public CDialog
{
	DECLARE_DYNAMIC(CCLDataFileDialog)

public:
	CCLDataFileDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCLDataFileDialog();

// Dialog Data
	enum { IDD = ID_CLDATA_FILE_DIALOG };

	void SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc *pDoc);

protected:
	CString m_sOpenedFileTitle;
	CString m_sOpenedFilePath;
	CString m_sOriginalDialogTitle;
	void UpdateDialogTitle();
	void UpdateCutterLocationDataObject();
	void LoadCLFile(const CString &sFileName,const CString &sFilePath);
	CUniversal5CMillingMachinePostprocessorDoc *m_pDoc;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnBnClickedButtonChooseFile();

	CEdit m_editP;
	CEdit m_editV;
	CEdit m_editLineStatus;
	CEdit m_editLineNumber;
	CEdit m_editPPrim;
	CEdit m_editVPrim;
	
	double m_dScale;
	double m_dAxisX;
	double m_dAxisY;
	double m_dAxisZ;
	double m_dAngleDegrees;
	double m_dOffsetX;
	double m_dOffsetY;
	double m_dOffsetZ;


	afx_msg void OnBnClickedButtonReload();
	CListBox m_listboxFileContents;
	BOOL m_bShowOnlyErrors;
	afx_msg void OnBnClickedRadioShowAllLines();
	afx_msg void OnBnClickedRadioShowLinesWithError();
	afx_msg void OnLbnSelchangeListFileContents();
	BOOL m_bTurnAlsoPosition;
	afx_msg void OnEnChangeEditXOffset();
	afx_msg void OnEnChangeEditYOffset();
	afx_msg void OnEnChangeEditZOffset();
	afx_msg void OnEnChangeEditTurnAxisX();
	afx_msg void OnEnChangeEditTurnAxisY();
	afx_msg void OnEnChangeEditTurnAxisZ();
	afx_msg void OnEnChangeEditTurnAngle();
	afx_msg void OnBnClickedCheckTurnAlsoPosition();
	afx_msg void OnEnChangeEditScale();
	CEdit m_editXMax;
	CEdit m_editYMax;
	CEdit m_editZMax;
	CEdit m_editXMin;
	CEdit m_editYMin;
	CEdit m_editZMin;
	CEdit m_editLinesCount;
	CEdit m_editErrorsCount;
	BOOL m_bBoxTransformed;
	void UpdateBoundingBoxControls(void);
	void SetText(CWnd & window, double d);
	afx_msg void OnBnClickedRadioP();
	afx_msg void OnBnClickedRadioPPrim();
	afx_msg void OnBnClickedButtonProcess();
	BOOL m_bInvertOrientation;
	afx_msg void OnBnClickedCheckInvertOrientation();
};
