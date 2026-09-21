#include "StdAfx.h"
#include "MillingToolRendererGL.h"

void CMillingToolRendererGL::Render(const CMillingTool &oTool)
{
	switch(oTool.m_type){
		case MILLING_TOOL_BALL:
			RenderBall(oTool);
			break;
		case MILLING_TOOL_CYLINDER:
			RenderCylinder(oTool);
			break;
		default:
			//no 3D model for this tool type
			break;
	}
}

void CMillingToolRendererGL::RenderBall(const CMillingTool &oTool)
{
	static GLUquadricObj * pQuadric = gluNewQuadric();
	glEnable(GL_LIGHTING);
	glColor3d(0.6,0.0,0.6);
	glTranslated(0.0,0.0, - oTool.m_dReferencePointZ);
	
	glTranslated(0.0,0.0,oTool.m_dR );
	gluSphere(pQuadric,oTool.m_dR,40,40);
	gluCylinder(pQuadric,oTool.m_dRz,oTool.m_dRz,oTool.m_dCuttingHeight-oTool.m_dR,40,40);

	glTranslated(0.0,0.0,-oTool.m_dR);
	glTranslated(0.0,0.0,  oTool.m_dReferencePointZ);
}

void CMillingToolRendererGL::RenderCylinder(const CMillingTool &oTool)
{
	static GLUquadricObj * pQuadric = gluNewQuadric();
	glColor3d(0.6,0.0,0.6);
	glEnable(GL_LIGHTING);
	glTranslated(0.0,0.0, - oTool.m_dReferencePointZ);
	gluCylinder(pQuadric,oTool.m_dRz,oTool.m_dRz,oTool.m_dCuttingHeight,40,40);
	gluDisk(pQuadric,oTool.m_dRz,oTool.m_dRz,40,20);
	glTranslated(0.0,0.0,  oTool.m_dReferencePointZ);

}
