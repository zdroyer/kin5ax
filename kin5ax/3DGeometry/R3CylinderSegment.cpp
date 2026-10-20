#include "../stdafx.h"
#include "R3CylinderSegment.h"
#include "double_tools.h"
#include "string_tools.h"

#include <cmath>
#include <algorithm>

namespace kin5ax {

namespace {

//the angle into 0 .. 360
double Normalised(double dAngleDeg)
{
	double d = fmod(dAngleDeg,360.0);
	if(d<0.0)
		d += 360.0;
	return d;
}

} // namespace

bool IsAngleInSector(double dAngleDeg, double dMinAngleDeg, double dMaxAngleDeg)
{
	if(dMaxAngleDeg-dMinAngleDeg>=360.0-dEPSILON)
		return true;								//the whole turn
	//the angle brought into the turn the sector starts in, so that a sector across 0 works
	double d = Normalised(dAngleDeg-dMinAngleDeg);
	return is_less_or_equal(d,dMaxAngleDeg-dMinAngleDeg);
}

CR3CylinderSegment::CR3CylinderSegment()
	:m_nAxis(R3_AXIS_Z)
	,m_r3pAxisPoint(0.0)
	,m_dBase(0.0)
	,m_dTop(0.0)
	,m_dRadius(0.0)
	,m_dMinAngleDeg(0.0)
	,m_dMaxAngleDeg(360.0)
{
}

CR3CylinderSegment::CR3CylinderSegment(R3_AXIS nAxis, const CR3Point &r3pAxisPoint, double dBase
	,double dTop, double dRadius, double dMinAngleDeg, double dMaxAngleDeg)
	:m_nAxis(nAxis)
	,m_r3pAxisPoint(r3pAxisPoint)
	,m_dBase(0.0)
	,m_dTop(0.0)
	,m_dRadius(fabs(dRadius))
	,m_dMinAngleDeg(0.0)
	,m_dMaxAngleDeg(360.0)
{
	SetEnds(dBase,dTop);
	SetSector(dMinAngleDeg,dMaxAngleDeg);
}

void CR3CylinderSegment::SetEnds(double dBase, double dTop)
{
	m_dBase = std::min(dBase,dTop);
	m_dTop = std::max(dBase,dTop);
}

void CR3CylinderSegment::SetSector(double dMinAngleDeg, double dMaxAngleDeg)
{
	const double dSweep = dMaxAngleDeg-dMinAngleDeg;
	if(dSweep>=360.0-dEPSILON || is_zero(dSweep)){
		m_dMinAngleDeg = 0.0;						//the whole turn, however it was written
		m_dMaxAngleDeg = 360.0;
		return;
	}
	m_dMinAngleDeg = Normalised(dMinAngleDeg);
	m_dMaxAngleDeg = m_dMinAngleDeg + Normalised(dSweep);
}

bool CR3CylinderSegment::IsWholeTurn() const
{
	return m_dMaxAngleDeg-m_dMinAngleDeg>=360.0-dEPSILON;
}

bool CR3CylinderSegment::IsEmpty() const
{
	return is_zero(m_dRadius) || is_zero(GetLength());
}

std::string CR3CylinderSegment::Problem() const
{
	if(m_dRadius<0.0)
		return "the radius of a cylinder cannot be negative";
	if(is_zero(m_dRadius))
		return "the cylinder has no radius";
	if(is_zero(GetLength()))
		return "the two ends of the cylinder are at the same place, so it has no length";
	if(m_dMaxAngleDeg-m_dMinAngleDeg<dEPSILON)
		return "the sector of the cylinder is empty: the two angles are the same";
	return "";
}

bool CR3CylinderSegment::Contains(const CR3Point &p) const
{
	if(IsEmpty())
		return false;
	if(!is_between_or_equal(R3AxisComponent(p,m_nAxis),m_dBase,m_dTop))
		return false;
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_Y;
	R3CrossAxes(m_nAxis,n1,n2);
	const double dU = R3AxisComponent(p,n1)-R3AxisComponent(m_r3pAxisPoint,n1);
	const double dV = R3AxisComponent(p,n2)-R3AxisComponent(m_r3pAxisPoint,n2);
	const double dR = sqrt(dU*dU+dV*dV);
	if(!is_less_or_equal(dR,m_dRadius))
		return false;
	if(IsWholeTurn() || is_zero(dR))
		return true;								//a point on the axis is in every wedge
	return IsAngleInSector(radians_to_degrees(atan2(dV,dU)),m_dMinAngleDeg,m_dMaxAngleDeg);
}

CR3Box CR3CylinderSegment::GetBoundingBox() const
{
	if(IsEmpty())
		return CR3Box();
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_Y;
	R3CrossAxes(m_nAxis,n1,n2);
	double dUMin = 0.0, dUMax = 0.0, dVMin = 0.0, dVMax = 0.0;
	if(IsWholeTurn()){
		dUMin = dVMin = -m_dRadius;
		dUMax = dVMax = m_dRadius;
	} else {
		//the wedge reaches the axis, so the centre counts; then the two ends of the arc and every
		//quarter turn between them, which is where the arc is furthest out along an axis
		std::vector<double> vAngles;
		vAngles.push_back(m_dMinAngleDeg);
		vAngles.push_back(m_dMaxAngleDeg);
		for(double d=0.0;d<=720.0;d+=90.0)
			if(d>m_dMinAngleDeg && d<m_dMaxAngleDeg)
				vAngles.push_back(d);
		for(size_t i=0;i<vAngles.size();i++){
			const double dRad = degrees_to_radians(vAngles[i]);
			dUMin = std::min(dUMin,m_dRadius*cos(dRad));
			dUMax = std::max(dUMax,m_dRadius*cos(dRad));
			dVMin = std::min(dVMin,m_dRadius*sin(dRad));
			dVMax = std::max(dVMax,m_dRadius*sin(dRad));
		}
	}
	const double dPointU = R3AxisComponent(m_r3pAxisPoint,n1);
	const double dPointV = R3AxisComponent(m_r3pAxisPoint,n2);
	const CR3Point p1 = R3PointFromAxes(m_nAxis,m_dBase,dPointU+dUMin,dPointV+dVMin);
	const CR3Point p2 = R3PointFromAxes(m_nAxis,m_dTop,dPointU+dUMax,dPointV+dVMax);
	return CR3Box(p1,p2);
}

CTransformation CR3CylinderSegment::GetCanonicalTransformation() const
{
	//the rows of the matrix are the images of the canonical basis vectors, the last one the origin:
	//canonical Z runs along the axis, canonical X and Y along the two axes across it
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_Y;
	R3CrossAxes(m_nAxis,n1,n2);
	const CR3Vector v1 = R3AxisDirection(n1);
	const CR3Vector v2 = R3AxisDirection(n2);
	const CR3Vector vA = R3AxisDirection(m_nAxis);
	const CR3Point pOrigin = GetBasePoint();
	CTransformation tr(1.0);
	tr.M[0][0] = v1.GetX();	tr.M[0][1] = v1.GetY();	tr.M[0][2] = v1.GetZ();
	tr.M[1][0] = v2.GetX();	tr.M[1][1] = v2.GetY();	tr.M[1][2] = v2.GetZ();
	tr.M[2][0] = vA.GetX();	tr.M[2][1] = vA.GetY();	tr.M[2][2] = vA.GetZ();
	tr.M[3][0] = pOrigin.GetX();
	tr.M[3][1] = pOrigin.GetY();
	tr.M[3][2] = pOrigin.GetZ();
	return tr;
}

CTransformation CR3CylinderSegment::GetInvertedCanonicalTransformation() const
{
	//CTransformation::Inverted refuses to invert a general matrix, and rightly: this one is a
	//rotation that permutes the axes and a translation, so the inverse is written out. The linear
	//part is orthonormal, so its inverse is its transpose, and the translation follows it.
	const CTransformation tr = GetCanonicalTransformation();
	CTransformation trInv(1.0);
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			trInv.M[i][j] = tr.M[j][i];
	for(int j=0;j<3;j++){
		double d = 0.0;
		for(int k=0;k<3;k++)
			d += tr.M[3][k]*tr.M[j][k];
		trInv.M[3][j] = -d;
	}
	return trInv;
}

CR3Point CR3CylinderSegment::GetBasePoint() const
{
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_Y;
	R3CrossAxes(m_nAxis,n1,n2);
	return R3PointFromAxes(m_nAxis,m_dBase,R3AxisComponent(m_r3pAxisPoint,n1)
		,R3AxisComponent(m_r3pAxisPoint,n2));
}

CR3Point CR3CylinderSegment::GetTopPoint() const
{
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_Y;
	R3CrossAxes(m_nAxis,n1,n2);
	return R3PointFromAxes(m_nAxis,m_dTop,R3AxisComponent(m_r3pAxisPoint,n1)
		,R3AxisComponent(m_r3pAxisPoint,n2));
}

bool CR3CylinderSegment::operator==(const CR3CylinderSegment &o) const
{
	//the coordinate of the axis point along the axis says nothing, so it is not compared
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_Y;
	R3CrossAxes(m_nAxis,n1,n2);
	return m_nAxis==o.m_nAxis
		&& are_equal(R3AxisComponent(m_r3pAxisPoint,n1),R3AxisComponent(o.m_r3pAxisPoint,n1))
		&& are_equal(R3AxisComponent(m_r3pAxisPoint,n2),R3AxisComponent(o.m_r3pAxisPoint,n2))
		&& are_equal(m_dBase,o.m_dBase)
		&& are_equal(m_dTop,o.m_dTop)
		&& are_equal(m_dRadius,o.m_dRadius)
		&& are_equal(m_dMinAngleDeg,o.m_dMinAngleDeg)
		&& are_equal(m_dMaxAngleDeg,o.m_dMaxAngleDeg);
}

} // namespace kin5ax
