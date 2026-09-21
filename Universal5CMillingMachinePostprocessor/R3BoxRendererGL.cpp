#include "StdAfx.h"
#include "R3BoxRendererGL.h"

void CR3BoxRendererGL::RenderSolid(const CR3Box &box)
{
	double dMinX = box.GetMinPoint().GetX(), dMaxX = box.GetMaxPoint().GetX();
	double dMinY = box.GetMinPoint().GetY(), dMaxY = box.GetMaxPoint().GetY();
	double dMinZ = box.GetMinPoint().GetZ(), dMaxZ = box.GetMaxPoint().GetZ();
	GLboolean bWasLighting = glIsEnabled(GL_LIGHTING);
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glBegin(GL_QUADS);{
		//X-planes
		glNormal3d(1.0,0.0,0.0);
		glVertex3d(dMaxX,dMaxY,dMaxZ);
		glVertex3d(dMaxX,dMaxY,dMinZ);
		glVertex3d(dMaxX,dMinY,dMinZ);
		glVertex3d(dMaxX,dMinY,dMaxZ);

		glNormal3d(-1.0,0.0,0.0);
		glVertex3d(dMinX,dMaxY,dMaxZ);
		glVertex3d(dMinX,dMaxY,dMinZ);
		glVertex3d(dMinX,dMinY,dMinZ);
		glVertex3d(dMinX,dMinY,dMaxZ);

		//Y-planes
		glNormal3d(0.0,1.0,0.0);
		glVertex3d(dMaxX,dMaxY,dMaxZ);
		glVertex3d(dMaxX,dMaxY,dMinZ);
		glVertex3d(dMinX,dMaxY,dMinZ);
		glVertex3d(dMinX,dMaxY,dMaxZ);

		glNormal3d(0.0,-1.0,0.0);
		glVertex3d(dMaxX,dMinY,dMaxZ);
		glVertex3d(dMaxX,dMinY,dMinZ);
		glVertex3d(dMinX,dMinY,dMinZ);
		glVertex3d(dMinX,dMinY,dMaxZ);

		//Z-planes
		glNormal3d(0.0,0.0,1.0);
		glVertex3d(dMaxX,dMaxY,dMaxZ);
		glVertex3d(dMinX,dMaxY,dMaxZ);
		glVertex3d(dMinX,dMinY,dMaxZ);
		glVertex3d(dMaxX,dMinY,dMaxZ);

		glNormal3d(0.0,0.0,-1.0);
		glVertex3d(dMaxX,dMaxY,dMinZ);
		glVertex3d(dMinX,dMaxY,dMinZ);
		glVertex3d(dMinX,dMinY,dMinZ);
		glVertex3d(dMaxX,dMinY,dMinZ);
	}glEnd();
	if(!bWasLighting)
		glDisable(GL_LIGHTING);
}

void CR3BoxRendererGL::RenderWire(const CR3Box &box, double dR, double dG, double dB)
{
	double dMinX = box.GetMinPoint().GetX(), dMaxX = box.GetMaxPoint().GetX();
	double dMinY = box.GetMinPoint().GetY(), dMaxY = box.GetMaxPoint().GetY();
	double dMinZ = box.GetMinPoint().GetZ(), dMaxZ = box.GetMaxPoint().GetZ();
	GLboolean bWasLighting = glIsEnabled(GL_LIGHTING);
	glDisable(GL_LIGHTING);
	glColor3d(dR,dG,dB);
	glBegin(GL_LINE_LOOP);{
		glVertex3d(dMaxX,dMaxY,dMaxZ);
		glVertex3d(dMaxX,dMaxY,dMinZ);
		glVertex3d(dMaxX,dMinY,dMinZ);
		glVertex3d(dMaxX,dMinY,dMaxZ);
	} glEnd();
	glBegin(GL_LINE_LOOP);{
		glVertex3d(dMinX,dMaxY,dMaxZ);
		glVertex3d(dMinX,dMaxY,dMinZ);
		glVertex3d(dMinX,dMinY,dMinZ);
		glVertex3d(dMinX,dMinY,dMaxZ);
	} glEnd();
	glBegin(GL_LINES);{
		glVertex3d(dMinX,dMaxY,dMaxZ);
		glVertex3d(dMaxX,dMaxY,dMaxZ);
		glVertex3d(dMinX,dMaxY,dMinZ);
		glVertex3d(dMaxX,dMaxY,dMinZ);
		glVertex3d(dMinX,dMinY,dMinZ);
		glVertex3d(dMaxX,dMinY,dMinZ);
		glVertex3d(dMinX,dMinY,dMaxZ);
		glVertex3d(dMaxX,dMinY,dMaxZ);	
	}glEnd();
	if(bWasLighting)
		glEnable(GL_LIGHTING);
}
