#pragma once

#include "millingtool.h"

//OpenGL rendering of milling tools (CMillingTool itself does not depend on OpenGL)
class CMillingToolRendererGL
{
public:
	static void Render(const CMillingTool &oTool);
	//The holder alone, hanging from the origin down its own -OZ: the frame of the spindle nose, which
	//is where the sections of a holder are measured from. A machine between jobs is this and no tool.
	static void RenderHolder(const CToolHolder &oHolder);
protected:
	//the shank and the holder above the cutting part, in steel grey: they cut nothing
	static void RenderNonCuttingPart(const CMillingTool &oTool);
	static void RenderBall(const CMillingTool &oTool);
	static void RenderCylinder(const CMillingTool &oTool);
	//a flat end mill with its bottom edge rounded by the corner radius: a quarter torus swept round
	static void RenderToroidal(const CMillingTool &oTool);
};
