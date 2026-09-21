#pragma once

#include "CutterLocationData.h"

//OpenGL rendering of the tool trajectory (CCutterLocationData itself does not depend on OpenGL)
class CCutterLocationDataRendererGL
{
public:
	static void Render(const CCutterLocationData &oData, bool bFastRendering);
};
