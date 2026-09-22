#include "../stdafx.h"
#include "MillingToolBall.h"
#include "ArgsMyException.h"


namespace kin5ax {

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
	//the ball: its centre is m_dR above the tip, the tip m_dReferencePointZ below the reference point
	CR3Point p(0.0,0.0,m_dR - m_dReferencePointZ);
	if(box.HasIntersectionWithSphere(trToBoxFrame*p,m_dR))
		return true;
	//the shank: a cylinder from the equator of the ball to the end of the tool
	if(m_dCuttingHeight<=m_dR)
		return false;
	CTranslation moveToShank(0.0,0.0,m_dReferencePointZ-m_dR);
	CTranslation moveFromShank(0.0,0.0,m_dR-m_dReferencePointZ);
	return box.HasIntersectionWithOZCylinder(m_dCuttingHeight-m_dR,m_dRz,trToBoxFrame*moveFromShank,moveToShank*trToToolFrame);
}



//the ball with the centre m_dR above the tip, and from its equator up the shank of radius m_dRz
bool CMillingToolBall::IsInOZCuttingSpace(const CR3Point &p) const
{
	if(p.GetZ()<-dEPSILON)
		return false;
	if(p.GetZ()>m_dCuttingHeight+dEPSILON)
		return false;
	if((p-CR3Point(0.0,0.0,m_dR)).GetLengthSquare()<=m_dRSquare+dEPSILON)
		return true;
	if(p.GetZ()<m_dR-dEPSILON)
		return false;
	double dSq = p.GetX()*p.GetX()+p.GetY()*p.GetY();
	return dSq<=m_dRzSquare+dEPSILON;
}

} // namespace kin5ax
