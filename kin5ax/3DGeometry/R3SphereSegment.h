#pragma once

#include "R3Axis.h"
#include "R3Box.h"

#include <string>

/**
 * @file R3SphereSegment.h
 * @brief A piece of a sphere: a centre, a radius, and two planes across one axis that cut it.
 *
 * The whole sphere is the usual case. The cut is what makes a dome of a swivel head or the ball end
 * of an arm: @ref GetAxis names the axis the cutting planes are perpendicular to, and @ref GetMin
 * and @ref GetMax are the coordinates along that axis, measured FROM THE CENTRE, between which the
 * sphere is kept. A half sphere is 0 .. radius; the whole one is -radius .. radius, which is what a
 * sphere with no cut given is.
 *
 *     a Z sphere cut at 0 .. radius            and one cut at -r .. r
 *          max = radius  ___                        ___
 *                      /////\                     /// \\\
 *          min = 0    -------                    ///   \\\
 *                                                 \\\_///
 */
namespace kin5ax {

class CR3SphereSegment
{
public:
	/// An empty sphere: no radius, which meets nothing.
	CR3SphereSegment();
	/// The whole sphere.
	CR3SphereSegment(const CR3Point &r3pCentre, double dRadius);
	/// The piece of it between the two coordinates along that axis, measured from the centre.
	CR3SphereSegment(const CR3Point &r3pCentre, double dRadius, R3_AXIS nAxis, double dMin
		,double dMax);

	const CR3Point & GetCentre() const { return m_r3pCentre; }
	double GetRadius() const { return m_dRadius; }
	R3_AXIS GetAxis() const { return m_nAxis; }
	double GetMin() const { return m_dMin; }	///< from the centre, along the axis
	double GetMax() const { return m_dMax; }
	/// True when the two cuts take nothing off, which is the usual case.
	bool IsWholeSphere() const;

	void SetCentre(const CR3Point &r3pCentre) { m_r3pCentre = r3pCentre; }
	/// Widens the cut with the radius when the sphere was whole, so that it stays whole.
	void SetRadius(double dRadius);
	void SetAxis(R3_AXIS nAxis) { m_nAxis = nAxis; }
	/// Keeps them in order and inside the sphere.
	void SetCut(double dMin, double dMax);

	/// No radius, or a cut that takes everything: nothing to meet.
	bool IsEmpty() const;
	/// Why these numbers are not a sphere, or an empty string when they are.
	std::string Problem() const;

	/// Is the point - in the frame the sphere is given in - inside it or on it?
	bool Contains(const CR3Point &p) const;
	/// The smallest box of the frame that holds it, the cut taken into account.
	CR3Box GetBoundingBox() const;

	bool operator==(const CR3SphereSegment &o) const;
	bool operator!=(const CR3SphereSegment &o) const { return !(*this==o); }

protected:
	CR3Point m_r3pCentre;
	double m_dRadius;
	R3_AXIS m_nAxis;
	double m_dMin, m_dMax;
};

} // namespace kin5ax
