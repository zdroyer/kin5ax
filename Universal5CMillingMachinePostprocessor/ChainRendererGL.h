#pragma once

#include "chain.h"
#include "material.h"
#include "CutterLocationData.h"

//OpenGL rendering of CChain with the material, the tool and the tool trajectory
//(CChain itself does not depend on OpenGL). Call CChain::Update before Render.
class CChainRendererGL
{
public:
	static void Render(const CChain &oChain
		, bool bFastRendering
		, CMaterial &oMaterial
		, const CMillingTool *pTool
		, const CCutterLocationData &oData);
};
