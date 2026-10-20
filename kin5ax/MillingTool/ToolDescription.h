#pragma once

#include "MillingTool.h"
#include "ToolHolder.h"

#include <string>

/**
 * @file ToolDescription.h
 * @brief The tool as a chain file describes it: dimensions and nothing that moves.
 *
 * A chain file names the holder its machine usually carries and the tool that goes in it. Both are
 * defaults for whoever puts something in the spindle, and neither takes part in the kinematics: what
 * places a tool on a machine is the tool offset given at run time (CChain::SetToolOffset).
 *
 * This is a plain value, which is why it is not a CMillingTool: a chain is copied, and a tool is a
 * polymorphic object with a position and an orientation of its own. MakeTool builds one when a
 * caller wants the solids.
 */
namespace kin5ax {

/// Where the point a cutter location programs lies on the tool.
enum TOOL_REFERENCE_POINT
{
	TOOL_REFERENCE_TIP = 0,		///< the tip of the tool, which is the default for every shape
	TOOL_REFERENCE_BALL_CENTRE,	///< the centre of the ball, for a ball nose tool only
	TOOL_REFERENCE_CUSTOM		///< a height above the tip, measured towards the spindle
};

/// The name of a reference point as a chain file spells it, and back; an unknown name gives false.
const char *ToolReferencePointName(TOOL_REFERENCE_POINT nReference);
bool ToolReferencePointFromName(const std::string &sName, TOOL_REFERENCE_POINT &nReference);
/// The name of a tool shape as a chain file spells it ("ball_end", "cylinder", "toroidal"), and back.
const char *MillingToolTypeName(MILLING_TOOL_TYPE nType);
bool MillingToolTypeFromName(const std::string &sName, MILLING_TOOL_TYPE &nType);

/**
 * @brief The tool of a chain file: a shape, its sizes, how far it sticks out and where it is programmed.
 */
class CToolDescription
{
public:
	CToolDescription();

	MILLING_TOOL_TYPE m_nType;		///< ball nose, flat end or toroidal
	double m_dRadius;				///< the radius of the tool: half the cutting diameter
	double m_dCornerRadius;			///< the rounding of the bottom edge; the ball of a ball nose tool
	double m_dCuttingHeight;		///< how far the cutting part reaches up from the tip
	double m_dLength;				///< the whole length that sticks out of the holder
	TOOL_REFERENCE_POINT m_nReference;
	double m_dReferenceHeight;		///< for TOOL_REFERENCE_CUSTOM: above the tip, towards the spindle

	/// True when a file described a tool at all; a chain may carry none.
	bool IsGiven() const { return m_bGiven; }
	void SetGiven(bool bGiven) { m_bGiven = bGiven; }

	/// How far the programmed point lies above the tip, with the reference point resolved.
	double GetReferencePointZ() const;
	/// The part of the stick-out above the cutting part, which must cut nothing.
	double GetShankLength() const;
	/// Why these dimensions are not a tool, or an empty string when they are.
	std::string DimensionProblem() const;
	/**
	 * A tool of these dimensions in that holder, for the caller to own and delete; null when no
	 * tool was described or the dimensions are not a tool.
	 */
	CMillingTool * MakeTool(const CToolHolder &oHolder) const;

	bool operator==(const CToolDescription &o) const;
	bool operator!=(const CToolDescription &o) const { return !(*this==o); }

protected:
	bool m_bGiven;
};

} // namespace kin5ax
