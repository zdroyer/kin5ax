#pragma once

#include "Solid.h"

/**
 * @brief OpenGL rendering of the solids a chain segment carries: a box, a piece of a cylinder, a
 * piece of a sphere (CSolid itself does not depend on OpenGL).
 *
 * Every solid is drawn in the frame it is given in - the frame of its segment - so the caller sets
 * that frame up and this draws the shape where its own numbers put it. The sector of a cylinder and
 * the cut of a sphere are drawn as they are, flanks and all, because a wedge drawn as a whole
 * cylinder would hide the very thing its numbers say.
 */
class CSolidRendererGL
{
public:
	static void RenderSolid(const CSolid &oSolid);
	static void RenderWire(const CSolid &oSolid, double dR = 1.0, double dG = 1.0, double dB = 1.0);

protected:
	static void RenderCylinder(const CR3CylinderSegment &oCylinder, bool bWire);
	static void RenderSphere(const CR3SphereSegment &oSphere, bool bWire);
	//the frame of the canonical cylinder - Z along the axis, the base at the origin - on the stack
	static void ApplyCanonicalFrame(const CR3CylinderSegment &oCylinder);
};
