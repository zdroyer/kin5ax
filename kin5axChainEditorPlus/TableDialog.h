#pragma once

#include "afxwin.h"
#include "ChainSegment.h"
#include "Chain.h"

/**
 * @brief The workpiece holder of a machine: what stands on its table and where the part sits in it.
 *
 * A vice, a chuck, a tombstone, a plate - or NO_HOLDER, which is the workpiece bolted to the table
 * itself. A holder is a frame, where the workpiece sits, and a few solids, which the tool must not
 * run into; the frame is given the way a segment gives its constant transformation, as a
 * translation and a rotation about an axis.
 *
 * **Its solids become solids of the table.** What the holder is made of is put into the solids of
 * the terminal when this dialog is closed with OK, so that it is drawn in the 3D view and checked
 * for collisions like everything else the machine is made of; picking another holder takes the
 * solids of the old one away again.
 *
 * A vice is not a block but two jaws, and the two numbers a workshop reads off one - how far apart
 * the jaws are and how long they are - build it. The fields are in millimetres, or in inches for a
 * chain that is in inches.
 */
class CTableDialog : public CDialog
{
	DECLARE_DYNAMIC(CTableDialog)

public:
	CTableDialog(CWnd *pParent = NULL);
	virtual ~CTableDialog();

	enum { IDD = ID_TABLE_DIALOG };

	/// The terminal to edit, and the unit its chain is in.
	void SetTerminal(CChainSegment *pSegment, CHAIN_UNIT nUnit);

protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnCbnSelchangeComboWorkpieceHolder();
	afx_msg void OnEnChangeViceSize();
	afx_msg void OnBnClickedButtonWpAddBox();
	afx_msg void OnBnClickedButtonWpDeleteBox();
	afx_msg void OnLbnDblclkListWpBoxes();
	afx_msg void OnBnClickedOk();

	CString m_sName;
	int m_nHolderIndex;			//the catalogue, whose first two entries are NO_HOLDER and CUSTOM_HOLDER
	double m_dTrX, m_dTrY, m_dTrZ;
	double m_dAxisX, m_dAxisY, m_dAxisZ;
	double m_dAngle;			//degrees, as the file carries it
	double m_dOpening;			//a vice: how far apart its jaws are
	double m_dJawLength;

protected:
	CChainSegment *m_pSegment;
	CHAIN_UNIT m_nUnit;
	double m_dShown;			//how many of the shown unit one of the chain's is
	CComboBox m_comboHolder;
	CListBox m_listBoxes;
	std::vector<CR3Box> m_vBoxes;		//in the unit of the chain, as the holder carries them

	const char * ShownUnitName() const;
	double FromChain(double d) const { return d*m_dShown; }
	double ToChain(double d) const { return (m_dShown!=0.0) ? d/m_dShown : d; }
	//the metres of the catalogue into the unit of the chain
	double FromMetres(double d) const;

	//true when the holder of the list is a vice, and which way its jaws open
	bool IsVice(R3_AXIS &nOpeningAxis) const;
	CWorkpieceHolder HolderFromDialog() const;
	void DialogFromTerminal();
	void UpdateTheBoxList();
	void EnableFields();
	//the solids of the terminal, with the ones of the old holder taken out and the new ones put in
	void PutTheSolidsOnTheTable(const CWorkpieceHolder &oNew);
};
