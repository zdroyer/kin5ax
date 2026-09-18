#pragma once
#include "millingtool.h"

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
		,double dReferencePointZ);
	virtual void Render3D(void) const ;
	virtual void Render2D(CPaintDC &dc) const;	
	virtual CR3Box GetCuttingPartBoundingBox() const;
	virtual bool CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const;

};
