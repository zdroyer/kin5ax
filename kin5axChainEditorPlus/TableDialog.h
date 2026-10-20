#pragma once

#include "afxwin.h"
#include "ChainSegment.h"

/**
 * @brief The terminal of the workpiece branch: its name and the holder standing on the table.
 *
 * What a chain file says the table usually carries: a vice, a chuck, a tombstone, a plate - or
 * NO_HOLDER, which is the workpiece bolted to the table itself. A holder is a frame, where the
 * workpiece sits, and a few solids, which the tool must not run into. The frame is given the way a
 * segment gives its constant transformation: a translation and a rotation about an axis.
 *
 * The catalogue of the library is in metres, so an entry picked from the list is scaled into the
 * unit of the chain. CUSTOM_HOLDER is the one whose frame and boxes are typed in here.
 */
class CTableDialog : public CDialog
{
	DECLARE_DYNAMIC(CTableDialog)

public:
	CTableDialog(CWnd *pParent = NULL);
	virtual ~CTableDialog();

	enum { IDD = ID_TABLE_DIALOG };

	//the terminal to edit, and the unit of its chain in millimetres (the catalogue is metres)
	void SetTerminal(CChainSegment *pSegment, double dUnitInMillimetres);

protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnCbnSelchangeComboWorkpieceHolder();
	afx_msg void OnBnClickedButtonWpAddBox();
	afx_msg void OnBnClickedButtonWpDeleteBox();
	afx_msg void OnLbnDblclkListWpBoxes();
	afx_msg void OnBnClickedOk();

	CString m_sName;
	int m_nHolderIndex;			//the catalogue, whose first two entries are NO_HOLDER and CUSTOM_HOLDER
	double m_dTrX, m_dTrY, m_dTrZ;
	double m_dAxisX, m_dAxisY, m_dAxisZ;
	double m_dAngle;			//degrees, as the file carries it

protected:
	CChainSegment *m_pSegment;
	double m_dUnitInMillimetres;
	CComboBox m_comboHolder;
	CListBox m_listBoxes;
	std::vector<CR3Box> m_vBoxes;

	CWorkpieceHolder HolderFromDialog() const;
	void DialogFromTerminal();
	void UpdateTheBoxList();
	void EnableFields();
};
