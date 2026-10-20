#include "../stdafx.h"
#include "WorkpieceHolder.h"
#include "ToolHolder.h"
#include "Translation.h"
#include "Rotation.h"

namespace kin5ax {

const char *szCUSTOM_HOLDER = "CUSTOM_HOLDER";

namespace {

/**
 * One entry of the catalogue: the designation, the frame the workpiece sits in and one box for the
 * body. Metres. A vice is a box with its jaws opening along one axis, which is the only thing that
 * tells an X vice from a Y vice as far as a collision check is concerned; a chuck is a box around
 * the cylinder, which is wider than the cylinder and therefore refuses a pose or two it could have
 * allowed; a tombstone holds the part on a face, which is why its frame has a rotation in it.
 */
struct CCatalogueEntry
{
	const char *szModel;
	double dX, dY, dZ;				//where the workpiece sits, read from the table
	double dAxisX, dAxisY, dAxisZ;	//the axis its frame is turned about
	double dAngleDegrees;
	bool bHasBody;
	double dMinX, dMinY, dMinZ;		//the body, in the frame of the table
	double dMaxX, dMaxY, dMaxZ;
};

const CCatalogueEntry vCATALOGUE[] = {
	//model             where the part sits      turned about      angle  body  the box of the body
	{szNO_HOLDER,       0.00, 0.00, 0.000,       0.0,0.0,1.0,      0.0,   false, 0,0,0, 0,0,0},
	{"CUSTOM_HOLDER",   0.00, 0.00, 0.000,       0.0,0.0,1.0,      0.0,   false, 0,0,0, 0,0,0},
	//a 125 mm machine vice, jaws opening along X: the part sits on the jaw tops
	{"X_VICE",          0.00, 0.00, 0.080,       0.0,0.0,1.0,      0.0,   true,
		-0.150,-0.075, 0.000,   0.150, 0.075, 0.080},
	//the same vice turned a quarter turn: the jaws open along Y
	{"Y_VICE",          0.00, 0.00, 0.080,       0.0,0.0,1.0,     90.0,   true,
		-0.075,-0.150, 0.000,   0.075, 0.150, 0.080},
	//a 200 mm three-jaw chuck standing on the table, the part on its face
	{"THREE_JAW_CHUCK", 0.00, 0.00, 0.100,       0.0,0.0,1.0,      0.0,   true,
		-0.100,-0.100, 0.000,   0.100, 0.100, 0.100},
	//a 250 mm four-jaw chuck, which is taller and wider
	{"FOUR_JAW_CHUCK",  0.00, 0.00, 0.110,       0.0,0.0,1.0,      0.0,   true,
		-0.125,-0.125, 0.000,   0.125, 0.125, 0.110},
	//a tombstone: the part is held on the +X face, so its Z points out along the X of the table
	{"TOMBSTONE",       0.150, 0.00, 0.250,      0.0,1.0,0.0,     90.0,   true,
		-0.150,-0.150, 0.000,   0.150, 0.150, 0.500},
	//a fixture plate: 25 mm of steel, the part on top of it
	{"FIXTURE_PLATE",   0.00, 0.00, 0.025,       0.0,0.0,1.0,      0.0,   true,
		-0.200,-0.200, 0.000,   0.200, 0.200, 0.025},
};

const size_t nCATALOGUE = sizeof(vCATALOGUE)/sizeof(vCATALOGUE[0]);

CWorkpieceHolder FromEntry(const CCatalogueEntry &oEntry)
{
	CWorkpieceHolder oHolder(oEntry.szModel
		,CR3Vector(oEntry.dX,oEntry.dY,oEntry.dZ)
		,CR3Vector(oEntry.dAxisX,oEntry.dAxisY,oEntry.dAxisZ)
		,oEntry.dAngleDegrees);
	if(oEntry.bHasBody){
		std::vector<CR3Box> vBoxes;
		vBoxes.push_back(CR3Box(CR3Point(oEntry.dMinX,oEntry.dMinY,oEntry.dMinZ)
			,CR3Point(oEntry.dMaxX,oEntry.dMaxY,oEntry.dMaxZ)));
		oHolder.SetBoxes(vBoxes);
	}
	return oHolder;
}

} // namespace

CWorkpieceHolder::CWorkpieceHolder()
	:m_r3vTranslation(0.0,0.0,0.0)
	,m_r3vAxis(0.0,0.0,1.0)
	,m_dAngleDegrees(0.0)
{
}

CWorkpieceHolder::CWorkpieceHolder(const std::string &sModel, const CR3Vector &r3vTranslation
	,const CR3Vector &r3vAxis, double dAngleDegrees)
	:m_sModel(sModel)
	,m_r3vTranslation(r3vTranslation)
	,m_r3vAxis(r3vAxis)
	,m_dAngleDegrees(dAngleDegrees)
{
}

void CWorkpieceHolder::SetFrame(const CR3Vector &r3vTranslation, const CR3Vector &r3vAxis
	,double dAngleDegrees)
{
	m_r3vTranslation = r3vTranslation;
	m_r3vAxis = r3vAxis;
	m_dAngleDegrees = dAngleDegrees;
}

CTransformation CWorkpieceHolder::GetTransformation() const
{
	//the rotation first and the translation after it, as a segment composes its constant frame
	return CTranslation(m_r3vTranslation)*CRotation(m_r3vAxis,m_dAngleDegrees,0.0);
}

CTransformation CWorkpieceHolder::GetInvertedTransformation() const
{
	return CRotation(m_r3vAxis,-m_dAngleDegrees,0.0)*CTranslation(-m_r3vTranslation);
}

bool CWorkpieceHolder::IsFrameIdentity() const
{
	return m_r3vTranslation.IsZero(1E-12) && is_zero(m_dAngleDegrees);
}

bool CWorkpieceHolder::IsEmpty() const
{
	return m_sModel.empty() && m_vBoxes.empty() && IsFrameIdentity();
}

bool CWorkpieceHolder::operator==(const CWorkpieceHolder &o) const
{
	if(m_sModel!=o.m_sModel || m_dAngleDegrees!=o.m_dAngleDegrees)
		return false;
	if(!(m_r3vTranslation==o.m_r3vTranslation) || !(m_r3vAxis==o.m_r3vAxis))
		return false;
	if(m_vBoxes.size()!=o.m_vBoxes.size())
		return false;
	for(size_t i=0;i<m_vBoxes.size();i++)
		if(!(m_vBoxes[i].GetMinPoint()==o.m_vBoxes[i].GetMinPoint())
			|| !(m_vBoxes[i].GetMaxPoint()==o.m_vBoxes[i].GetMaxPoint()))
			return false;
	return true;
}

size_t WorkpieceHolderCatalogueSize()
{
	return nCATALOGUE;
}

CWorkpieceHolder WorkpieceHolderFromCatalogue(size_t nIndex)
{
	if(nIndex>=nCATALOGUE)
		return CWorkpieceHolder();
	return FromEntry(vCATALOGUE[nIndex]);
}

bool WorkpieceHolderByModel(const std::string &sModel, CWorkpieceHolder &oHolder)
{
	for(size_t i=0;i<nCATALOGUE;i++)
		if(sModel==vCATALOGUE[i].szModel){
			oHolder = FromEntry(vCATALOGUE[i]);
			return true;
		}
	return false;
}

} // namespace kin5ax
