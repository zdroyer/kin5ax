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
	//The holder hangs from the tool mount, which is the gauge length above the tip and is where the
	//tool branch of the chain ends. It is up to three cylinders given from the mount downwards, so
	//they are drawn from the top down; when the tool was pulled further out of its holder than its
	//own solids account for, the gap between the shank and the holder is the truth and is shown.
	const double dMount = oTool.GetGaugeLength();
	const std::vector<CToolHolderSection> &vSections = oTool.m_oHolder.GetSections();
	for(size_t i=0;i<vSections.size();i++){
		const double dLength = vSections[i].GetLength();
		const double dRadius = vSections[i].GetRadius();
		if(dLength<=0.0 || dRadius<=0.0)
			continue;
		const double dBottom = dMount-oTool.m_oHolder.GetSectionTop(i)-dLength;
		glTranslated(0.0,0.0,dBottom);
		gluDisk(pQuadric,0.0,dRadius,40,20);
		gluCylinder(pQuadric,dRadius,dRadius,dLength,40,40);
		glTranslated(0.0,0.0,-dBottom);
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
