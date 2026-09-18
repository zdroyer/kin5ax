#include "StdAfx.h"
#include ".\millingtoolcylinder.h"
#include "argsmyexception.h"


CMillingToolCylinder::CMillingToolCylinder
	(double dRz
	,double dR
	,double dCuttingHeight
	,double dReferencePointZ)
:CMillingTool
		( dRz
		, 0.0
		, dCuttingHeight
		, dReferencePointZ)

{
	m_type = MILLING_TOOL_CYLINDER;
}

void CMillingToolCylinder::Render2D(CPaintDC &dc) const
{
	static CRect rt;
	dc.GetWindow()->GetClientRect(rt);
	rt.NormalizeRect();
	rt.MoveToXY(0,0);
	dc.Rectangle(rt);

	int margin = 10;
	int x0 = rt.CenterPoint().x;
	int z0 = rt.bottom-margin;
	double dAspectRation = (double)rt.Width()/(double)rt.Height();
	double dScale;
	if(dAspectRation<(2.0*m_dRz/m_dCuttingHeight))
		dScale  = (0.5 * (double)(rt.Width()-margin))/m_dRz;
	else 
		dScale = ( (double)(rt.Height()-2*margin))/m_dCuttingHeight;
	
	int nRz = round_to_int(m_dRz * dScale);
	int nH = round_to_int(m_dCuttingHeight*dScale);


	dc.Rectangle(x0-nRz,z0,x0+nRz,margin);

	CBrush cuttingBrush(RGB(255,128,0));
	CBrush *pOldBrush = dc.SelectObject(&cuttingBrush);
	dc.Rectangle(x0-nRz,z0,x0+nRz,max(z0-nH,margin));
	dc.SelectObject(pOldBrush);
	cuttingBrush.DeleteObject();

	dc.MoveTo(0,z0);
	dc.LineTo(rt.Width(),z0);
	dc.TextOut(10,z0-8,"z=0");

	int nRef = round_to_int(m_dReferencePointZ*dScale);

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
bool CMillingToolCylinder::CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const
{
	CTranslation moveToO(0.0,0.0,m_dReferencePointZ); 
	CTranslation moveFromO(0.0,0.0,-m_dReferencePointZ); 
	return box.HasIntersectionWithOZCylinder(m_dCuttingHeight,m_dRz,trToBoxFrame*moveFromO,moveToO*trToToolFrame);
}

void CMillingToolCylinder::Render3D(void)const
{
	static GLUquadricObj * pQuadric = gluNewQuadric();
	glColor3d(0.6,0.0,0.6);
	glEnable(GL_LIGHTING);
	glTranslated(0.0,0.0, - m_dReferencePointZ);
	gluCylinder(pQuadric,m_dRz,m_dRz,m_dCuttingHeight,40,40);
	gluDisk(pQuadric,m_dRz,m_dRz,40,20);
	glTranslated(0.0,0.0,  m_dReferencePointZ);

}


CR3Box CMillingToolCylinder::GetCuttingPartBoundingBox() const
{
	return AxialCylinderBoundingBox(m_dRz,-m_dReferencePointZ,m_dCuttingHeight-m_dReferencePointZ);
}


bool CMillingToolCylinder::IsInOZCuttingSpace(const CR3Point &p) const
{
	if(p.GetZ()<-dEPSILON-m_dReferencePointZ)
		return false;
	if(p.GetZ()>m_dCuttingHeight-m_dReferencePointZ+dEPSILON)
		return false;
	double dSq = p.GetX()*p.GetX()+p.GetY()*p.GetY();
	if(dSq>m_dRzSquare+dEPSILON)
		return false;
	return true;
}