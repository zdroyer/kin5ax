#pragma once
#include "MillingTool.h"

//Ball nose tool: a ball of radius m_dR at the tip, then the shank of radius m_dRz up to m_dCuttingHeight.
namespace kin5ax {

class CMillingToolBall :
	public CMillingTool
{
protected:
		virtual bool IsInOZCuttingSpace(const CR3Point &p) const;
public:
	CMillingToolBall
		(double dRz
		,double dR
		,double dCuttingHeight
		,double dReferencePointZ
		,double dNonCuttingLength = 0.0
		,const CToolHolder &oHolder = CToolHolder()
		,double dGaugeLength = 0.0);
	virtual double DefaultZeroPointZ() const;
	virtual CR3Box GetCuttingPartBoundingBox() const;
	virtual bool CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const;

};

} // namespace kin5ax
