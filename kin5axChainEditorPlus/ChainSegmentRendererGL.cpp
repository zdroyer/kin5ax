#include "StdAfx.h"
#include "ChainSegmentRendererGL.h"
#include "GLTools.h"
#include "R3BoxRendererGL.h"
#include "SolidRendererGL.h"
#include "MillingToolRendererGL.h"
#include "ToolAssembly.h"
#include <memory>

extern CApplicationConfiguration oConfiguration;

void CChainSegmentRendererGL::RenderParts(const CChainSegment &oSegment)
{
	glColor3d(0.3,0.3,0.3);
	for(size_t i = 0 ; i<oSegment.GetPartVector().size();i++)
		if(i==oSegment.GetHighlightedPartIndex()){
			if(oSegment.IsPartInCollision(i)){
				glColor3d(1.0,0.0,0.0);
				CSolidRendererGL::RenderSolid(oSegment.GetPartVector()[i]);
				glColor3d(0.3,0.3,0.3);
			} else {
				glColor3d(0.6,0.6,0.6);	
				CSolidRendererGL::RenderSolid(oSegment.GetPartVector()[i]);
				glColor3d(0.3,0.3,0.3);
			}
		} else {
			if(oSegment.IsPartInCollision(i)){
				glColor3d(0.5,0.0,0.0);
				CSolidRendererGL::RenderSolid(oSegment.GetPartVector()[i]);
				glColor3d(0.3,0.3,0.3);
			} else {
				CSolidRendererGL::RenderSolid(oSegment.GetPartVector()[i]);
			}
		}


	

}
void CChainSegmentRendererGL::Render(const CChainSegment &oSegment, bool bDrawBigFrame)
{
	DrawArm(oSegment.m_r3vConstTranslation);
	ApplyConstantTransformation(oSegment);
	if(oConfiguration.m_bShowParts)
		RenderParts(oSegment);
	glEnable(GL_LIGHTING);
	switch(oSegment.m_type){
		case CHAIN_SEGMENT_REVOLUTE_LINK:
			RenderRevolute(oSegment,bDrawBigFrame);
			break;
		case CHAIN_SEGMENT_PRISMATIC_LINK:
			RenderPrismatic(oSegment,bDrawBigFrame);
			break;
		case CHAIN_SEGMENT_TABLE:
			RenderMaterialHolder(oSegment,bDrawBigFrame);
			break;
		case CHAIN_SEGMENT_TOOL_SPINDLE_NOSE:
			RenderToolHolder(oSegment,bDrawBigFrame);
			break;
		default:
			return;
	}
}

void CChainSegmentRendererGL::RenderRevolute(const CChainSegment &oSegment, bool bDrawBigFrame)
{
	if(bDrawBigFrame)
		CGLTools::DrawAxes(oConfiguration.m_dUnit,oConfiguration.m_dUnit);
	else{
		if(oConfiguration.m_bShowLinksFrames)
			CGLTools::DrawAxes(0.3,0.3*oConfiguration.m_dUnit);
	}
	if(oConfiguration.m_bShowLinks){
		static double *pdU = &oConfiguration.m_dUnit;
		static GLUquadricObj *pQObj = gluNewQuadric();
		glColor3d(0.0,0.5,0.5);
		glNormal3d(0.0,0.0,1.0);
		glTranslated(0.0,0.0,-0.001*(*pdU));
		gluDisk(pQObj,0.01*(*pdU),0.1*(*pdU),40,6);
		glTranslated(0.0,0.0,0.002*(*pdU));
		glNormal3d(0.0,0.0,-1.0);

		gluDisk(pQObj,0.01*(*pdU),0.1*(*pdU),40,6);
		glTranslated(0.0,0.0,-0.001*(*pdU));

		glColor3d(0.3,0.3,0.3);
		glTranslated(0.0,0.0,-0.002*(*pdU));
		glNormal3d(0.0,0.0,1.0);

		gluPartialDisk(pQObj,0.03*(*pdU),0.09*(*pdU),20,4,0.0,radians_to_degrees(-oSegment.m_dActualParameterValue));
		glTranslated(0.0,0.0,0.004*(*pdU));
		glNormal3d(0.0,0.0,-1.0);

		gluPartialDisk(pQObj,0.03*(*pdU),0.09*(*pdU),20,4,0.0,radians_to_degrees(-oSegment.m_dActualParameterValue));
		glTranslated(0.0,0.0,-0.002*(*pdU));
	}	
	glRotated(radians_to_degrees( oSegment.m_dActualParameterValue),0.0,0.0,1.0);
}
void CChainSegmentRendererGL::RenderPrismatic(const CChainSegment &oSegment, bool bDrawBigFrame)
{
	if(bDrawBigFrame)
		CGLTools::DrawAxes(oConfiguration.m_dUnit,oConfiguration.m_dUnit);
	else{
		if(oConfiguration.m_bShowLinksFrames)
			CGLTools::DrawAxes(0.3,0.3*oConfiguration.m_dUnit);
	}
	if(oConfiguration.m_bShowLinks){
		static double *pdU = &oConfiguration.m_dUnit;
		glColor3d(0.0,0.5,0.5);
		double dArmThicknessDiv2 = 0.05*(*pdU); 
		glDisable(GL_LIGHTING);
		glBegin(GL_LINE_LOOP);
			glVertex3d(-dArmThicknessDiv2,-dArmThicknessDiv2,0);
			glVertex3d(-dArmThicknessDiv2,dArmThicknessDiv2,0);
			glVertex3d(dArmThicknessDiv2,dArmThicknessDiv2,0);
			glVertex3d(dArmThicknessDiv2,-dArmThicknessDiv2,0);
		glEnd();
		glEnable(GL_LIGHTING);
		OZBox(dArmThicknessDiv2,dArmThicknessDiv2,oSegment.m_dActualParameterValue);
	}
	glTranslated(0.0,0.0,oSegment.m_dActualParameterValue);
	

}
//The terminal of the tool branch is the nose of the tool spindle, and what the chain file says is
//usually clamped in it hangs below: the holder, and the tool sticking out of it. This is the default
//assembly of the machine, not the tool of a run - that one the view draws at its own place - so it
//follows the same switch that shows a tool at all.
void CChainSegmentRendererGL::RenderToolHolder(const CChainSegment &oSegment, bool bDrawBigFrame)
{
	if(bDrawBigFrame || oConfiguration.m_bShowToolFrame)
		CGLTools::DrawAxes(oConfiguration.m_dUnit,oConfiguration.m_dUnit);
	else
		if(oConfiguration.m_bShowLinksFrames)
			CGLTools::DrawAxes(0.3,0.3*oConfiguration.m_dUnit);
	
	if(!oConfiguration.m_bShowTool)
		return;
	const CToolAssembly oAssembly(oSegment.GetToolHolder(),oSegment.GetTool());
	if(!oAssembly.HasBody())
		return;								//nothing in this spindle, or a holder with no radius
	std::unique_ptr<CMillingTool> pTool(oAssembly.MakeTool());
	glEnable(GL_LIGHTING);
	if(!pTool.get()){
		CMillingToolRendererGL::RenderHolder(oSegment.GetToolHolder());
		return;								//a holder waiting for a tool
	}
	//the renderer draws an assembly from the reference point of its tool, and that point hangs the
	//tool offset below the nose
	glTranslated(0.0,0.0,-oAssembly.GetToolOffset());
	CMillingToolRendererGL::Render(*pTool);
	glTranslated(0.0,0.0,oAssembly.GetToolOffset());

}
void CChainSegmentRendererGL::RenderMaterialHolder(const CChainSegment &oSegment, bool bDrawBigFrame)
{
	if(bDrawBigFrame || oConfiguration.m_bShowMaterialFrame){
		CGLTools::DrawAxes(oConfiguration.m_dUnit,oConfiguration.m_dUnit);
	} else {
		if(oConfiguration.m_bShowLinksFrames)
			CGLTools::DrawAxes(0.3,0.3*oConfiguration.m_dUnit);
	}
	glColor3d(0.15,0.15,0.3);
	if(!oConfiguration.m_bShowMaterialHolder)
		return;
	static double *pdU = &oConfiguration.m_dUnit;
	glBegin(GL_POLYGON);
		glNormal3d(0.0,0.0,1.0*(*pdU));
		glVertex3d(-0.5*(*pdU),-0.5*(*pdU),0.0);
		glVertex3d(-0.5*(*pdU),0.5*(*pdU),0.0);
		glVertex3d(0.5*(*pdU),0.5*(*pdU),0.0);
		glNormal3d(0.0,0.0,1.0);
		glVertex3d(0.5*(*pdU),-0.5*(*pdU),0.0);
	glEnd();
	
}
void CChainSegmentRendererGL::DrawArm(const CR3Vector &r3vEndOfArm)
{
	if(oConfiguration.m_bShowLinks){
		glColor3d(1.0,1.0,0.0);
		glBegin(GL_LINES);
			glVertex3dv(CR3Vector(0.0).toGL3dv());
			glVertex3d(r3vEndOfArm.GetX(),r3vEndOfArm.GetY(),r3vEndOfArm.GetZ());
		glEnd();
	}

}
void CChainSegmentRendererGL::ApplyConstantTransformation(const CChainSegment &oSegment)
{
glTranslated(oSegment.m_r3vConstTranslation.GetX()
							,oSegment.m_r3vConstTranslation.GetY()
							,oSegment.m_r3vConstTranslation.GetZ());

glRotated((oSegment.m_dConstRevoluteAngle)
					,oSegment.m_r3vConstRevoluteAxis.GetX()
					,oSegment.m_r3vConstRevoluteAxis.GetY()
					,oSegment.m_r3vConstRevoluteAxis.GetZ());
	
}

void CChainSegmentRendererGL::OZBox(double dXDiv2, double dYDiv2, double dZ)
{
	CR3Box box = CR3Box::BoxFromOZBox(dXDiv2*2.0,dYDiv2*2.0,dZ);
	CR3BoxRendererGL::RenderSolid(box);
	static double d = 1E-5;
	box.ExpandXYZ(d,d,d);
	CR3BoxRendererGL::RenderWire(box);	
	return;
	if(is_zero(dZ)){
		static double d = 1E-5;
		glShadeModel(GL_FLAT);
		glBegin(GL_QUADS);
			glNormal3d(0.0,0.0,-1.0);						
			glVertex3d(-dXDiv2,-dYDiv2,-d);
			glVertex3d(dXDiv2,-dYDiv2,-d);
			glVertex3d(dXDiv2,dYDiv2,-d);
			glVertex3d(-dXDiv2,dYDiv2,-d);
			glNormal3d(0.0,0.0,1.0);						
			glVertex3d(-dXDiv2,-dYDiv2,d);
			glVertex3d(dXDiv2,-dYDiv2,d);
			glVertex3d(dXDiv2,dYDiv2,d);
			glVertex3d(-dXDiv2,dYDiv2,d);
		glEnd();
		glShadeModel(GL_SMOOTH);
		return;
	}

	double dSign = 1.0;
	if(dZ<0.0)
		dSign=-1.0;
	glShadeModel(GL_FLAT);
	glBegin(GL_TRIANGLE_FAN);
		glNormal3d(-dSign,0.0,0.0);						
		glVertex3d(-dXDiv2,-dYDiv2,0.0);
		glVertex3d(-dXDiv2,-dYDiv2,dZ);
		glVertex3d(-dXDiv2,dYDiv2,dZ);
		glVertex3d(-dXDiv2,dYDiv2,0.0);
        glNormal3d(0.0,0.0,-dSign);
		glVertex3d(dXDiv2,dYDiv2,0.0);
		glVertex3d(dXDiv2,-dYDiv2,0.0);
		glNormal3d(0.0,-dSign,0.0);
		glVertex3d(dXDiv2,-dYDiv2,dZ);
		glVertex3d(-dXDiv2,-dYDiv2,dZ);			
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glNormal3d(1.0,0.0,0.0);						
		glVertex3d(dXDiv2,dYDiv2,dZ);
		glVertex3d(dXDiv2,dYDiv2,0.0);
		glVertex3d(dXDiv2,-dYDiv2,0.0);
		glVertex3d(dXDiv2,-dYDiv2,dZ);
        glNormal3d(0.0,0.0,dSign);
		glVertex3d(-dXDiv2,-dYDiv2,dZ);
		glVertex3d(-dXDiv2,dYDiv2,dZ);
		glNormal3d(0.0,1.0,0.0);
		glVertex3d(-dXDiv2,dYDiv2,0.0);
		glVertex3d(dXDiv2,dYDiv2,0.0);			
	glEnd();
	glShadeModel(GL_SMOOTH);
	return;
	
}
