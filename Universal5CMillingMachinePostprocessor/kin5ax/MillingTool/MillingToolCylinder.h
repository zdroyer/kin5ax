#pragma once
#include "MillingTool.h"


class CMillingToolCylinder	:	
	public CMillingTool
{
protected:
		virtual bool IsInOZCuttingSpace(const CR3Point &p) const;
public:
	CMillingToolCylinder
		(double dRz
		,double dR
		,double dCuttingHeight
		,double dReferencePointZ);
	virtual CR3Box GetCuttingPartBoundingBox() const;
	virtual bool CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const;

};
