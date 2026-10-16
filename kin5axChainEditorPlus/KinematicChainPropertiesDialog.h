	#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "kin5axChainEditorPlusDoc.h"



// CKinematicChainPropertiesDialog dialog

class CKinematicChainPropertiesDialog : public CDialog
{
	DECLARE_DYNAMIC(CKinematicChainPropertiesDialog)

public:
	CKinematicChainPropertiesDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CKinematicChainPropertiesDialog();

// Dialog Data
	enum { IDD = ID_KINEMATIC_CHAIN_PROPERTIES_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void OnBnClickedCancel();
	void SetActiveDocument(CKin5axChainEditorPlusDoc *pDoc);
	CKin5axChainEditorPlusDoc *m_pDoc;
	// tree with segments of kinematic chain
	CTreeCtrl m_treeSegments;
	// types of segment
	CComboBox m_comboSegmentType;
	// the unit every length of the chain is in: mm, m or in
	CComboBox m_comboUnit;

	//The tool holder the machine of this chain usually carries, which only a TOOL terminal has. It is
	//a default for whoever puts a tool in the spindle and for a host that has holders of its own to
	//match against; the end of the tool branch is the tool mount whether a holder is named or not.
	CComboBox m_comboChainHolderModel;
	CString m_sChainHolderModel;
	double m_dChainHolderLength;
	double m_dChainHolderRadius;
	CEdit m_editChainHolderLength;
	CEdit m_editChainHolderRadius;
	//the holder the three controls describe, empty when they are
	CToolHolder ChainHolderFromDialog() const;
	//shows the holder of that segment, and grays the controls for a segment that cannot carry one
	void UpdateChainHolderFields(const CChainSegment *pSegment);
	//puts the holder the chain names onto the tool of the document, so that the view shows it
	void ApplyChainHolderToTool();
	afx_msg void OnCbnSelchangeComboChainHolderModel();
	afx_msg void OnCbnEditchangeComboChainHolderModel();
	afx_msg void OnEnChangeEditChainHolderLength();
	afx_msg void OnEnChangeEditChainHolderRadius();

	// coordinates of initial translation of segment	
	double m_dTrX;
	double m_dTrY;
	double m_dTrZ;

	CChainSegment *m_pActiveSegment;
	void ForgetActiveSegment();

	CComboBox m_comboPredefinedAxes;
	CComboBox m_comboPredefinedAngles;

	// coordinates of initial revolute axis
	double m_dAxisX, m_dAxisY, m_dAxisZ;
	int m_nOldIndex;

	// fixed segment tree nodes
	HTREEITEM m_hToolPart,m_hMaterialHolderPart,*m_pOldParentNode;
	virtual BOOL OnInitDialog();
	afx_msg void OnTvnSelchangedSegmentTree(NMHDR *pNMHDR, LRESULT *pResult);
	CButton m_buttonAddSegment;
	CButton m_buttonDeleteSegment;
	afx_msg void OnBnClickedAddSegment();
	afx_msg void OnBnClickedDeleteSegment();
	afx_msg void OnBnClickedOk();
	CEdit m_editTrX;
	CEdit m_editTrY;
	CEdit m_editTrZ;
	CEdit m_editAxisX;
	CEdit m_editAxisY;
	CEdit m_editAxisZ;
	CEdit m_editAngle;
	// angle of initial revolute (in radians)
	double m_dAngle;
protected:
	// makes gray all controls concerned with chain segment
	void DisableSegmentProperties(void);
public:
	// enables and fills controls conserned with passed chain segment
	void EnableSegmentProperties(const CChainSegment & segment);
	bool GetSegmentVectorFromTreeItem(const HTREEITEM & hTreeItem,std::vector<CChainSegment> **ppvVectorSegment, int &nIndex);
	afx_msg void OnCbnSelchangeComboPredefinedAxis();
	// index of selected item in Segment Type combo
	int m_nSegmentTypeIndex;
	// translates index of selected valuea from SegmentTypeCombo into CHAIN_SEGMENT_TYPE
	CHAIN_SEGMENT_TYPE GetSegmentTypeFromTypeIndex(int nIndex);
	// creates chain segment from data displayed on dialog
	CChainSegment GetChainSegmentFromDialogData(void);
	// changes actually selected (in the SegmentTree) segment with dialog data
	void UpdateActualSegment(CWnd *pControlWnd=0,bool bRestoreFocus=true);
	void SetSegmentTypeComboValue(CHAIN_SEGMENT_TYPE type);
	afx_msg void OnCbnSelchangeComboSegmentType();
	afx_msg void OnCbnSelchangeComboUnit();
	afx_msg void OnEnChangeEditTrX();
	afx_msg void OnEnChangeEditTrY();
	afx_msg void OnEnChangeEditTrZ();
	afx_msg void OnEnChangeEditAxisX();
	afx_msg void OnEnChangeEditAxisY();
	afx_msg void OnEnChangeEditAxisZ();
	afx_msg void OnEnChangeEditAngle();
	afx_msg void OnCbnSelchangeComboPredefinedAngles();
	afx_msg void OnEnKillfocusEditTrZ();
	afx_msg void OnEnKillfocusEditTrY();
	afx_msg void OnEnKillfocusEditTrX();
	afx_msg void OnEnKillfocusEditAxisX();
	afx_msg void OnEnKillfocusEditAxisY();
	afx_msg void OnEnKillfocusEditAxisZ();
	afx_msg void OnEnKillfocusEditAngle();
	afx_msg void OnBnClickedBtnZeroTrX();
	afx_msg void OnBnClickedBtnZeroTrY();
	afx_msg void OnBnClickedBtnZeroTrZ();
	afx_msg void OnBnClickedBtnZeroTr();
//	CEdit m_editParameterOffset;
//	CEdit m_editParameterDefault;
	CButton m_checkParameterIsConstrained;
	CEdit m_editParameterMinimum;
	CEdit m_editParameterMaximum;
	//double m_dParameterOffset;
	//double m_dParameterDefault;
	BOOL m_bParameterIsConstrained;
	double m_dParameterMaximum;
	double m_dParameterMinimum;
	afx_msg void OnEnChangeEditParameterOffset();
	afx_msg void OnEnChangeEditParameterDefault();
	afx_msg void OnEnChangeEditParameterMinimum();
	afx_msg void OnEnChangeEditParameterMaximum();
	afx_msg void OnBnClickedCheckConstraint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CEdit m_editSegmentName;
	afx_msg void OnEnChangeSegmentName();
	//CString m_csSegmentName;
	int m_nSliderValue;
	CEdit m_edtParameterValue;
	double m_dParameterValue;
	double m_dParameterStart;
	double m_dParameterEnd;
	CSliderCtrl m_sldParameter;
	CButton m_btnZeroParameter;
	afx_msg void OnBnClickedBtnZeroParameter();
	void SetParameterValue(double dNewValue , int nSliderPosition);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	
	std::vector<CR3Box> m_vBoxes;
	int m_nActivePart;	//index into m_vBoxes, -1 when no box is selected
protected:
	void UpdateParameterSliderUnits(void);
	void UpdateSegmentPartsList(void);
public:
	afx_msg void OnBnClickedBtnResetAllParams();
	afx_msg void OnEnChangeEditParameterValue();
	CString m_sSegmentName;
	afx_msg void OnEnKillfocusEditSegmentName();
	CListBox m_listboxSegmentParts;
	afx_msg void OnLbnSelchangeListParts();
	afx_msg void OnLbnDblclkListParts();
	CButton m_buttonDeletePart;
	CButton m_buttonAddPart;
	afx_msg void OnBnClickedButtonAddPart();
	afx_msg void OnBnClickedButtonDeletePart();
};
