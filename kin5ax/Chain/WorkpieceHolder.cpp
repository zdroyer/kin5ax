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
	//the two vices are built from their opening and their jaw length, below: a vice is two jaws
	//and the gap between them, not one block
	{"X_VICE",          0.00, 0.00, 0.000,       0.0,0.0,1.0,      0.0,   false, 0,0,0, 0,0,0},
	{"Y_VICE",          0.00, 0.00, 0.000,       0.0,0.0,1.0,      0.0,   false, 0,0,0, 0,0,0},
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

//what a 125 mm machine vice opens to and how long its jaws are: the defaults of the catalogue
const double dDEFAULT_VICE_OPENING = 0.100;
const double dDEFAULT_VICE_JAW_LENGTH = 0.125;

CWorkpieceHolder FromEntry(const CCatalogueEntry &oEntry)
{
	const std::string sModel(oEntry.szModel);
	if(sModel=="X_VICE")
		return MakeVice(sModel,R3_AXIS_X,dDEFAULT_VICE_OPENING,dDEFAULT_VICE_JAW_LENGTH);
	if(sModel=="Y_VICE")
		return MakeVice(sModel,R3_AXIS_Y,dDEFAULT_VICE_OPENING,dDEFAULT_VICE_JAW_LENGTH);
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

CWorkpieceHolder MakeVice(const std::string &sModel, R3_AXIS nOpeningAxis, double dOpening
	,double dJawLength, double dJawThickness, double dJawHeight)
{
	const double dHalfOpening = 0.5*fabs(dOpening);
	const double dHalfLength = 0.5*fabs(dJawLength);
	const double dThickness = fabs(dJawThickness);
	const double dHeight = fabs(dJawHeight);
	//across the opening and along it: the jaws are as long as the one, as thick as the other
	R3_AXIS nAlong = R3_AXIS_X, nUp = R3_AXIS_Z;
	R3CrossAxes(R3_AXIS_Z,nAlong,nUp);			//the two axes of the table, whichever opens
	const R3_AXIS nLength = (nOpeningAxis==R3_AXIS_X) ? R3_AXIS_Y : R3_AXIS_X;
	std::vector<CR3Box> vBoxes;
	for(int nSide=0;nSide<2;nSide++){
		const double dNear = nSide ? dHalfOpening : -dHalfOpening-dThickness;
		const double dFar = nSide ? dHalfOpening+dThickness : -dHalfOpening;
		double dMin[3] = {0.0,0.0,0.0}, dMax[3] = {0.0,0.0,0.0};
		dMin[nOpeningAxis] = dNear;
		dMax[nOpeningAxis] = dFar;
		dMin[nLength] = -dHalfLength;
		dMax[nLength] = dHalfLength;
		dMin[R3_AXIS_Z] = 0.0;
		dMax[R3_AXIS_Z] = dHeight;
		vBoxes.push_back(CR3Box(CR3Point(dMin[0],dMin[1],dMin[2])
			,CR3Point(dMax[0],dMax[1],dMax[2])));
	}
	//the part sits on the jaw tops, in the middle of the opening
	CWorkpieceHolder oHolder(sModel,CR3Vector(0.0,0.0,dHeight),CR3Vector(0.0,0.0,1.0),0.0);
	oHolder.SetBoxes(vBoxes);
	return oHolder;
}

bool ViceParameters(const CWorkpieceHolder &oHolder, R3_AXIS &nOpeningAxis, double &dOpening
	,double &dJawLength)
{
	if(oHolder.GetBoxes().size()!=2)
		return false;
	const CR3Point p1Min = oHolder.GetBoxes()[0].GetMinPoint();
	const CR3Point p1Max = oHolder.GetBoxes()[0].GetMaxPoint();
	const CR3Point p2Min = oHolder.GetBoxes()[1].GetMinPoint();
	const CR3Point p2Max = oHolder.GetBoxes()[1].GetMaxPoint();
	//the axis the two boxes are apart along is the one they open along
	for(int i=0;i<2;i++){
		const R3_AXIS nAxis = (R3_AXIS)i;
		const double dGap = R3AxisComponent(p2Min,nAxis)-R3AxisComponent(p1Max,nAxis);
		if(dGap<=dEPSILON)
			continue;
		const R3_AXIS nLength = (nAxis==R3_AXIS_X) ? R3_AXIS_Y : R3_AXIS_X;
		nOpeningAxis = nAxis;
		dOpening = dGap;
		dJawLength = R3AxisComponent(p1Max,nLength)-R3AxisComponent(p1Min,nLength);
		return true;
	}
	(void)p2Max;
	return false;
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
