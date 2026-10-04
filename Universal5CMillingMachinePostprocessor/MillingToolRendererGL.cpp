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
	RenderNonCuttingPart(oTool);
}

void CMillingToolRendererGL::RenderNonCuttingPart(const CMillingTool &oTool)
{
	if(!oTool.HasNonCuttingPart())
		return;
	static GLUquadricObj * pQuadric = gluNewQuadric();
	glColor3d(0.55,0.55,0.60);
	glTranslated(0.0,0.0,-oTool.m_dReferencePointZ);		//to the tip
	if(oTool.m_dNonCuttingLength>0.0 && oTool.m_dRz>0.0){
		glTranslated(0.0,0.0,oTool.m_dCuttingHeight);
		gluCylinder(pQuadric,oTool.m_dRz,oTool.m_dRz,oTool.m_dNonCuttingLength,40,40);
		glTranslated(0.0,0.0,-oTool.m_dCuttingHeight);
	}
	if(oTool.m_oHolder.HasBody()){
		glTranslated(0.0,0.0,oTool.GetToolLength());
		gluDisk(pQuadric,0.0,oTool.m_oHolder.GetRadius(),40,20);
		gluCylinder(pQuadric,oTool.m_oHolder.GetRadius(),oTool.m_oHolder.GetRadius()
			,oTool.m_oHolder.GetLength(),40,40);
		glTranslated(0.0,0.0,-oTool.GetToolLength());
	}
	glTranslated(0.0,0.0,oTool.m_dReferencePointZ);
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
