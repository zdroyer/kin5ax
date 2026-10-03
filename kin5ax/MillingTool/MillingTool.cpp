#include "../stdafx.h"
#include "MillingTool.h"


namespace kin5ax {

extern CR3Vector r3vOZ;

CMillingTool::CMillingTool
	(double dRz
	,double dR
	,double dCuttingHeight
	,double dReferencePointZ)
:m_dRz(fabs(dRz))
,m_dR(fabs(dR))
,m_dCuttingHeight(fabs(dCuttingHeight))
,m_r3pPosition(0.0)
,m_r3vOrientation(r3vOZ)
,m_dAxisOZAngle(0.0)
,m_dSineAxisOZAngle(0.0)
,m_m4x4ToOZ(CTransformation(1.0))
,m_dReferencePointZ(dReferencePointZ)
,m_r3vMoveVector(0.0)
{
	m_dRSquare = m_dR * m_dR;
	m_dRzSquare = m_dRz * m_dRz;
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

const CR3Vector &CMillingTool::GetMoveVector() const
{
	return m_r3vMoveVector; 
}

void CMillingTool::SetPositionAndOrientation(const CR3CaughtVector &cv)
{
	SetPositionAndOrientation(cv.m_p,cv.m_v);
}
void CMillingTool::SetPositionAndOrientation(const CR3Point &r3pNewPosition, const CR3Vector &r3vNewOrientation)
{

	assert(!r3vNewOrientation.IsZero());
	if(r3vNewOrientation.IsZero())
		return;
	static bool bFirst=true;
	if(bFirst)
		bFirst=false;
	else 
		m_r3vMoveVector = r3pNewPosition-m_r3pPosition;
	m_r3pPosition.SetPoint(r3pNewPosition);
	m_r3vOrientation = r3vNewOrientation.GetUnitVector();
	m_dSineAxisOZAngle = m_r3vOrientation.vectorProduct(r3vOZ).GetLength();
	m_dAxisOZAngle = atan2(m_dSineAxisOZAngle, m_r3vOrientation.scalarProduct(r3vOZ));
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
	CR3Box box(m_r3pPosition + ( m_r3vOrientation * dBegin )
		,m_r3pPosition + ( m_r3vOrientation * (dEnd)));

	double dDeltaZ = dRadius * m_dSineAxisOZAngle;
	box.ExpandXYZ(dRadius,dRadius,dDeltaZ);
	return box;
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
