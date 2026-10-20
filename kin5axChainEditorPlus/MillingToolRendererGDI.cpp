#include "StdAfx.h"
#include "MillingToolRendererGDI.h"

#include <vector>
#include <algorithm>

namespace {

//The outline of the cutting part in a plane through the axis: the right half of it, from the top of
//the cutting part down to the axis at the tip, as (radius, height above the tip) pairs. The shape of
//the tool is in here and nowhere else.
std::vector<std::pair<double,double> > CuttingProfile(const CMillingTool &oTool)
{
	std::vector<std::pair<double,double> > v;
	const double dRz = oTool.m_dRz;
	const double dHeight = oTool.m_dCuttingHeight;
	if(dRz<=0.0 || dHeight<=0.0)
		return v;
	const int nSteps = 24;
	switch(oTool.m_type){
	case MILLING_TOOL_BALL:
		{
			//a sphere of radius R at the tip, the shank of radius Rz running into it
			const double dR = std::max(oTool.m_dR,dRz);
			const double dMeet = dR-sqrt(std::max(0.0,dR*dR-dRz*dRz));	//where the two meet
			if(dHeight>dMeet)
				v.push_back(std::make_pair(dRz,dHeight));
			const double dTMeet = asin(std::min(1.0,dRz/dR));
			for(int i=0;i<=nSteps;i++){
				const double dT = dTMeet*(nSteps-i)/nSteps;
				v.push_back(std::make_pair(dR*sin(dT),dR-dR*cos(dT)));
			}
		}
		break;
	case MILLING_TOOL_TORUS:
		{
			//a cylinder with its bottom edge rounded by the corner radius
			const double dCorner = std::min(oTool.m_dR,dRz);
			const double dCentre = dRz-dCorner;
			v.push_back(std::make_pair(dRz,dHeight));
			if(dCorner<=0.0){
				v.push_back(std::make_pair(dRz,0.0));
			} else {
				if(dHeight>dCorner)
					v.push_back(std::make_pair(dRz,dCorner));
				for(int i=0;i<=nSteps;i++){
					const double dA = 0.5*dPi*i/nSteps;
					v.push_back(std::make_pair(dCentre+dCorner*cos(dA),dCorner-dCorner*sin(dA)));
				}
			}
		}
		break;
	default:
		//a flat end mill, and anything else this preview has no shape for
		v.push_back(std::make_pair(dRz,dHeight));
		v.push_back(std::make_pair(dRz,0.0));
		break;
	}
	v.push_back(std::make_pair(0.0,0.0));		//down to the axis at the tip
	return v;
}

//the four strokes of an X on the axis, which is how the reference point is marked
void MarkPoint(CDC &dc, int nX, int nZ)
{
	for(int i=0;i<2;i++)
		for(int j=0;j<2;j++){
			dc.MoveTo(nX,nZ);
			dc.LineTo(nX+(i?4:-4),nZ+(j?4:-4));
		}
}

} // namespace

void CMillingToolRendererGDI::Render(CDC &dc, const CMillingTool &oTool)
{
	CRect rt;
	dc.GetWindow()->GetClientRect(rt);
	rt.NormalizeRect();
	rt.MoveToXY(0,0);
	dc.Rectangle(rt);

	CPlacement oPlace;
	if(!Fit(dc,oTool,rt,oPlace)){
		dc.TextOut(4,4,CString("Nothing to draw: the assembly has no length and no radius"));
		return;
	}
	RenderHolder(dc,oTool,oPlace);
	RenderShank(dc,oTool,oPlace);
	RenderCuttingPart(dc,oTool,oPlace);
	RenderMarks(dc,oTool,oPlace,rt);
}

bool CMillingToolRendererGDI::Fit(CDC &dc, const CMillingTool &oTool, const CRect &rt, CPlacement &oOut)
{
	const double dLength = oTool.GetAssemblyLength();
	const double dWidest = std::max(oTool.m_oHolder.GetRadius()
		,std::max(oTool.m_dRz,oTool.m_dR));
	if(dLength<=0.0 || dWidest<=0.0 || rt.Width()<=0 || rt.Height()<=0)
		return false;
	const int nMargin = 12;
	const double dByWidth = (0.5*(double)rt.Width()-nMargin)/dWidest;
	const double dByHeight = ((double)rt.Height()-2.0*nMargin)/dLength;
	oOut.m_dScale = std::min(dByWidth,dByHeight);
	oOut.m_nX0 = rt.CenterPoint().x;
	oOut.m_nZ0 = rt.bottom-nMargin;
	return oOut.m_dScale>0.0;
}

void CMillingToolRendererGDI::RenderHolder(CDC &dc, const CMillingTool &oTool, const CPlacement &oPlace)
{
	const std::vector<CToolHolderSection> &vSections = oTool.m_oHolder.GetSections();
	if(vSections.empty())
		return;
	CBrush oBrush(RGB(170,170,180));
	CBrush *pOld = dc.SelectObject(&oBrush);
	//the sections are measured from the nose of the spindle downwards, the drawing from the tip up
	const double dNose = oTool.GetAssemblyLength();
	for(size_t i=0;i<vSections.size();i++){
		const double dRadius = vSections[i].GetRadius();
		const double dTop = dNose-oTool.m_oHolder.GetSectionTop(i);
		const double dBottom = dTop-vSections[i].GetLength();
		if(dRadius<=0.0)
			continue;						//a length without a radius is not a solid to show
		dc.Rectangle(oPlace.X(-dRadius),oPlace.Z(dTop),oPlace.X(dRadius),oPlace.Z(dBottom));
	}
	dc.SelectObject(pOld);
	oBrush.DeleteObject();
}

void CMillingToolRendererGDI::RenderShank(CDC &dc, const CMillingTool &oTool, const CPlacement &oPlace)
{
	if(oTool.m_dNonCuttingLength<=0.0 || oTool.m_dRz<=0.0)
		return;
	CBrush oBrush(RGB(205,205,215));
	CBrush *pOld = dc.SelectObject(&oBrush);
	dc.Rectangle(oPlace.X(-oTool.m_dRz),oPlace.Z(oTool.GetToolLength())
		,oPlace.X(oTool.m_dRz),oPlace.Z(oTool.m_dCuttingHeight));
	dc.SelectObject(pOld);
	oBrush.DeleteObject();
}

void CMillingToolRendererGDI::RenderCuttingPart(CDC &dc, const CMillingTool &oTool, const CPlacement &oPlace)
{
	const std::vector<std::pair<double,double> > vProfile = CuttingProfile(oTool);
	if(vProfile.size()<3)
		return;
	//the right half down to the tip, then the same points mirrored back up: one closed outline
	std::vector<CPoint> vPolygon;
	vPolygon.reserve(2*vProfile.size());
	for(size_t i=0;i<vProfile.size();i++)
		vPolygon.push_back(CPoint(oPlace.X(vProfile[i].first),oPlace.Z(vProfile[i].second)));
	for(size_t i=vProfile.size();i>0;i--)
		vPolygon.push_back(CPoint(oPlace.X(-vProfile[i-1].first),oPlace.Z(vProfile[i-1].second)));

	CBrush oBrush(RGB(255,128,0));
	CBrush *pOld = dc.SelectObject(&oBrush);
	dc.Polygon(&vPolygon[0],(int)vPolygon.size());
	dc.SelectObject(pOld);
	oBrush.DeleteObject();
}

void CMillingToolRendererGDI::RenderMarks(CDC &dc, const CMillingTool &oTool, const CPlacement &oPlace
	,const CRect &rt)
{
	const int nTip = oPlace.Z(0.0);
	const int nNose = oPlace.Z(oTool.GetAssemblyLength());
	dc.MoveTo(rt.left,nTip);
	dc.LineTo(rt.right,nTip);
	dc.TextOut(rt.left+4,nTip-16,CString("tip"));
	dc.MoveTo(rt.left,nNose);
	dc.LineTo(rt.right,nNose);
	dc.TextOut(rt.left+4,nNose+2,CString("nose"));

	//the point the cutter locations are programmed at, which hangs the tool offset below the nose
	const int nRef = oPlace.Z(oTool.m_dReferencePointZ);
	MarkPoint(dc,oPlace.m_nX0,nRef);
	dc.TextOut(oPlace.m_nX0+6,nRef-5,CString("ref."));

	CPen oPen(PS_DASH,1,(COLORREF)0);
	CPen *pOld = dc.SelectObject(&oPen);
	dc.MoveTo(oPlace.m_nX0,rt.top);
	dc.LineTo(oPlace.m_nX0,rt.bottom);
	dc.SelectObject(pOld);
	oPen.DeleteObject();
}
