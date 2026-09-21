#include "StdAfx.h"
#include "MillingToolRendererGDI.h"

void CMillingToolRendererGDI::Render(CDC &dc, const CMillingTool &oTool)
{
	switch(oTool.m_type){
		case MILLING_TOOL_BALL:
			RenderBall(dc,oTool);
			break;
		case MILLING_TOOL_CYLINDER:
			RenderCylinder(dc,oTool);
			break;
		default:
			dc.TextOut(0,0,"No preview for this tool type");
	}
}

void CMillingToolRendererGDI::RenderBall(CDC &dc, const CMillingTool &oTool)
{
	static CRect rt;
	dc.GetWindow()->GetClientRect(rt);
	rt.NormalizeRect();
	rt.MoveToXY(0,0);
	dc.Rectangle(rt);

	if(oTool.m_dR<oTool.m_dRz){
		dc.TextOut(1,1,CString("Invalid parameters:"));
		dc.TextOut(1,20,CString("      R < Rz       "));
		return;
	}
	int margin = 10;
	int x0 = rt.CenterPoint().x;
	int z0 = rt.bottom-margin;
	double dAspectRatio = (double)rt.Width()/(double)rt.Height();
	double dScale;
	if(dAspectRatio<0.33)
		dScale  = (0.5 * (double)(rt.Width()-margin))/oTool.m_dR;
	else 
		dScale = (0.25 * (double)(rt.Height()-margin))/oTool.m_dR;
	
	int nR = round_to_int(oTool.m_dR * dScale);
	int nRz = round_to_int(oTool.m_dRz * dScale);
	int nH = round_to_int(oTool.m_dCuttingHeight*dScale);


	dc.Rectangle(x0-nRz,z0-nR,x0+nRz,margin);
	dc.Ellipse(x0-nR,z0,x0+nR,z0-2*nR);

	CBrush cuttingBrush(RGB(255,128,0));
	CBrush *pOldBrush = dc.SelectObject(&cuttingBrush);
	if(nH>2*nR)
		dc.Rectangle(x0-nRz,z0-nR,x0+nRz,max(z0-nH,margin));
	if(nH<2*nR){
		if(nH){
			if(nH>nR)
				dc.Rectangle(x0-nRz,z0-nR,x0+nRz,max(z0-nH-1,margin));
			int r = (int)sqrt((double)(2*nR*nH-nH*nH));
			dc.MoveTo(x0-r,z0-nH);
			dc.LineTo(x0+r,z0-nH);
			dc.Chord(CRect(x0-nR,z0-2*nR,x0+nR,z0),CPoint(x0-r,z0-nH),CPoint(x0+r,z0-nH));
			
		}
	} else {
		dc.Ellipse(x0-nR,z0,x0+nR,z0-2*nR);
	}
	dc.SelectObject(pOldBrush);
	cuttingBrush.DeleteObject();

	dc.MoveTo(0,z0);
	dc.LineTo(rt.Width(),z0);
	dc.TextOut(10,z0-8,"z=0");

	int nRef = round_to_int(oTool.m_dReferencePointZ*dScale);

	dc.MoveTo(x0,(z0-nRef));
	dc.LineTo(x0-4,(z0-nRef)-4);
	dc.MoveTo(x0,(z0-nRef));
	dc.LineTo(x0+4,(z0-nRef)+4);
	dc.MoveTo(x0,(z0-nRef));
	dc.LineTo(x0-4,(z0-nRef)+4);
	dc.MoveTo(x0,(z0-nRef));
	dc.LineTo(x0+4,(z0-nRef)-4);
	dc.TextOut(x0+6,z0-nRef-5,CString("ref."));


	CPen axialPen(PS_DASH,1,(COLORREF)0);
	CPen * oldPen = dc.SelectObject(&axialPen);
	dc.MoveTo(x0,0);
	dc.LineTo(x0,rt.bottom);
	dc.SelectObject(oldPen);
	axialPen.DeleteObject();
	

}

void CMillingToolRendererGDI::RenderCylinder(CDC &dc, const CMillingTool &oTool)
{
	static CRect rt;
	dc.GetWindow()->GetClientRect(rt);
	rt.NormalizeRect();
	rt.MoveToXY(0,0);
	dc.Rectangle(rt);

	int margin = 10;
	int x0 = rt.CenterPoint().x;
	int z0 = rt.bottom-margin;
	double dAspectRatio = (double)rt.Width()/(double)rt.Height();
	double dScale;
	if(dAspectRatio<(2.0*oTool.m_dRz/oTool.m_dCuttingHeight))
		dScale  = (0.5 * (double)(rt.Width()-margin))/oTool.m_dRz;
	else 
		dScale = ( (double)(rt.Height()-2*margin))/oTool.m_dCuttingHeight;
	
	int nRz = round_to_int(oTool.m_dRz * dScale);
	int nH = round_to_int(oTool.m_dCuttingHeight*dScale);


	dc.Rectangle(x0-nRz,z0,x0+nRz,margin);

	CBrush cuttingBrush(RGB(255,128,0));
	CBrush *pOldBrush = dc.SelectObject(&cuttingBrush);
	dc.Rectangle(x0-nRz,z0,x0+nRz,max(z0-nH,margin));
	dc.SelectObject(pOldBrush);
	cuttingBrush.DeleteObject();

	dc.MoveTo(0,z0);
	dc.LineTo(rt.Width(),z0);
	dc.TextOut(10,z0-8,"z=0");

	int nRef = round_to_int(oTool.m_dReferencePointZ*dScale);

	dc.MoveTo(x0,(z0-nRef));
	dc.LineTo(x0-4,(z0-nRef)-4);
	dc.MoveTo(x0,(z0-nRef));
	dc.LineTo(x0+4,(z0-nRef)+4);
	dc.MoveTo(x0,(z0-nRef));
	dc.LineTo(x0-4,(z0-nRef)+4);
	dc.MoveTo(x0,(z0-nRef));
	dc.LineTo(x0+4,(z0-nRef)-4);
	dc.TextOut(x0+6,z0-nRef-5,CString("ref."));


	CPen axialPen(PS_DASH,1,(COLORREF)0);
	CPen * oldPen = dc.SelectObject(&axialPen);
	dc.MoveTo(x0,0);
	dc.LineTo(x0,rt.bottom);
	dc.SelectObject(oldPen);
	axialPen.DeleteObject();
	

}
