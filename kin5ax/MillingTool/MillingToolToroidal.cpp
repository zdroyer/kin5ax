#include "../stdafx.h"
#include "MillingToolToroidal.h"
#include "ArgsMyException.h"

namespace kin5ax {

CMillingToolToroidal::CMillingToolToroidal
	(double dRz
	,double dCornerRadius
	,double dCuttingHeight
	,double dReferencePointZ
	,double dNonCuttingLength
	,const CToolHolder &oHolder)
:CMillingTool
		( dRz
		, dCornerRadius
		, dCuttingHeight
		, dReferencePointZ
		, dNonCuttingLength
		, oHolder)
{
	m_type = MILLING_TOOL_TORUS;
	CheckDimensions();
}

/**
 * The solid of revolution of a rectangle with one rounded corner: above the corner radius it is the
 * full cylinder, below it the cross-section narrows along the arc of the corner.
 */
bool CMillingToolToroidal::IsInOZCuttingSpace(const CR3Point &p) const
{
	if(p.GetZ()<-dEPSILON)
		return false;
	if(p.GetZ()>m_dCuttingHeight+dEPSILON)
		return false;
	const double dSq = p.GetX()*p.GetX()+p.GetY()*p.GetY();
	if(dSq>m_dRzSquare+dEPSILON)
		return false;
	if(p.GetZ()>=m_dR-dEPSILON)
		return true;						//above the corner: the whole cylinder cuts
	const double dInner = m_dRz-m_dR;		//the radius of the flat part of the tip
	const double d = sqrt(dSq);
	if(d<=dInner+dEPSILON)
		return true;						//over the flat bottom inside the corner circle
	//the quarter torus: the distance from the centre circle of the corner
	const double dAcross = d-dInner;
	const double dUp = m_dR-p.GetZ();
	return dAcross*dAcross+dUp*dUp<=m_dRSquare+dEPSILON;
}

CR3Box CMillingToolToroidal::GetCuttingPartBoundingBox() const
{
	return AxialCylinderBoundingBox(m_dRz,-m_dReferencePointZ,m_dCuttingHeight-m_dReferencePointZ);
}

/**
 * The cutting part against a box of a machine part, as the cylinder that encloses it.
 *
 * This is conservative at the rounded corner: a box that fits under the rounding, within the corner
 * radius of the bottom edge, is reported as met although the real cutter would clear it by at most
 * that radius. A collision check that errs this way refuses a pose it could have allowed, which is
 * the safe direction; the cut test above is exact, and that is what takes material off.
 *
 * @todo The exact test, a box against the quarter torus of the corner (KnownBugs #71).
 */
bool CMillingToolToroidal::CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const
{
	//the cylinder frame has its base at the tip, m_dReferencePointZ below the origin of the tool frame
	CTranslation moveToO(0.0,0.0,m_dReferencePointZ);
	CTranslation moveFromO(0.0,0.0,-m_dReferencePointZ);
	return box.HasIntersectionWithOZCylinder(m_dCuttingHeight,m_dRz,trToBoxFrame*moveFromO,moveToO*trToToolFrame);
}

double CMillingToolToroidal::DefaultZeroPointZ() const
{
	//the programmed point of a toroidal cutter is its tip
	return 0.0;
}

} // namespace kin5ax
