#pragma once

#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

//The tool holder: what is clamped in the tool mount of the machine, which is the TOOL terminal of the
//kinematic chain, and what the tool is then clamped in. It hangs from the mount downwards, towards
//the tool; where the tip of the tool ends up is the gauge length of the tool and has nothing to do
//with the holder's own length (CMillingTool::GetGaugeLength).
//
//The shape is up to three cylinders, which is enough to tell a slim collet chuck from a face-mill
//arbor for the one thing this library does with it: find out whether it meets a part of the machine
//or ploughs through material that was still there. Nobody is machining from this model, so the taper
//is a cylinder, the flange is a cylinder and the nose is a cylinder. The sections are given from the
//mount downwards, so the first is the one against the spindle.
//
//The model is free text, the designation on the holder itself: "BT40 ER32", "HSK63A SF12". The
//library never reads it; it is carried so that a host can match what the machine file says against
//the holders it has of its own. A chain file names the holder its machine usually carries, which is
//a default for a host to offer and nothing more.
namespace kin5ax {

//One cylinder of a holder, measured along the axis from the mount downwards
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

//the most cylinders a holder of this library is made of; more than three says nothing more about
//whether the holder meets anything
const size_t nTOOL_HOLDER_SECTIONS_MAX = 3;

class CToolHolder
{
public:
	//A holder of one cylinder, which is what this class was until it grew sections. A length without
	//a radius keeps the tool away from the mount without having a body to collide with.
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

	//the designation on the holder: free text, never read by this library
	const std::string & GetModel() const { return m_sModel; }
	void SetModel(const std::string &sModel) { m_sModel = sModel; }

	//the cylinders, from the mount downwards
	const std::vector<CToolHolderSection> & GetSections() const { return m_vSections; }
	//Keeps at most nTOOL_HOLDER_SECTIONS_MAX of them and drops the empty ones: a section of no length
	//is not a shape, and carrying it would only make two holders that are the same compare unequal.
	void SetSections(const std::vector<CToolHolderSection> &vSections)
	{
		m_vSections.clear();
		for(size_t i=0;i<vSections.size() && m_vSections.size()<nTOOL_HOLDER_SECTIONS_MAX;i++)
			if(vSections[i].GetLength()>0.0)
				m_vSections.push_back(vSections[i]);
	}

	//the whole length from the mount down to where the holder ends
	double GetLength() const
	{
		double d = 0.0;
		for(size_t i=0;i<m_vSections.size();i++)
			d += m_vSections[i].GetLength();
		return d;
	}
	//the widest of the cylinders: what a quick bounding test needs
	double GetRadius() const
	{
		double d = 0.0;
		for(size_t i=0;i<m_vSections.size();i++)
			d = std::max(d,m_vSections[i].GetRadius());
		return d;
	}
	//how far below the mount a section begins, measured downwards
	double GetSectionTop(size_t nIndex) const
	{
		double d = 0.0;
		for(size_t i=0;i<nIndex && i<m_vSections.size();i++)
			d += m_vSections[i].GetLength();
		return d;
	}
	//true when the holder has a solid to draw and to test for collisions
	bool HasBody() const
	{
		for(size_t i=0;i<m_vSections.size();i++)
			if(m_vSections[i].GetRadius()>0.0)
				return true;
		return false;
	}
	//a holder that is nothing at all: no length, no body, no name
	bool IsEmpty() const { return m_vSections.empty() && m_sModel.empty(); }

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

//---- the catalogue: the holders a five-axis machine usually carries.
//
//Ten of them, to be picked from a list rather than typed in, because a holder that is roughly right
//catches the collisions that matter and nobody has the drawing of theirs to hand. The dimensions are
//the usual ones for each taper and are near enough for a collision check; they are not a substitute
//for measuring the holder on the machine, and a host that knows better should say so.
//
//Lengths are metres, which is the unit of every model shipped with this library. A host whose chain
//is in another unit scales them.
size_t ToolHolderCatalogueSize();
//the holder at that place in the catalogue, empty when the index is past the end
CToolHolder ToolHolderFromCatalogue(size_t nIndex);
//the one whose model is that text, or an empty holder when the catalogue has no such name
CToolHolder ToolHolderByModel(const std::string &sModel);

} // namespace kin5ax
