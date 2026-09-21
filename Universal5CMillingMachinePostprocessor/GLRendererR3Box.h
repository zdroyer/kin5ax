#pragma once

#include "3DGeometry/r3box.h"

//OpenGL rendering of CR3Box (CR3Box itself does not depend on OpenGL)
class CGLRendererR3Box
{
public:
	static void RenderWire(const CR3Box &box, double dR=1.0, double dG=1.0, double dB=1.0);
	static void RenderSolid(const CR3Box &box);
};
