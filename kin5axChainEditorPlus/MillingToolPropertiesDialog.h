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
	//The designation of the holder: one of the catalogue, or anything the workshop calls its own. A
	//name the catalogue knows brings the shape with it, and then the length and the radius are that
	//holder's and not to be typed over; anything else is the one cylinder of the two fields.
	CString m_sHolderModel;
	CComboBox m_comboHolderModel;
	CEdit m_editHolderLength;
	CEdit m_editHolderRadius;
	//the holder the fields of the dialog describe
	CToolHolder HolderFromDialog() const;
	//shows the sizes of the holder that was picked, and lets them be typed only when it is nobody's
	void UpdateHolderFields();
	int m_nToolShapeIndex;

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
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeComboShape();
	afx_msg void OnCbnSelchangeComboHolderModel();
	afx_msg void OnCbnEditchangeComboHolderModel();
	afx_msg void OnEnChangeEditHolderLength();
	afx_msg void OnEnChangeEditHolderRadius();
	CEdit m_editH;
	CEdit m_editR;
};
