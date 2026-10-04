#pragma once

#include "millingtool.h"

//OpenGL rendering of milling tools (CMillingTool itself does not depend on OpenGL)
class CMillingToolRendererGL
{
public:
	static void Render(const CMillingTool &oTool);
protected:
	//the shank and the holder above the cutting part, in steel grey: they cut nothing
	static void RenderNonCuttingPart(const CMillingTool &oTool);
	static void RenderBall(const CMillingTool &oTool);
	static void RenderCylinder(const CMillingTool &oTool);
};
