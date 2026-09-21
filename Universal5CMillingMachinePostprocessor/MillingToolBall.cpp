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
	CR3BoxRendererGL::RenderWire(box);
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