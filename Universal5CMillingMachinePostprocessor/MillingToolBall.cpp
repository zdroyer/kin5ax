#include "StdAfx.h"
#include ".\millingtoolball.h"
#include "argsmyexception.h"


CMillingToolBall::CMillingToolBall
	(double dRz
	,double dR
	,double dCuttingHeight
	,double dReferencePointZ)
:CMillingTool
		( dRz
		, dR
		, dCuttingHeight
		, dReferencePointZ)

{
	m_type = MILLING_TOOL_BALL;
	//if(m_dR<m_dRz)
	//	MY_THROW(CArgsMyException,"R<Rz in ball nose milling tool");
}

/*
CMillingToolBall::~CMillingToolBall(void)
{
}

//*/

void CMillingToolBall::Render2D(CPaintDC &dc) const
{
	static CRect rt;
	dc.GetWindow()->GetClientRect(rt);
	rt.NormalizeRect();
	rt.MoveToXY(0,0);
	dc.Rectangle(rt);

	if(m_dR<m_dRz){
		dc.TextOut(1,1,CString("Invalid parameters:"));
		dc.TextOut(1,20,CString("      R < Rz       "));
		return;
	}
	int margin = 10;
	int x0 = rt.CenterPoint().x;
	int z0 = rt.bottom-margin;
	double dAspectRation = (double)rt.Width()/(double)rt.Height();
	double dScale;
	if(dAspectRation<0.33)
		dScale  = (0.5 * (double)(rt.Width()-margin))/m_dR;
	else 
		dScale = (0.25 * (double)(rt.Height()-margin))/m_dR;
	
	int nR = round(m_dR * dScale);
	int nRz = round(m_dRz * dScale);
	int nH = round(m_dCuttingHeight*dScale);


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

	int nRef = round(m_dReferencePointZ*dScale);

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

void CMillingToolBall::Render3D(void)const
{
	static GLUquadricObj * pQuadric = gluNewQuadric();
	glEnable(GL_LIGHTING);
	glColor3d(0.6,0.0,0.6);
	glTranslated(0.0,0.0, - m_dReferencePointZ);
	
	glTranslated(0.0,0.0,m_dR );
	gluSphere(pQuadric,m_dR,40,40);
	gluCylinder(pQuadric,m_dRz,m_dRz,m_dCuttingHeight-m_dR,40,40);

	glTranslated(0.0,0.0,-m_dR);
	glTranslated(0.0,0.0,  m_dReferencePointZ);
}


CR3Box CMillingToolBall::GetCuttingPartBoundingBox() const
{
	CR3Point o(m_r3pPosition + (m_r3vOrientation*(m_dR-m_dReferencePointZ)));
	CR3Box sphereBox(o,m_dR) ;
	if(m_dR>=m_dCuttingHeight)
		return  sphereBox;
	return sphereBox + AxialCylinderBoundingBox(m_dRz,-m_dReferencePointZ,m_dCuttingHeight-m_dReferencePointZ);
}

bool CMillingToolBall::CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const
{
 /*
	glColor3d(1.0,1.0,0.0);
	box.Render3DWire();
	glPushMatrix();
	glMultMatrixd(CTransformation(trToBoxFrame).toGLMatrix());
	GLUquadricObj *pQ = gluNewQuadric();
	gluSphere(pQ,m_dR,10,10);
	glPopMatrix();
//*/	
	CR3Point p(0.0,0.0,m_dR - m_dReferencePointZ);
	if(box.HasIntersectionWithSphere(trToBoxFrame*p,m_dR))
		return true;
	CTranslation moveToO(0.0,0.0,m_dReferencePointZ); 
	CTranslation moveFromO(0.0,0.0,-m_dReferencePointZ); 
	return box.HasIntersectionWithOZCylinder(m_dCuttingHeight-m_dR,m_dRz,trToBoxFrame*moveFromO,moveToO*trToToolFrame);
	//todo: implementation for cylinder
	return false;
}



bool CMillingToolBall::IsInOZCuttingSpace(const CR3Point &p) const
{
	if(p.GetZ()<-dEPSILON)
		return false;
	if(p.GetZ()>m_dCuttingHeight+dEPSILON)
		return false;
	if(p.GetZ()<2.0*m_dR+dEPSILON ){
		if((p-CR3Point(0.0,0.0,m_dR)).GetLengthSquare()<=m_dRSquare+dEPSILON)
			return true;
		return false;
	}
	double dSq = p.GetX()*p.GetX()+p.GetY()*p.GetY();
	if(dSq>m_dRzSquare+dEPSILON)
		return false;
	return true;
}