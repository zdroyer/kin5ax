// Universal5CMillingMachinePostprocessorView.h : interface of the CUniversal5CMillingMachinePostprocessorView class
//


#pragma once
#include "3dviewmanipulator.h"


class CUniversal5CMillingMachinePostprocessorView : public CView
{
protected: // create from serialization only
	CUniversal5CMillingMachinePostprocessorView();
	DECLARE_DYNCREATE(CUniversal5CMillingMachinePostprocessorView)

// Attributes
public:
	CUniversal5CMillingMachinePostprocessorDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CUniversal5CMillingMachinePostprocessorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HGLRC m_hGLrc;
	void CreateGLRenderingContext(void);
	void InitGL(void);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
protected:
	// 3D OpenGL scene manipulator
	C3DViewManipulator m_o3DViewManipulator;
public:
	afx_msg void On3dviewOriginalOrienation();
	afx_msg void On3dviewOriginalView();
	afx_msg void On3dviewOriginalPosition();
	afx_msg void On3dviewOriginalScale();
	afx_msg void On3dviewZoomOut();
	afx_msg void On3dviewZoomIn();
	afx_msg void On3dviewOrthogonalProjection();
	afx_msg void On3dviewPerspectiveProjection();
	afx_msg void On3dviewOptimalSize();
};

#ifndef _DEBUG  // debug version in Universal5CMillingMachinePostprocessorView.cpp
inline CUniversal5CMillingMachinePostprocessorDoc* CUniversal5CMillingMachinePostprocessorView::GetDocument() const
   { return reinterpret_cast<CUniversal5CMillingMachinePostprocessorDoc*>(m_pDocument); }
#endif

