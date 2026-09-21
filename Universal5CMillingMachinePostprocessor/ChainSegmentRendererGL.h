#pragma once

#include "chainsegment.h"

//OpenGL rendering of CChainSegment (CChainSegment itself does not depend on OpenGL).
//Render leaves the OpenGL model-view matrix in the frame of the next segment.
class CChainSegmentRendererGL
{
public:
	static void Render(const CChainSegment &oSegment, bool bDrawBigFrame);
protected:
	static void RenderParts(const CChainSegment &oSegment);

	static void RenderRevolute(const CChainSegment &oSegment, bool bDrawBigFrame);
	static void RenderPrismatic(const CChainSegment &oSegment, bool bDrawBigFrame);
	static void RenderToolHolder(const CChainSegment &oSegment, bool bDrawBigFrame);
	static void RenderMaterialHolder(const CChainSegment &oSegment, bool bDrawBigFrame);

	static void DrawArm(const CR3Vector &r3vEndOfArm);
	static void OZBox(double dXDiv2, double dYDiv2, double dZ);
	static void ApplyConstantTransformation(const CChainSegment &oSegment);
};
