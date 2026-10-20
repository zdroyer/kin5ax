#pragma once

#include "R3Box.h"
#include "R3CylinderSegment.h"
#include "R3SphereSegment.h"
#include "Transformation.h"

#include <string>

/**
 * @file Solid.h
 * @brief One solid of a chain segment: a box, a piece of a cylinder or a piece of a sphere.
 *
 * A segment of a kinematic chain carries the solids that must not meet the solids of another
 * segment, of the tool or of the material. A box is the one a machine file has always had; a
 * cylinder segment and a sphere segment are there because a swivel head, a fork cheek, a rotary
 * table and a round arm end are not boxes, and boxing them loses the poses the machine really has.
 *
 * Every solid is given in the frame of its segment - the frame after the constant transformation of
 * the segment and before its joint motion - and carries its own position in that frame. There is no
 * frame of its own to place, which is why the cylinder names an axis of the segment frame and a
 * point on it rather than a rotation.
 *
 * @section meeting What "meets" means here
 *
 * @ref CSolid::Meets is **never wrong about a collision that is there**, and may say two solids
 * meet when they only nearly do:
 *
 *  - box against box is exact (CR3Box::HasIntersectionWithBox);
 *  - the limits of every solid - the two ends of a cylinder, the two cuts of a sphere, the sector
 *    of a cylinder - are exact as far as the box of the solid goes, since the test starts by
 *    comparing the two boxes;
 *  - beyond that, a cylinder is tested as the whole cylinder and a sphere as the whole sphere, so a
 *    wedge or a dome is treated as the shape it was cut from. A machine is refused a pose it could
 *    have had; it is never allowed one that breaks something. KnownBugs #72.
 */
namespace kin5ax {

enum SOLID_TYPE
{
	SOLID_BOX = 0,
	SOLID_CYLINDER,
	SOLID_SPHERE,
	SOLID_TYPE_COUNT
};

/// The name of a solid type as a chain file spells it: "box", "cylinder", "sphere".
const char *SolidTypeName(SOLID_TYPE nType);
bool SolidTypeFromName(const std::string &sName, SOLID_TYPE &nType);

class CSolid
{
public:
	/// An empty box, which is what a solid nobody has filled in yet is.
	CSolid();
	//not explicit: a box is a solid, and every caller that had one still hands one over
	CSolid(const CR3Box &oBox);
	CSolid(const CR3CylinderSegment &oCylinder);
	CSolid(const CR3SphereSegment &oSphere);

	SOLID_TYPE GetType() const { return m_nType; }
	/// The name of the type, which is also the word a collision report uses.
	const char * GetTypeName() const { return SolidTypeName(m_nType); }

	/// The box of a box solid; an empty box for the other two types.
	const CR3Box & GetBox() const { return m_oBox; }
	const CR3CylinderSegment & GetCylinder() const { return m_oCylinder; }
	const CR3SphereSegment & GetSphere() const { return m_oSphere; }
	void SetBox(const CR3Box &oBox);
	void SetCylinder(const CR3CylinderSegment &oCylinder);
	void SetSphere(const CR3SphereSegment &oSphere);

	/// The smallest box of the segment frame that holds this solid, whatever its type.
	CR3Box GetBoundingBox() const;
	/// Nothing to meet: an empty box, a cylinder with no radius or length, a sphere with no radius.
	bool IsEmpty() const;
	/// Why these numbers are not a solid, or an empty string when they are.
	std::string Problem() const;
	/// Is that point, in the frame of this solid, inside it or on it?
	bool Contains(const CR3Point &p) const;

	/**
	 * Does this solid meet the other one? See @ref meeting for how exact the answer is.
	 * @param oOther the other solid, in its own frame
	 * @param tr from the frame of the other solid to the frame of this one
	 * @param trInv the inverse of tr
	 */
	bool Meets(const CSolid &oOther, const CTransformation &tr, const CTransformation &trInv) const;

	bool operator==(const CSolid &o) const;
	bool operator!=(const CSolid &o) const { return !(*this==o); }

protected:
	SOLID_TYPE m_nType;
	CR3Box m_oBox;
	CR3CylinderSegment m_oCylinder;
	CR3SphereSegment m_oSphere;
};

} // namespace kin5ax
