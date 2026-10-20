#include "../stdafx.h"
#include "R3SphereSegment.h"
#include "double_tools.h"

#include <cmath>
#include <algorithm>

namespace kin5ax {

CR3SphereSegment::CR3SphereSegment()
	:m_r3pCentre(0.0)
	,m_dRadius(0.0)
	,m_nAxis(R3_AXIS_Z)
	,m_dMin(0.0)
	,m_dMax(0.0)
{
}

CR3SphereSegment::CR3SphereSegment(const CR3Point &r3pCentre, double dRadius)
	:m_r3pCentre(r3pCentre)
	,m_dRadius(fabs(dRadius))
	,m_nAxis(R3_AXIS_Z)
	,m_dMin(-fabs(dRadius))
	,m_dMax(fabs(dRadius))
{
}

CR3SphereSegment::CR3SphereSegment(const CR3Point &r3pCentre, double dRadius, R3_AXIS nAxis
	,double dMin, double dMax)
	:m_r3pCentre(r3pCentre)
	,m_dRadius(fabs(dRadius))
	,m_nAxis(nAxis)
	,m_dMin(0.0)
	,m_dMax(0.0)
{
	SetCut(dMin,dMax);
}

bool CR3SphereSegment::IsWholeSphere() const
{
	return is_less_or_equal(m_dMin,-m_dRadius) && is_more_or_equal(m_dMax,m_dRadius);
}

void CR3SphereSegment::SetRadius(double dRadius)
{
	const bool bWasWhole = IsWholeSphere();
	m_dRadius = fabs(dRadius);
	if(bWasWhole){
		m_dMin = -m_dRadius;
		m_dMax = m_dRadius;
	} else {
		SetCut(m_dMin,m_dMax);
	}
}

void CR3SphereSegment::SetCut(double dMin, double dMax)
{
	m_dMin = std::max(std::min(dMin,dMax),-m_dRadius);
	m_dMax = std::min(std::max(dMin,dMax),m_dRadius);
}

bool CR3SphereSegment::IsEmpty() const
{
	return is_zero(m_dRadius) || m_dMax-m_dMin<dEPSILON;
}

std::string CR3SphereSegment::Problem() const
{
	if(m_dRadius<0.0)
		return "the radius of a sphere cannot be negative";
	if(is_zero(m_dRadius))
		return "the sphere has no radius";
	if(m_dMax-m_dMin<dEPSILON)
		return "the two cuts of the sphere take all of it: they are at the same place";
	return "";
}

bool CR3SphereSegment::Contains(const CR3Point &p) const
{
	if(IsEmpty())
		return false;
	const double dX = p.GetX()-m_r3pCentre.GetX();
	const double dY = p.GetY()-m_r3pCentre.GetY();
	const double dZ = p.GetZ()-m_r3pCentre.GetZ();
	if(!is_less_or_equal(dX*dX+dY*dY+dZ*dZ,m_dRadius*m_dRadius))
		return false;
	if(IsWholeSphere())
		return true;
	const double dAlong = R3AxisComponent(p,m_nAxis)-R3AxisComponent(m_r3pCentre,m_nAxis);
	return is_between_or_equal(dAlong,m_dMin,m_dMax);
}

CR3Box CR3SphereSegment::GetBoundingBox() const
{
	if(IsEmpty())
		return CR3Box();
	//across the axis the widest circle of the piece decides: the equator when the cut straddles it,
	//otherwise the circle at the end of the cut that is nearer to it
	double dAcross = m_dRadius;
	if(!IsWholeSphere()){
		const double dNearest = (m_dMin>0.0) ? m_dMin : ((m_dMax<0.0) ? m_dMax : 0.0);
		dAcross = sqrt(std::max(0.0,m_dRadius*m_dRadius-dNearest*dNearest));
	}
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_Y;
	R3CrossAxes(m_nAxis,n1,n2);
	const double dAlong = R3AxisComponent(m_r3pCentre,m_nAxis);
	const double dU = R3AxisComponent(m_r3pCentre,n1);
	const double dV = R3AxisComponent(m_r3pCentre,n2);
	const CR3Point p1 = R3PointFromAxes(m_nAxis,dAlong+m_dMin,dU-dAcross,dV-dAcross);
	const CR3Point p2 = R3PointFromAxes(m_nAxis,dAlong+m_dMax,dU+dAcross,dV+dAcross);
	return CR3Box(p1,p2);
}

bool CR3SphereSegment::operator==(const CR3SphereSegment &o) const
{
	if(!are_equal(m_dRadius,o.m_dRadius)
		|| !are_equal(m_r3pCentre.GetX(),o.m_r3pCentre.GetX())
		|| !are_equal(m_r3pCentre.GetY(),o.m_r3pCentre.GetY())
		|| !are_equal(m_r3pCentre.GetZ(),o.m_r3pCentre.GetZ()))
		return false;
	if(IsWholeSphere() && o.IsWholeSphere())
		return true;								//the axis of a cut that takes nothing says nothing
	return m_nAxis==o.m_nAxis && are_equal(m_dMin,o.m_dMin) && are_equal(m_dMax,o.m_dMax);
}

} // namespace kin5ax
