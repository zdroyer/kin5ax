#pragma once
#include "millingtoolpreview.h"
#include "kin5axChainEditorPlusDoc.h"

#include "afxwin.h"


// CMillingToolPropertiesDialog dialog

class CMillingToolPropertiesDialog : public CDialog
{
	DECLARE_DYNAMIC(CMillingToolPropertiesDialog)

public:
	CMillingToolPropertiesDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMillingToolPropertiesDialog();
	void UpdateMillingTool();
	//the shape the combo box stands for
	MILLING_TOOL_TYPE ToolType() const;
	//why the fields of the dialog do not make a tool, or an empty string when they do (KnownBugs #50)
	std::string ToolDimensionProblem() const;
	void SetActiveDocument(CKin5axChainEditorPlusDoc *pDoc);
// Dialog Data
	enum { IDD = ID_MILLING_TOOL_DIALOG };


protected:
	CKin5axChainEditorPlusDoc *m_pDoc;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CMillingToolPreview m_wndMillingToolPreview;

	/*
	double m_dNotCuttingAxialBegin;
	double m_dNotCuttingAxialEnd;
	double m_dNotCuttingAxialRadius;
	double m_dNotCuttingSideBegin;
	double m_dNotCuttingSideEnd;
	double m_dNotCuttingSideRadius;
//*/
	double m_dH;
	double m_dRz;
	double m_dR;
	double m_dCuttingPartEnd;
	double m_dReferencePointHeight;		//derived from the shape: the tip, or the centre of the ball
	double m_dNonCuttingLength;			//the shank between the cutting part and the holder
	double m_dHolderLength;
	double m_dHolderRadius;
	int m_nToolShapeIndex;
	//The holder the list stands for: 0 is the one cylinder the two fields above describe, and the
	//rest is the catalogue of the library, whose first entry is NO_HOLDER - a tool clamped straight
	//into the spindle, which is a holder in the data with no body.
	int m_nHolderIndex;
	//what that index means, in the unit of the chain: the catalogue is in metres
	CToolHolder HolderFromDialog() const;

	CMillingTool *m_pOldTool;


	afx_msg void OnEnChangeEditRz();
	afx_msg void OnEnChangeEditR();
	afx_msg void OnEnChangeEditH();
	afx_msg void OnEnChangeEditCuttingPartHeight();
	afx_msg void OnEnChangeEditReferencePointHeight();
/*
	afx_msg void OnEnChangeEditNotCuttingAxialRadius();
	afx_msg void OnEnChangeEditNotCuttingAxialEnd();
	afx_msg void OnEnChangeEditNotCuttingAxialBegin();
	afx_msg void OnEnChangeEditNotCuttingSideRadius();
	afx_msg void OnEnChangeEditNotCuttingSideEnd();
	afx_msg void OnEnChangeEditNotCuttingSideBegin();
//*/
	CEdit m_editCuttingPartHeight;
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnBnClickedCancel();
	virtual INT_PTR DoModal();
	afx_msg void OnCbnSelchangeComboShape();
	afx_msg void OnCbnSelchangeComboToolHolder();
	virtual BOOL OnInitDialog();
	CEdit m_editH;
	CEdit m_editR;
	CComboBox m_comboShape;
	CComboBox m_comboHolder;
	CEdit m_editHolderLength;
	CEdit m_editHolderRadius;
protected:
	//the shapes and the holders of the two lists: their order is this, not the alphabet's
	void FillTheLists();
	//a catalogue holder is shown but not edited; the custom one is the other way round
	void EnableHolderFields();
};
