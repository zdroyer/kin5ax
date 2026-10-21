// MillingToolPreview.cpp : implementation file
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "MillingToolPreview.h"
#include ".\millingtoolpreview.h"
#include "MillingToolRendererGDI.h"


// CMillingToolPreview

IMPLEMENT_DYNAMIC(CMillingToolPreview, CWnd)
CMillingToolPreview::CMillingToolPreview()
:m_pMillingTool(NULL)
{
}

CMillingToolPreview::~CMillingToolPreview()
{
}


BEGIN_MESSAGE_MAP(CMillingToolPreview, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CMillingToolPreview message handlers

void CMillingToolPreview::SetMillingTool(CMillingTool *pMillingTool)
{
	m_pMillingTool = pMillingTool;
}

void CMillingToolPreview::OnPaint()
{
	CPaintDC dc(this);
	//a static window has no background brush of its own, so nothing is erased for us: whatever was
	//drawn last time would show through the next drawing, and a tool that became nothing would
	//leave its own picture behind
	CRect rt;
	GetClientRect(rt);
	dc.FillSolidRect(rt,::GetSysColor(COLOR_WINDOW));
	if(m_pMillingTool)
		CMillingToolRendererGDI::Render(dc,*m_pMillingTool);
	else
		dc.TextOut(4,4,"No tool defined");

}
