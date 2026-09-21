#include "StdAfx.h"
#include "CutterLocationDataRendererGL.h"
#include "GLTools.h"

extern CApplicationConfiguration oConfiguration;

void CCutterLocationDataRendererGL::Render(const CCutterLocationData &oData, bool bFastRendering)
{
	CR3Point p;
	CR3Vector v;
	glDisable(GL_LIGHTING);
	if(!bFastRendering){
		glColor3d(0.6,0.0,0.6);
		for(size_t i = 0; i<oData.GetLinesCount(); i++){
			//transformed location of the lines with a cutter location only
			if(oData.GetLocation(p,v,i))
				CGLTools::LineArrow(p,v*(oConfiguration.m_dUnit*0.2));
		}
	}
	glColor3d(0.8,0.8,0.8);
	glBegin(GL_LINE_STRIP);
	for(size_t i = 0; i<oData.GetLinesCount(); i++)
		if(oData.GetLocation(p,v,i))
			glVertex3dv(p.toGL3dv());
	glEnd();
}
