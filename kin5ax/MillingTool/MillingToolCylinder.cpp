#include "../stdafx.h"
#include "MillingToolCylinder.h"
#include "ArgsMyException.h"


namespace kin5ax {

CMillingToolCylinder::CMillingToolCylinder
	(double dRz
	,double dR
	,double dCuttingHeight
	,double dReferencePointZ
	,double dNonCuttingLength
	,const CToolHolder &oHolder
	,double dGaugeLength)
:CMillingTool
		( dRz
		, 0.0
		, dCuttingHeight
		, dReferencePointZ
		, dNonCuttingLength
		, oHolder
		, dGaugeLength)

{
	m_type = MILLING_TOOL_CYLINDER;
	CheckDimensions();
}

bool CMillingToolCylinder::CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const
{
	//the cylinder frame has its base at the tip, m_dReferencePointZ below the origin of the tool frame
	CTranslation moveToO(0.0,0.0,m_dReferencePointZ); 
	CTranslation moveFromO(0.0,0.0,-m_dReferencePointZ); 
	return box.HasIntersectionWithOZCylinder(m_dCuttingHeight,m_dRz,trToBoxFrame*moveFromO,moveToO*trToToolFrame);
}


CR3Box CMillingToolCylinder::GetCuttingPartBoundingBox() const
{
	return AxialCylinderBoundingBox(m_dRz,-m_dReferencePointZ,m_dCuttingHeight-m_dReferencePointZ);
}


//the cylinder of radius m_dRz from the tip (the origin of this frame) to the end of the tool
bool CMillingToolCylinder::IsInOZCuttingSpace(const CR3Point &p) const
{
	if(p.GetZ()<-dEPSILON)
		return false;
	if(p.GetZ()>m_dCuttingHeight+dEPSILON)
		return false;
	double dSq = p.GetX()*p.GetX()+p.GetY()*p.GetY();
	if(dSq>m_dRzSquare+dEPSILON)
		return false;
	return true;
}

double CMillingToolCylinder::DefaultZeroPointZ() const
{
	//the programmed point of a flat end mill is its tip
	return 0.0;
}

} // namespace kin5ax
