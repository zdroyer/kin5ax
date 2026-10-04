#pragma once
#include "millingtoolpreview.h"
#include "Universal5CMillingMachinePostprocessorDoc.h"

#include "afxwin.h"


// CMillingToolPropertiesDialog dialog

class CMillingToolPropertiesDialog : public CDialog
{
	DECLARE_DYNAMIC(CMillingToolPropertiesDialog)

public:
	CMillingToolPropertiesDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMillingToolPropertiesDialog();
	void UpdateMillingTool();
	void SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc *pDoc);
// Dialog Data
	enum { IDD = ID_MILLING_TOOL_DIALOG };


protected:
	CUniversal5CMillingMachinePostprocessorDoc *m_pDoc;
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
	CEdit m_editH;
	CEdit m_editR;
};
