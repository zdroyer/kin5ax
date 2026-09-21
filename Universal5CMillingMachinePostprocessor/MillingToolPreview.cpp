// MillingToolPreview.cpp : implementation file
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
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
	if(m_pMillingTool)
		CMillingToolRendererGDI::Render(dc,*m_pMillingTool);
	else
		dc.TextOut(0,0,"No tool defined");

}
