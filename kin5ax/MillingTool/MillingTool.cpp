#include "../stdafx.h"
#include "MillingTool.h"
#include "ArgsMyException.h"


namespace kin5ax {

extern CR3Vector r3vOZ;

CMillingTool::CMillingTool
	(double dRz
	,double dR
	,double dCuttingHeight
	,double dReferencePointZ
	,double dNonCuttingLength
	,const CToolHolder &oHolder
	,double dGaugeLength)
:m_dRz(dRz)
,m_dR(dR)
,m_dReferencePointZ(dReferencePointZ)
,m_dCuttingHeight(dCuttingHeight)
,m_dNonCuttingLength(dNonCuttingLength)
,m_oHolder(oHolder)
,m_dGaugeLength(dGaugeLength>0.0 ? dGaugeLength : 0.0)
,m_r3pPosition(0.0)
,m_r3vOrientation(r3vOZ)
,m_m4x4ToOZ(CTransformation(1.0))
{
	//the dimensions are taken as they are given and checked by the shape (CheckDimensions in the
	//constructor of the tool), instead of being made positive behind the caller's back (KnownBugs #50)
	m_dRSquare = m_dR * m_dR;
	m_dRzSquare = m_dRz * m_dRz;
}

std::string CMillingTool::DimensionProblem(MILLING_TOOL_TYPE type, double dRz, double dR, double dCuttingHeight
	,double dReferencePointZ, double dNonCuttingLength)
{
	if(dRz<0.0 || dR<0.0 || dCuttingHeight<0.0 || dNonCuttingLength<0.0)
		return "a dimension is negative";
	if(is_zero(dRz))
		return "the tool has no radius (Rz is zero)";
	if(is_zero(dCuttingHeight))
		return "the cutting part has no length";
	if(type==MILLING_TOOL_BALL){
		if(is_zero(dR))
			return "a ball nose tool has no ball (R is zero)";
		if(dR<dRz-dEPSILON)
			return "the ball is narrower than the shank (R < Rz), so the shank would stand out of the nose";
		if(dR>dCuttingHeight+dEPSILON)
			return "the ball is taller than the cutting part (R > the cutting height)";
	}
	if(!is_between_or_equal(dReferencePointZ,0.0,dCuttingHeight+dNonCuttingLength))
		return "the programmed point is not on the tool: its height above the tip is outside 0 .. the length of the tool";
	return "";
}

void CMillingTool::CheckDimensions() const
{
	std::string sProblem = DimensionProblem(m_type,m_dRz,m_dR,m_dCuttingHeight,m_dReferencePointZ,m_dNonCuttingLength);
	if(sProblem.size())
		MY_THROW(CArgsMyException,std::string("These dimensions do not make a milling tool: ")+sProblem);
}

void CMillingTool::MakeMinMax(double &d1,double &d2) const
{
	if(d1<=d2)
		return;
	double d = d1;
	d1 = d2;
	d2 = d;
}

CMillingTool::~CMillingTool(void)
{
}

std::string CMillingTool::GetCollisionLog() const
{
	if(m_sCollisionLog.size())
		return std::string("Milling tool in collision with:")+m_sCollisionLog;
	return "";
}
void CMillingTool::ClearCollisionLog()
{
	m_sCollisionLog="";
}
void CMillingTool::AddToCollisionLog(const std::string &s)
{
	m_sCollisionLog+=("\n\t\t"+s);
}

void CMillingTool::SetPositionAndOrientation(const CR3AnchoredVector &cv)
{
	SetPositionAndOrientation(cv.m_p,cv.m_v);
}
void CMillingTool::SetPositionAndOrientation(const CR3Point &r3pNewPosition, const CR3Vector &r3vNewOrientation)
{

	assert(!r3vNewOrientation.IsZero());
	if(r3vNewOrientation.IsZero())
		return;
	m_r3pPosition.SetPoint(r3pNewPosition);
	m_r3vOrientation = r3vNewOrientation.GetUnitVector();
	//to the frame of IsInOZCuttingSpace (tool axis = OZ, tip = origin): the reference point goes to the origin,
	//the tool axis is turned to OZ, and only then the tool is lifted by the height of the reference point
	m_m4x4ToOZ = CTransformation( CTranslation(r3vOZ*m_dReferencePointZ) )
		* CTransformation( CRotation(m_r3vOrientation,r3vOZ) )
		* CTransformation( CTranslation(-m_r3pPosition.toR3Vector()) );
}



CR3Box CMillingTool::AxialCylinderBoundingBox(double dRadius,double dBegin, double dEnd) const
{
	if(dRadius==0.0){
		return CR3Box();
	}
	//exact, since the box of a cylinder is exact now; it used to grow by the radius in X and in Y whatever
	//the direction of the axis (KnownBugs #55)
	return CR3Box::BoxFromCylinder(m_r3pPosition + (m_r3vOrientation*dBegin)
		,m_r3pPosition + (m_r3vOrientation*dEnd),dRadius);
}


double CMillingTool::GetToolLength() const
{
	return m_dCuttingHeight + m_dNonCuttingLength;
}

double CMillingTool::GetGaugeLength() const
{
	//0 was given: the caller has not measured a tool, so it stands right up against its holder
	return m_dGaugeLength>0.0 ? m_dGaugeLength : GetToolLength()+m_oHolder.GetLength();
}

void CMillingTool::SetGaugeLength(double dLength)
{
	m_dGaugeLength = dLength>0.0 ? dLength : 0.0;
}

double CMillingTool::GetAssemblyLength() const
{
	return GetGaugeLength();
}

double CMillingTool::GetMountToToolPointDistance() const
{
	return GetAssemblyLength() - m_dReferencePointZ;
}

bool CMillingTool::HasNonCuttingPart() const
{
	return (m_dNonCuttingLength>0.0 && m_dRz>0.0) || m_oHolder.HasBody();
}

bool CMillingTool::IsInOZNonCuttingSpace(const CR3Point &p) const
{
	const double dToolLength = GetToolLength();
	const double dSq = p.GetX()*p.GetX()+p.GetY()*p.GetY();
	if(m_dNonCuttingLength>0.0 && m_dRz>0.0
		&& p.GetZ()>=m_dCuttingHeight-dEPSILON && p.GetZ()<=dToolLength+dEPSILON
		&& dSq<=m_dRzSquare+dEPSILON)
		return true;
	//the holder hangs from the mount, which is the gauge length above the tip
	const double dMount = GetGaugeLength();
	if(m_oHolder.HasBody()
		&& p.GetZ()>=dMount-m_oHolder.GetLength()-dEPSILON && p.GetZ()<=dMount+dEPSILON
		&& dSq<=m_oHolder.GetRadius()*m_oHolder.GetRadius()+dEPSILON)
		return true;
	return false;
}

bool CMillingTool::IsInNonCuttingPart(const CR3Point &r3pP) const
{
	return IsInOZNonCuttingSpace(m_m4x4ToOZ * r3pP);
}

CR3Box CMillingTool::GetNonCuttingPartBoundingBox() const
{
	if(!HasNonCuttingPart())
		return CR3Box();
	const bool bShank = m_dNonCuttingLength>0.0 && m_dRz>0.0;
	const double dRadius = std::max(bShank ? m_dRz : 0.0, m_oHolder.HasBody() ? m_oHolder.GetRadius() : 0.0);
	//from the bottom of whichever solid reaches lowest to the top of whichever reaches highest
	const double dHolderBottom = GetGaugeLength()-m_oHolder.GetLength();
	const double dBegin = bShank ? m_dCuttingHeight
		: (m_oHolder.HasBody() ? dHolderBottom : GetToolLength());
	const double dEnd = m_oHolder.HasBody() ? GetGaugeLength() : GetToolLength();
	return AxialCylinderBoundingBox(dRadius,dBegin-m_dReferencePointZ,dEnd-m_dReferencePointZ);
}

bool CMillingTool::CylinderMeetsBox(const CR3Box &box, double dBeginZ, double dLength, double dRadius
	,const CTransformation &trToBoxFrame, const CTransformation &trToToolFrame) const
{
	if(dLength<=0.0 || dRadius<=0.0)
		return false;
	//the cylinder frame has its base dBeginZ above the tip, that is dBeginZ-m_dReferencePointZ from the tool frame
	CTranslation moveFromCylinder(0.0,0.0,dBeginZ-m_dReferencePointZ);
	CTranslation moveToCylinder(0.0,0.0,m_dReferencePointZ-dBeginZ);
	return box.HasIntersectionWithOZCylinder(dLength,dRadius,trToBoxFrame*moveFromCylinder,moveToCylinder*trToToolFrame);
}

bool CMillingTool::CauseNonCuttingCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const
{
	//the shank above the cutting part
	if(CylinderMeetsBox(box,m_dCuttingHeight,m_dNonCuttingLength,m_dRz,trToBoxFrame,trToToolFrame))
		return true;
	//the holder, hanging from the mount
	if(m_oHolder.HasBody()
		&& CylinderMeetsBox(box,GetGaugeLength()-m_oHolder.GetLength(),m_oHolder.GetLength()
			,m_oHolder.GetRadius(),trToBoxFrame,trToToolFrame))
		return true;
	return false;
}

CUT_TYPE CMillingTool::IsCut(const CR3Point &r3pP) const
{
	CR3Point p = m_m4x4ToOZ * r3pP;
	CUT_TYPE cutType = CUT_TYPE_NOT_CUT;
	if(IsInOZCuttingSpace(p))
		cutType |= (unsigned int)CUT_TYPE_CUT;
	return cutType;
}


} // namespace kin5ax
