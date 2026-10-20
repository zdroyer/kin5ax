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
		case MILLING_TOOL_TORUS:
			RenderToroidal(oTool);
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
		//the sections of a holder are measured from the nose of the spindle, which is the far end of
		//the assembly from the tip the drawing stands at
		glTranslated(0.0,0.0,oTool.GetAssemblyLength());
		RenderHolder(oTool.m_oHolder);
		glTranslated(0.0,0.0,-oTool.GetAssemblyLength());
	}
	glTranslated(0.0,0.0,oTool.m_dReferencePointZ);
}

void CMillingToolRendererGL::RenderHolder(const CToolHolder &oHolder)
{
	static GLUquadricObj * pQuadric = gluNewQuadric();
	glColor3d(0.55,0.55,0.60);
	const std::vector<CToolHolderSection> &vSections = oHolder.GetSections();
	for(size_t i=0;i<vSections.size();i++){
		const double dRadius = vSections[i].GetRadius();
		if(dRadius<=0.0)
			continue;						//a length without a radius holds the tool away, no more
		//each cylinder is drawn upwards from its own bottom end, and the disk closes it: the step
		//down to a narrower section below shows, and so does the bottom of the whole holder
		const double dBottom = oHolder.GetSectionTop(i)+vSections[i].GetLength();
		glTranslated(0.0,0.0,-dBottom);
		gluDisk(pQuadric,0.0,dRadius,40,20);
		gluCylinder(pQuadric,dRadius,dRadius,vSections[i].GetLength(),40,40);
		glTranslated(0.0,0.0,dBottom);
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

//The cutting part of a bull nose cutter: the flat of the tip, the quarter torus of the rounded edge
//and the cylinder above it. The corner radius is m_dR, so a corner radius equal to the tool radius
//draws the same solid as a ball nose one.
void CMillingToolRendererGL::RenderToroidal(const CMillingTool &oTool)
{
	static GLUquadricObj * pQuadric = gluNewQuadric();
	const double dCorner = std::min(oTool.m_dR,oTool.m_dRz);
	const double dCentre = oTool.m_dRz-dCorner;		//the radius the centre circle of the torus lies at
	glEnable(GL_LIGHTING);
	glColor3d(0.6,0.0,0.6);
	glTranslated(0.0,0.0,-oTool.m_dReferencePointZ);			//to the tip
	if(dCentre>0.0)
		gluDisk(pQuadric,0.0,dCentre,40,10);				//the flat the rounding leaves
	if(oTool.m_dCuttingHeight>dCorner){
		glTranslated(0.0,0.0,dCorner);
		gluCylinder(pQuadric,oTool.m_dRz,oTool.m_dRz,oTool.m_dCuttingHeight-dCorner,40,40);
		glTranslated(0.0,0.0,-dCorner);
	}
	if(dCorner>0.0){
		//a quarter circle from the side of the cylinder round to the flat, swept about the axis
		const int nAround = 40, nAlong = 10;
		for(int i=0;i<nAlong;i++){
			glBegin(GL_QUAD_STRIP);
			for(int j=0;j<=nAround;j++){
				const double dT = 2.0*dPi*j/nAround;
				for(int k=0;k<2;k++){
					const double dA = 0.5*dPi*(i+k)/nAlong;
					glNormal3d(cos(dA)*cos(dT),cos(dA)*sin(dT),-sin(dA));
					glVertex3d((dCentre+dCorner*cos(dA))*cos(dT)
						,(dCentre+dCorner*cos(dA))*sin(dT)
						,dCorner-dCorner*sin(dA));
				}
			}
			glEnd();
		}
	}
	glTranslated(0.0,0.0,oTool.m_dReferencePointZ);
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
