#pragma once
#include "MillingTool.h"

/**
 * @file MillingToolToroidal.h
 * @brief A toroidal cutter: a flat end mill with its bottom edge rounded.
 */
namespace kin5ax {

/**
 * @brief Toroidal (bull nose) cutter: radius m_dRz, the bottom edge rounded by the corner radius m_dR.
 *
 * The cutting part is a cylinder of radius m_dRz from the tip up to m_dCuttingHeight, with the edge
 * at the tip replaced by a quarter torus of tube radius m_dR whose centre circle lies at radius
 * m_dRz - m_dR and at the height m_dR. A corner radius of zero is a flat end mill, a corner radius
 * equal to the tool radius is a ball nose; everything between is a bull nose.
 *
 * The programmed point is the tip by default. The centre of the corner circle is not a point the
 * library offers by name: a caller who programs there gives it as a custom reference height.
 */
class CMillingToolToroidal : public CMillingTool
{
protected:
	virtual bool IsInOZCuttingSpace(const CR3Point &p) const;
public:
	CMillingToolToroidal
		(double dRz
		,double dCornerRadius
		,double dCuttingHeight
		,double dReferencePointZ
		,double dNonCuttingLength = 0.0
		,const CToolHolder &oHolder = CToolHolder());
	/// The radius the bottom edge is rounded by.
	double GetCornerRadius() const { return m_dR; }
	virtual double DefaultZeroPointZ() const;
	virtual CR3Box GetCuttingPartBoundingBox() const;
	virtual bool CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const;
};

} // namespace kin5ax
