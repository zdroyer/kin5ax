#pragma once

#include "afxwin.h"
#include "ChainSegment.h"
#include "ToolAssembly.h"
#include "Chain.h"
#include "MillingToolPreview.h"

#include <memory>

/**
 * @brief The default tool assembly of a machine: the holder hanging from the tool spindle and the
 * tool in it, with the two lengths that follow from them.
 *
 * What a chain file says is usually clamped in this spindle. Neither the holder nor the tool takes
 * part in the kinematics - what places a tool on a machine is the tool offset given at run time -
 * so this edits a default to offer, and shows the assembly drawn from the nose down to the tip, the
 * assembly length, and the offset from the nose to the point a cutter location programs.
 *
 * The fields are in millimetres, or in inches for a chain that is in inches: a machine file is in
 * metres as often as not, and nobody measures a holder in metres. Every field says its unit, and a
 * field nothing reads - the corner radius of a flat end mill - is greyed with its name and its unit.
 */
class CSpindleNoseDialog : public CDialog
{
	DECLARE_DYNAMIC(CSpindleNoseDialog)

public:
	CSpindleNoseDialog(CWnd *pParent = NULL);
	virtual ~CSpindleNoseDialog();

	enum { IDD = ID_TOOL_SPINDLE_NOSE_DIALOG };

	/// The terminal to edit, and the unit its chain is in; the fields are shown in mm or in inches.
	void SetTerminal(CChainSegment *pSegment, CHAIN_UNIT nUnit);

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
	CHAIN_UNIT m_nUnit;			//the unit of the chain, which the fields are converted from
	double m_dShown;			//how many of the shown unit one of the chain's is
	CMillingToolPreview m_wndPreview;
	std::unique_ptr<CMillingTool> m_pTool;		//what the preview draws; the dialog owns it
	CComboBox m_comboHolder;
	CComboBox m_comboShape;
	CComboBox m_comboReference;

	//the unit of the fields: "mm", or "in" for a chain in inches
	const char * ShownUnitName() const;
	double FromChain(double d) const { return d*m_dShown; }
	double ToChain(double d) const { return (m_dShown!=0.0) ? d/m_dShown : d; }

	//the holder and the tool of the fields, in the unit of the chain
	CToolHolder HolderFromDialog() const;
	CToolDescription ToolFromDialog() const;
	void DialogFromTerminal();
	//the lengths, the complaint and the picture, after anything was typed
	void UpdateTheSums();
	//a field nothing reads is greyed, with its name and its unit
	void EnableFields();
	//the corner radius a shape says it has: the ball of a ball nose tool, nothing on a flat end
	void SyncCornerRadius();
};
