#include "StdAfx.h"
#include "ChainRendererGL.h"
#include "GLTools.h"
#include "R3BoxRendererGL.h"
#include "ChainSegmentRendererGL.h"
#include "MillingToolRendererGL.h"
#include "CutterLocationDataRendererGL.h"
#include "MaterialRendererGL.h"

extern CApplicationConfiguration oConfiguration;

void CChainRendererGL::Render(const CChain &oChain
					  , bool bFastRendering
					  , CMaterial &oMaterial
					  , const CR3Vector &r3vMaterialObserver
					  , double dPixelSize
					  , const CMillingTool *pTool
					  , const CCutterLocationData &oData)
{
	glPushMatrix();
	if(oConfiguration.m_bFixedMaterialFrame){
		CTransformation m4x4T = oChain.GetActualToMaterialInvertedTransformation();
		glMultMatrixd(m4x4T.toGLMatrix());
		glPushMatrix();
	} 

	if(oConfiguration.m_bShowBase){
		glShadeModel(GL_FLAT);
		glEnable(GL_LIGHTING);
		glColor3d(0.0,0.0,0.3);
		glBegin(GL_QUADS);
			double d=-2E-3;
			glNormal3d(0.0,0.0,1.0);
			glVertex3d(-1.0*oConfiguration.m_dUnit,-1.0*oConfiguration.m_dUnit,d);
			glVertex3d(-1.0*oConfiguration.m_dUnit,1.0*oConfiguration.m_dUnit,d);
			glVertex3d(1.0*oConfiguration.m_dUnit,1.0*oConfiguration.m_dUnit,d);
			glVertex3d(1.0*oConfiguration.m_dUnit,-1.0*oConfiguration.m_dUnit,d);
		glEnd();
		glShadeModel(GL_SMOOTH);
	}
	if(oConfiguration.m_bShowBaseFrame)
		CGLTools::DrawAxes(1.0,2.2*oConfiguration.m_dUnit);

	for(size_t i=0;i<oChain.m_vMaterialHolderPart.size();i++){	
		CChainSegmentRendererGL::Render(oChain.m_vMaterialHolderPart[i],oChain.m_nActivePart!=CHAIN_PART_SPINDLE && i==oChain.m_nActiveSegmentIndex);
	}
	
	if(!oChain.m_toolPosition.m_v.IsZero()){
		glColor3d(0.6,0.6,0.0);
		static GLUquadricObj *pQObj = gluNewQuadric();
		glTranslated(oChain.m_toolPosition.m_p.GetX(),oChain.m_toolPosition.m_p.GetY(),oChain.m_toolPosition.m_p.GetZ());
		gluSphere(pQObj,0.02*oConfiguration.m_dUnit,20,20);
		glTranslated(-oChain.m_toolPosition.m_p.GetX(),-oChain.m_toolPosition.m_p.GetY(),-oChain.m_toolPosition.m_p.GetZ());
		CGLTools::Arrow(oChain.m_toolPosition,1.0,1.0,1.0);
	}
	if(pTool){
		if(oConfiguration.m_bShowToolBoundingBox){
			glColor3d(0.9,0.9,0.9);
			CR3BoxRendererGL::RenderWire(pTool->GetCuttingPartBoundingBox());
		}
		if(!bFastRendering){
			glColor3d(0.2,0.7,0.4);
//			CR3BoxRendererGL::RenderSolid(pTool->GetCuttingPartBoundingBox());
		}
	}
		

	if(oConfiguration.m_bShowMaterial)
		CMaterialRendererGL::Render(oMaterial,r3vMaterialObserver,dPixelSize,bFastRendering);
	if(oConfiguration.m_bShowMaterialContour)
		CMaterialRendererGL::RenderContour(oMaterial);
	if(oConfiguration.m_bShowToolTrajectory)
		CCutterLocationDataRendererGL::Render(oData,bFastRendering);
	glPopMatrix();
	glPushMatrix();
	for(size_t i=0;i<oChain.m_vToolPart.size();i++){
		CChainSegmentRendererGL::Render(oChain.m_vToolPart[i],oChain.m_nActivePart==CHAIN_PART_SPINDLE && i==oChain.m_nActiveSegmentIndex);
	}
	if(pTool && oConfiguration.m_bShowTool){
		//the frame after the segments is the tool mount; the tool hangs below it
		glTranslated(0.0,0.0,-oChain.GetToolOffset());
		CMillingToolRendererGL::Render(*pTool);
	}

	glPopMatrix();	
	if(oConfiguration.m_bFixedMaterialFrame)
		glPopMatrix();
}
