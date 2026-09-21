#pragma once

#include "millingtool.h"

//GDI (2D preview) rendering of milling tools (CMillingTool itself does not depend on GDI)
class CMillingToolRendererGDI
{
public:
	static void Render(CDC &dc, const CMillingTool &oTool);
protected:
	static void RenderBall(CDC &dc, const CMillingTool &oTool);
	static void RenderCylinder(CDC &dc, const CMillingTool &oTool);
};
