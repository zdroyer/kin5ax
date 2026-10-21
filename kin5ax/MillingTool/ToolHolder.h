#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

/**
 * @file ToolHolder.h
 * @brief What is clamped in the nose of the tool spindle, and what the tool is clamped in.
 *
 * A holder hangs from the terminal of the tool branch, which is the nose of the tool spindle, and
 * the tool sticks out of it. Its shape is up to @ref kin5ax::nTOOL_HOLDER_SECTIONS_MAX cylinders
 * given from the nose downwards, which is enough to tell a slim collet chuck from a face-mill arbor
 * for the one thing this library does with it: find out whether it meets a part of the machine or
 * ploughs through material that was still there. Nobody machines from this model, so the taper is a
 * cylinder, the flange is a cylinder and the nose is a cylinder.
 *
 * The model is the designation on the holder itself: "BT40 ER32", "HSK63A SF12". The library never
 * reads it; it is carried so that a host can match what the machine file says against the holders it
 * has of its own. @ref kin5ax::szNO_HOLDER is the holder of a tool clamped straight into the spindle:
 * a holder in the data with nothing in it.
 *
 * Lengths are in the unit of the chain file; the catalogue is in metres.
 */
namespace kin5ax {

/// One cylinder of a holder, measured along the axis from the spindle nose downwards.
class CToolHolderSection
{
public:
	CToolHolderSection(double dLength = 0.0, double dRadius = 0.0)
		:m_dLength(fabs(dLength))
		,m_dRadius(fabs(dRadius))
	{}
	double GetLength() const { return m_dLength; }
	double GetRadius() const { return m_dRadius; }
	void SetLength(double dLength) { m_dLength = fabs(dLength); }
	void SetRadius(double dRadius) { m_dRadius = fabs(dRadius); }
	bool operator==(const CToolHolderSection &o) const
	{ return m_dLength==o.m_dLength && m_dRadius==o.m_dRadius; }
	bool operator!=(const CToolHolderSection &o) const { return !(*this==o); }

protected:
	double m_dLength;
	double m_dRadius;
};

/// The most cylinders a holder is made of; a fourth says nothing more about what it meets.
const size_t nTOOL_HOLDER_SECTIONS_MAX = 3;

/// The designation of no holder at all: the tool is clamped straight into the spindle.
extern const char *szNO_HOLDER;

/**
 * @brief A tool holder: a designation and up to three cylinders hanging from the spindle nose.
 */
class CToolHolder
{
public:
	/**
	 * A holder of one cylinder, which is what this class was before it grew sections. A length
	 * without a radius keeps the tool away from the nose without a body to collide with.
	 */
	CToolHolder(double dLength = 0.0, double dRadius = 0.0)
	{
		//the section makes them positive; what matters here is whether anything was given at all
		if(dLength!=0.0 || dRadius!=0.0)
			m_vSections.push_back(CToolHolderSection(dLength,dRadius));
	}
	CToolHolder(const std::string &sModel, const std::vector<CToolHolderSection> &vSections)
		:m_sModel(sModel)
	{
		SetSections(vSections);
	}

	/// The designation on the holder: free text, never read by this library.
	const std::string & GetModel() const { return m_sModel; }
	void SetModel(const std::string &sModel) { m_sModel = sModel; }

	/// The cylinders, from the spindle nose downwards.
	const std::vector<CToolHolderSection> & GetSections() const { return m_vSections; }
	/**
	 * Keeps at most nTOOL_HOLDER_SECTIONS_MAX of them and drops the empty ones: a section of no
	 * length is not a shape, and carrying it would make two holders that are the same compare
	 * unequal.
	 */
	void SetSections(const std::vector<CToolHolderSection> &vSections)
	{
		m_vSections.clear();
		for(size_t i=0;i<vSections.size() && m_vSections.size()<nTOOL_HOLDER_SECTIONS_MAX;i++)
			if(vSections[i].GetLength()>0.0)
				m_vSections.push_back(vSections[i]);
	}

	/// The whole length from the spindle nose down to where the holder ends and the tool begins.
	double GetLength() const
	{
		double d = 0.0;
		for(size_t i=0;i<m_vSections.size();i++)
			d += m_vSections[i].GetLength();
		return d;
	}
	/// The widest of the cylinders: what a quick bounding test needs.
	double GetRadius() const
	{
		double d = 0.0;
		for(size_t i=0;i<m_vSections.size();i++)
			d = std::max(d,m_vSections[i].GetRadius());
		return d;
	}
	/// How far below the spindle nose a section begins, measured downwards.
	double GetSectionTop(size_t nIndex) const
	{
		double d = 0.0;
		for(size_t i=0;i<nIndex && i<m_vSections.size();i++)
			d += m_vSections[i].GetLength();
		return d;
	}
	/// True when the holder has a solid to draw and to test for collisions.
	bool HasBody() const
	{
		for(size_t i=0;i<m_vSections.size();i++)
			if(m_vSections[i].GetRadius()>0.0)
				return true;
		return false;
	}
	/// A holder that is nothing at all: no cylinder and no designation.
	bool IsEmpty() const { return m_vSections.empty() && m_sModel.empty(); }
	/// The tool is clamped straight into the spindle: no cylinder, whatever it is called.
	bool IsNoHolder() const { return m_vSections.empty(); }

	void SetLength(double dLength)
	{
		if(m_vSections.empty())
			m_vSections.push_back(CToolHolderSection(dLength,0.0));
		else
			m_vSections[0].SetLength(dLength);
	}
	void SetRadius(double dRadius)
	{
		if(m_vSections.empty())
			m_vSections.push_back(CToolHolderSection(0.0,dRadius));
		else
			m_vSections[0].SetRadius(dRadius);
	}

	bool operator==(const CToolHolder &oOther) const
	{ return m_sModel==oOther.m_sModel && m_vSections==oOther.m_vSections; }
	bool operator!=(const CToolHolder &oOther) const
	{ return !(*this==oOther); }

protected:
	std::string m_sModel;
	std::vector<CToolHolderSection> m_vSections;
};

/**
 * @name The catalogue
 *
 * The holders a five-axis machine usually carries, to be picked from a list rather than measured.
 * The first of them is @ref szNO_HOLDER, the tool clamped straight into the spindle. Then come the
 * bare tapers of the three standards a 7/24 spindle takes - BT (JIS B 6339), CAT (ASME B5.50) and
 * SK (DIN 69871), in sizes 30, 40 and 50 - which are the flange against the spindle nose and the
 * body below it; then the chucks, each a taper with something named on the end of it. The
 * dimensions are the usual ones for each taper and are near enough for a collision check; they are
 * no substitute for measuring the holder on the machine.
 *
 * Lengths are METRES, which is the unit of every model shipped with this library. A host whose chain
 * is in another unit scales them.
 * @{
 */
size_t ToolHolderCatalogueSize();
/// The holder at that place in the catalogue; an empty one when the index is past the end.
CToolHolder ToolHolderFromCatalogue(size_t nIndex);
/// Looks the designation up, exactly as the catalogue spells it; false when it has no such name.
bool ToolHolderByModel(const std::string &sModel, CToolHolder &oHolder);
/// @}

} // namespace kin5ax
