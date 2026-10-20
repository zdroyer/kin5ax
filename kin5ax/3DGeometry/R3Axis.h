#pragma once

#include "R3Point.h"
#include "R3Vector.h"

#include <string>

/**
 * @file R3Axis.h
 * @brief One of the three axes of a frame, and the two that lie across it.
 *
 * The solids a chain segment carries are kept simple on purpose: a cylinder runs along one axis of
 * the frame it is given in, a sphere is cut by a plane perpendicular to one. Naming that axis is
 * all the orientation they have, and this is the name.
 *
 * The two axes across one are given in cyclic order - X then Y across Z, Y then Z across X, Z then
 * X across Y - so that the three always form a right-handed frame. An angle about the axis is
 * measured from the first of them towards the second.
 */
namespace kin5ax {

enum R3_AXIS
{
	R3_AXIS_X = 0,
	R3_AXIS_Y,
	R3_AXIS_Z,
	R3_AXIS_COUNT
};

/// "x", "y", "z", as a chain file spells it; "x" for anything else.
const char *R3AxisName(R3_AXIS nAxis);
/// The axis of that spelling, which may be upper case; false when it is none of the three.
bool R3AxisFromName(const std::string &sName, R3_AXIS &nAxis);

/// The coordinate of the point along that axis.
double R3AxisComponent(const CR3Point &p, R3_AXIS nAxis);
double R3AxisComponent(const CR3Vector &v, R3_AXIS nAxis);
/// The unit vector of the axis.
CR3Vector R3AxisDirection(R3_AXIS nAxis);
/// The two axes across it, in cyclic order: X then Y across Z.
void R3CrossAxes(R3_AXIS nAxis, R3_AXIS &nFirst, R3_AXIS &nSecond);
/// A point from its coordinate along the axis and the two across it, in that cyclic order.
CR3Point R3PointFromAxes(R3_AXIS nAxis, double dAlong, double dFirst, double dSecond);

} // namespace kin5ax
