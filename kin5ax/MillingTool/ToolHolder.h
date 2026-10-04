#pragma once

//The tool holder: the cylinder between the tool mount of the machine (the TOOL terminal of the kinematic
//chain) and the tool, along the Z axis of the mount. The chain file gives the default of a machine; the
//application may replace it together with the tool. Lengths are the unit of the chain file.
//A length of zero means no holder, which is what a file without a holder gives: the behaviour of the
//library before the holder existed.
namespace kin5ax {

class CToolHolder
{
public:
	CToolHolder(double dLength = 0.0, double dRadius = 0.0)
		:m_dLength(fabs(dLength))
		,m_dRadius(fabs(dRadius))
	{}
	//the distance from the tool mount to the end of the holder, where the tool begins
	double GetLength() const { return m_dLength; }
	double GetRadius() const { return m_dRadius; }
	void SetLength(double dLength) { m_dLength = fabs(dLength); }
	void SetRadius(double dRadius) { m_dRadius = fabs(dRadius); }
	//true when the holder has a solid to draw and to test for collisions; a holder with a length but no radius
	//keeps the tool away from the mount without a body of its own
	bool HasBody() const { return m_dLength>0.0 && m_dRadius>0.0; }
	bool operator==(const CToolHolder &oOther) const
	{ return m_dLength==oOther.m_dLength && m_dRadius==oOther.m_dRadius; }
	bool operator!=(const CToolHolder &oOther) const
	{ return !(*this==oOther); }

protected:
	double m_dLength;
	double m_dRadius;
};

} // namespace kin5ax
