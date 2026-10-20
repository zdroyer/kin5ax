#pragma once

#include "afxwin.h"
#include "Solid.h"

/**
 * @brief One solid of a chain segment: a box, a piece of a cylinder or a piece of a sphere.
 *
 * The kind is chosen at the top and the group of that kind is the one that is enabled; the other
 * two keep their numbers so that a kind can be changed back without typing them again. The solid of
 * the segment is written on every keystroke, the way the box dialog has always done it, so the 3D
 * view shows the shape while it is being measured; Cancel is the caller's business - it put the
 * solid in the segment and takes it out again.
 */
class CSolidDialog : public CDialog
{
	DECLARE_DYNAMIC(CSolidDialog)

public:
	CSolidDialog(CWnd *pParent = NULL);
	virtual ~CSolidDialog();

	enum { IDD = ID_SOLID_DIALOG };

	/// The solid the fields start from.
	void SetSolid(const CSolid &oSolid);
	/// What the fields say now.
	CSolid GetSolid() const;
	/// The solid to write into and the window to refresh on every keystroke; either may be null.
	void SetWindowAndSolidForUpdates(CWnd *pWnd, CSolid *pSolid);

protected:
	virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnCbnSelchangeComboSolidType();
	afx_msg void OnEnChangeAnySolidField();
	afx_msg void OnBnClickedCheckWholeTurn();
	afx_msg void OnBnClickedCheckWholeSphere();
	afx_msg void OnBnClickedOk();

	int m_nTypeIndex;				//SOLID_TYPE
	//the box
	double m_dX1, m_dX2, m_dY1, m_dY2, m_dZ1, m_dZ2;
	//the cylinder
	int m_nAxisIndex;				//R3_AXIS
	double m_dBase, m_dTop, m_dRadius;
	double m_dPX, m_dPY, m_dPZ;
	double m_dAngle1, m_dAngle2;
	BOOL m_bWholeTurn;
	//the sphere
	double m_dCX, m_dCY, m_dCZ;
	double m_dSphereRadius;
	int m_nCutAxisIndex;			//R3_AXIS
	double m_dCutMin, m_dCutMax;
	BOOL m_bWholeSphere;
	CString m_sProblem;

protected:
	CSolid *m_pSolid;
	CWnd *m_pWindow;
	CComboBox m_comboType;
	CComboBox m_comboAxis;
	CComboBox m_comboCutAxis;

	//the group of the chosen kind is the one that can be typed in
	void EnableFields();
	//the solid of the fields into the segment, and the view redrawn
	void UpdateWindowAndSolid();
};
