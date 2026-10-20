#pragma once

#include "R3Vector.h"
#include "R3Box.h"
#include "Transformation.h"

#include <string>
#include <vector>

/**
 * @file WorkpieceHolder.h
 * @brief What is bolted to the table of the machine, and where it holds the workpiece.
 *
 * The terminal of the workpiece branch is the table. What stands on it - a vice, a chuck, a
 * tombstone, a plate - is a frame and a few solids: the frame says where the workpiece sits, the
 * solids are what the tool must not run into. The frame is given the way a segment gives its
 * constant transformation, as a translation and a rotation about an axis, which is the R and the T
 * of it; the solids are boxes in the frame of the table.
 *
 * A chain file names the holder its machine usually carries as a default to offer. @ref
 * kin5ax::szNO_HOLDER is the workpiece bolted to the table itself: the identity and no solid.
 * @ref kin5ax::szCUSTOM_HOLDER is the one a host fills in, which is why the catalogue has it at all.
 *
 * Lengths are in the unit of the chain file; the catalogue is in metres.
 */
namespace kin5ax {

/// The designation of a holder whose frame and solids the host gives itself.
extern const char *szCUSTOM_HOLDER;

/**
 * @brief A workpiece holder: a designation, the frame it holds the part in and the solids it is.
 */
class CWorkpieceHolder
{
public:
	CWorkpieceHolder();
	CWorkpieceHolder(const std::string &sModel, const CR3Vector &r3vTranslation
		,const CR3Vector &r3vAxis, double dAngleDegrees);

	/// The designation: free text, never read by this library, matched by a host against its own.
	const std::string & GetModel() const { return m_sModel; }
	void SetModel(const std::string &sModel) { m_sModel = sModel; }

	/// Where the workpiece sits, read from the table: the translation and the rotation of its frame.
	const CR3Vector & GetTranslation() const { return m_r3vTranslation; }
	const CR3Vector & GetRotationAxis() const { return m_r3vAxis; }
	double GetRotationAngleDegrees() const { return m_dAngleDegrees; }
	void SetFrame(const CR3Vector &r3vTranslation, const CR3Vector &r3vAxis, double dAngleDegrees);

	/// The frame as one transformation, composed as a chain segment composes its constant one.
	CTransformation GetTransformation() const;
	CTransformation GetInvertedTransformation() const;
	/// True when that frame is the identity: the workpiece sits on the table itself.
	bool IsFrameIdentity() const;

	/// The solids of the holder, in the frame of the table.
	const std::vector<CR3Box> & GetBoxes() const { return m_vBoxes; }
	void SetBoxes(const std::vector<CR3Box> &vBoxes) { m_vBoxes = vBoxes; }
	bool HasBody() const { return !m_vBoxes.empty(); }

	/// A holder that is nothing at all: no designation, no solid and the identity frame.
	bool IsEmpty() const;

	bool operator==(const CWorkpieceHolder &o) const;
	bool operator!=(const CWorkpieceHolder &o) const { return !(*this==o); }

protected:
	std::string m_sModel;
	CR3Vector m_r3vTranslation;
	CR3Vector m_r3vAxis;
	double m_dAngleDegrees;
	std::vector<CR3Box> m_vBoxes;
};

/**
 * @name The catalogue
 *
 * The holders a table usually carries. The first two are the ones that are not holders at all:
 * @ref szNO_HOLDER, the part bolted to the table, and @ref szCUSTOM_HOLDER, the one a host
 * describes itself. The rest are the usual sizes, near enough for a collision check and no
 * substitute for measuring what is on the machine. Lengths are METRES.
 * @{
 */
size_t WorkpieceHolderCatalogueSize();
/// The holder at that place in the catalogue; an empty one when the index is past the end.
CWorkpieceHolder WorkpieceHolderFromCatalogue(size_t nIndex);
/// Looks the designation up, exactly as the catalogue spells it; false when it has no such name.
bool WorkpieceHolderByModel(const std::string &sModel, CWorkpieceHolder &oHolder);
/// @}

} // namespace kin5ax
