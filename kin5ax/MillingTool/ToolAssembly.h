#pragma once

#include "ToolHolder.h"
#include "ToolDescription.h"

/**
 * @file ToolAssembly.h
 * @brief A tool holder with the tool installed in it: what is clamped in the spindle, as one thing.
 *
 * The two ends of the measurement a machine needs are the nose of the tool spindle and the point a
 * cutter location programs. What lies between them is a holder and a tool sticking out of it, and
 * that is an assembly:
 *
 *     nose ──┬── holder ──┬── tool ──┬── tip
 *            │            │          │
 *            │<- GetHolderLength()   │
 *            │<--------- GetLength() │    nose to the tip
 *            │<----- GetToolOffset()      nose to the programmed point
 *
 * The two lengths differ only when the programmed point is not the tip: a ball nose tool programmed
 * at the centre of its ball has its point one radius higher up.
 *
 * An EMPTY assembly - @ref kin5ax::szNO_HOLDER with @ref kin5ax::szNO_TOOL - is nothing in the
 * spindle at all: no solid to collide with and no length. A postprocessor takes it like any other
 * and works the same way; there is simply nothing to check collisions against.
 */
namespace kin5ax {

/// The designation a host shows for a tool of no length at all. No chain file writes it: a file
/// without a tool means the same thing.
extern const char *szNO_TOOL;

/// A tool of no length: what is in the spindle when nothing is.
CToolDescription NoTool();

/**
 * @brief A holder and the tool in it, with the lengths a machine needs.
 */
class CToolAssembly
{
public:
	CToolAssembly();
	CToolAssembly(const CToolHolder &oHolder, const CToolDescription &oTool);
	/// Nothing in the spindle: NO_HOLDER and NO_TOOL.
	static CToolAssembly Empty();

	const CToolHolder & GetHolder() const { return m_oHolder; }
	const CToolDescription & GetTool() const { return m_oTool; }
	void SetHolder(const CToolHolder &oHolder) { m_oHolder = oHolder; }
	void SetTool(const CToolDescription &oTool) { m_oTool = oTool; }

	/// From the nose of the spindle to the end of the holder, where the tool begins.
	double GetHolderLength() const { return m_oHolder.GetLength(); }
	/// From the nose of the spindle to the tip of the tool.
	double GetLength() const;
	/**
	 * From the nose of the spindle to the point a cutter location programs: the length of the
	 * assembly less the height of that point above the tip. This is the number a postprocessor
	 * works from, and @ref CChain::SetToolOffset takes.
	 */
	double GetToolOffset() const;

	/// Nothing in the spindle: no holder, no tool, no length and nothing to collide with.
	bool IsEmpty() const;
	/// True when there is a solid here that a collision check can use.
	bool HasBody() const;
	/// Why this is not an assembly, or an empty string when it is; an empty assembly is one.
	std::string Problem() const;

	/**
	 * The tool and its holder as the solids of a CMillingTool, for the caller to own and delete;
	 * null for an empty assembly, which has no solids, and for one that does not make a tool.
	 */
	CMillingTool * MakeTool() const;

	bool operator==(const CToolAssembly &o) const
	{ return m_oHolder==o.m_oHolder && m_oTool==o.m_oTool; }
	bool operator!=(const CToolAssembly &o) const { return !(*this==o); }

protected:
	CToolHolder m_oHolder;
	CToolDescription m_oTool;
};

} // namespace kin5ax
