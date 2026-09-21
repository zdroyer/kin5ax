#pragma once

#include "millingtool.h"

//OpenGL rendering of milling tools (CMillingTool itself does not depend on OpenGL)
class CMillingToolRendererGL
{
public:
	static void Render(const CMillingTool &oTool);
protected:
	static void RenderBall(const CMillingTool &oTool);
	static void RenderCylinder(const CMillingTool &oTool);
};
