#pragma once

#include "afxwin.h"
#include "ChainSegment.h"
#include "ToolAssembly.h"
#include "MillingToolPreview.h"

#include <memory>

/**
 * @brief The terminal of the tool branch: its name, the holder hanging from it and the tool in it.
 *
 * What a chain file says is usually clamped in this spindle. Neither the holder nor the tool takes
 * part in the kinematics - what places a tool on a machine is the tool offset given at run time -
 * so this dialog edits a default to offer, and shows the two lengths that follow from it: the
 * assembly from the nose to the tip, and the offset from the nose to the point a cutter location
 * programs.
 *
 * The holder is picked from the catalogue of the library, whose first entry is NO_HOLDER - the tool
 * clamped straight into the spindle - or typed in as up to three cylinders. The catalogue is in
 * metres and the chain in its own unit, so a picked holder is scaled.
 */
class CSpindleNoseDialog : public CDialog
{
	DECLARE_DYNAMIC(CSpindleNoseDialog)

public:
	CSpindleNoseDialog(CWnd *pParent = NULL);
	virtual ~CSpindleNoseDialog();

	enum { IDD = ID_TOOL_SPINDLE_NOSE_DIALOG };

	//the terminal to edit, and the unit of its chain in millimetres (the catalogue is metres)
	void SetTerminal(CChainSegment *pSegment, double dUnitInMillimetres);

protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnCbnSelchangeComboNoseHolder();
	afx_msg void OnCbnSelchangeComboToolShape();
	afx_msg void OnCbnSelchangeComboToolReference();
	afx_msg void OnEnChangeAnyDimension();
	afx_msg void OnBnClickedOk();

	CString m_sName;
	int m_nHolderIndex;			//0: the cylinders below; the rest is the catalogue
	double m_dSectionLength[nTOOL_HOLDER_SECTIONS_MAX];
	double m_dSectionRadius[nTOOL_HOLDER_SECTIONS_MAX];
	int m_nShapeIndex;			//0: no tool at all; then ball nose, flat end, toroidal
	double m_dRadius;
	double m_dCornerRadius;
	double m_dCuttingHeight;
	double m_dToolLength;
	int m_nReferenceIndex;		//TOOL_REFERENCE_POINT
	double m_dReferenceHeight;
	CString m_sAssemblyLength;	//shown, never typed: what the fields above add up to
	CString m_sToolOffset;
	CString m_sProblem;

protected:
	CChainSegment *m_pSegment;
	double m_dUnitInMillimetres;
	//the assembly of the fields, drawn to scale from the nose of the spindle down to the tip
	CMillingToolPreview m_wndPreview;
	std::unique_ptr<CMillingTool> m_pTool;		//what the preview draws; the dialog owns it
	CComboBox m_comboHolder;
	CComboBox m_comboShape;
	CComboBox m_comboReference;
	CEdit m_editReferenceHeight;

	//the dialog fields as the library's own values, and the other way round
	CToolHolder HolderFromDialog() const;
	CToolDescription ToolFromDialog() const;
	void DialogFromTerminal();
	//the lengths and the complaint at the bottom, after anything was typed
	void UpdateTheSums();
	void EnableFields();
};
