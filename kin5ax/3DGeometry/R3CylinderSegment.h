#pragma once

#include "R3Axis.h"
#include "R3Box.h"
#include "Transformation.h"

#include <string>

/**
 * @file R3CylinderSegment.h
 * @brief A piece of a cylinder, kept as simple as a machine part needs: one axis, two ends, a
 * radius and a sector.
 *
 * The axis runs along the X, the Y or the Z of the frame the solid is given in, through a point of
 * that frame - so a barrel off to one side is still a cylinder and not a box. @ref GetBase and
 * @ref GetTop are coordinates ALONG that axis, read off the frame; the coordinate of the axis point
 * along the axis itself says nothing and is ignored.
 *
 * The sector is two angles about the axis, in degrees, measured from the first axis across it
 * towards the second (for a Z cylinder: from +X towards +Y). A sector of 360 degrees or more is the
 * whole cylinder; a smaller one is a wedge that includes the axis, the shape of a swivel cheek or
 * the segment a rotary table sweeps.
 *
 *     a Z cylinder, seen along the axis        and from the side
 *          +Y                                   top  ---------
 *           |     max_angle_deg                      |///////|
 *       ,---+---,     /                              |///////|
 *      / ///|///, \  /                               |///////|
 *     | //// \ ////|/                           base ---------
 *     +------+-----+-- +X                            axis point
 *      \ min_angle_deg
 */
namespace kin5ax {

class CR3CylinderSegment
{
public:
	/// An empty cylinder: no radius and no length, which meets nothing.
	CR3CylinderSegment();
	CR3CylinderSegment(R3_AXIS nAxis, const CR3Point &r3pAxisPoint, double dBase, double dTop
		,double dRadius, double dMinAngleDeg = 0.0, double dMaxAngleDeg = 360.0);

	R3_AXIS GetAxis() const { return m_nAxis; }
	/// Where the axis runs: only its two coordinates across the axis matter.
	const CR3Point & GetAxisPoint() const { return m_r3pAxisPoint; }
	double GetBase() const { return m_dBase; }		///< the lower coordinate along the axis
	double GetTop() const { return m_dTop; }		///< the upper one
	double GetRadius() const { return m_dRadius; }
	double GetMinAngleDegrees() const { return m_dMinAngleDeg; }
	double GetMaxAngleDegrees() const { return m_dMaxAngleDeg; }
	double GetLength() const { return m_dTop-m_dBase; }
	/// True when the sector is the whole turn, which is the usual case.
	bool IsWholeTurn() const;

	void SetAxis(R3_AXIS nAxis) { m_nAxis = nAxis; }
	void SetAxisPoint(const CR3Point &r3pAxisPoint) { m_r3pAxisPoint = r3pAxisPoint; }
	/// Keeps them in order, so that base is the lower coordinate whichever way round they come.
	void SetEnds(double dBase, double dTop);
	void SetRadius(double dRadius) { m_dRadius = fabs(dRadius); }
	/// Keeps the sector as min .. max going the positive way; equal angles are the whole turn.
	void SetSector(double dMinAngleDeg, double dMaxAngleDeg);

	/// No radius or no length: nothing to meet.
	bool IsEmpty() const;
	/// Why these numbers are not a cylinder, or an empty string when they are.
	std::string Problem() const;

	/// Is the point - in the frame the cylinder is given in - inside it or on it?
	bool Contains(const CR3Point &p) const;
	/// The smallest box of the frame that holds it, the sector taken into account.
	CR3Box GetBoundingBox() const;

	/**
	 * From the frame where this is the cylinder x^2+y^2 <= radius^2, 0 <= z <= GetLength(), to the
	 * frame the cylinder is given in. What CR3Box::HasIntersectionWithOZCylinder asks for; the
	 * sector is not in it, which is why that test is conservative for a wedge.
	 */
	CTransformation GetCanonicalTransformation() const;
	CTransformation GetInvertedCanonicalTransformation() const;
	/// The two ends of the axis itself, in the frame the cylinder is given in.
	CR3Point GetBasePoint() const;
	CR3Point GetTopPoint() const;

	bool operator==(const CR3CylinderSegment &o) const;
	bool operator!=(const CR3CylinderSegment &o) const { return !(*this==o); }

protected:
	R3_AXIS m_nAxis;
	CR3Point m_r3pAxisPoint;
	double m_dBase, m_dTop;
	double m_dRadius;
	double m_dMinAngleDeg, m_dMaxAngleDeg;
};

/// Is that angle, in degrees, inside the sector min .. max (the positive way round)?
bool IsAngleInSector(double dAngleDeg, double dMinAngleDeg, double dMaxAngleDeg);

} // namespace kin5ax
