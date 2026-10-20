#pragma once

#include "millingtool.h"

/**
 * @brief GDI (2D preview) rendering of a tool assembly: the holder, the tool in it and the two
 * points that are measured from the nose of the spindle.
 *
 * The whole assembly is drawn to one scale, from the nose of the spindle at the top down to the tip
 * of the tool at the bottom, so that the numbers of a dialog can be read off the picture: the
 * sections of the holder, the shank that must cut nothing, the cutting part in orange, and the
 * reference point the cutter locations are programmed at. CMillingTool itself knows nothing of GDI.
 */
class CMillingToolRendererGDI
{
public:
	static void Render(CDC &dc, const CMillingTool &oTool);
protected:
	//where the drawing sits in the window and how many pixels a length unit is
	struct CPlacement
	{
		int m_nX0;				//the axis of the tool
		int m_nZ0;				//the tip of the tool, which is z = 0
		double m_dScale;		//pixels per length unit of the tool
		int X(double dR) const { return m_nX0+round_to_int(dR*m_dScale); }
		int Z(double dZ) const { return m_nZ0-round_to_int(dZ*m_dScale); }
	};
	//the whole assembly into the client area of the window, or false when there is nothing to draw
	static bool Fit(CDC &dc, const CMillingTool &oTool, const CRect &rt, CPlacement &oOut);
	//the holder hanging from the nose, section by section
	static void RenderHolder(CDC &dc, const CMillingTool &oTool, const CPlacement &oPlace);
	//the shank between the cutting part and the holder: it must meet nothing at all
	static void RenderShank(CDC &dc, const CMillingTool &oTool, const CPlacement &oPlace);
	//the cutting part, whose outline is the shape of the tool: flat, ball or rounded edge
	static void RenderCuttingPart(CDC &dc, const CMillingTool &oTool, const CPlacement &oPlace);
	//the nose of the spindle, the tip, the reference point and the axis
	static void RenderMarks(CDC &dc, const CMillingTool &oTool, const CPlacement &oPlace, const CRect &rt);
};
