#pragma once

#include "Material.h"

//OpenGL rendering of CMaterial (CMaterial itself does not depend on OpenGL)
class CMaterialRendererGL
{
public:
	//The surface voxels as cubes, or as points when a voxel is smaller than a pixel. r3vObserver: the
	//direction the material is seen from, in the material frame; it decides which faces of a voxel are
	//drawn. dPixelSize: the size of a pixel in the material frame. bFastRendering: the contour only.
	static void Render(CMaterial &oMaterial, const CR3Vector &r3vObserver, double dPixelSize, bool bFastRendering);
	//the twelve edges of the material block
	static void RenderContour(const CMaterial &oMaterial);
};
