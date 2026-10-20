#include "../stdafx.h"
#include "Solid.h"
#include "double_tools.h"

#include <cmath>
#include <algorithm>

namespace kin5ax {

namespace {

const char *const szTYPES[SOLID_TYPE_COUNT] = {"box","cylinder","sphere"};

double Dot(const CR3Point &a, const CR3Point &b)
{
	return a.GetX()*b.GetX()+a.GetY()*b.GetY()+a.GetZ()*b.GetZ();
}

CR3Point Minus(const CR3Point &a, const CR3Point &b)
{
	return CR3Point(a.GetX()-b.GetX(),a.GetY()-b.GetY(),a.GetZ()-b.GetZ());
}

double LengthSquare(const CR3Point &a)
{
	return Dot(a,a);
}

//the point of the segment a..b nearest to p, as the parameter 0..1 along it
double NearestOnSegment(const CR3Point &p, const CR3Point &a, const CR3Point &b)
{
	const CR3Point d = Minus(b,a);
	const double dLengthSquare = LengthSquare(d);
	if(is_zero(dLengthSquare))
		return 0.0;								//a segment of no length is its own end
	const double dT = Dot(Minus(p,a),d)/dLengthSquare;
	return std::max(0.0,std::min(1.0,dT));
}

CR3Point PointAt(const CR3Point &a, const CR3Point &b, double dT)
{
	return CR3Point(a.GetX()+dT*(b.GetX()-a.GetX())
		,a.GetY()+dT*(b.GetY()-a.GetY())
		,a.GetZ()+dT*(b.GetZ()-a.GetZ()));
}

double DistanceToSegment(const CR3Point &p, const CR3Point &a, const CR3Point &b)
{
	return sqrt(LengthSquare(Minus(p,PointAt(a,b,NearestOnSegment(p,a,b)))));
}

/**
 * The distance between two segments. The two parameters are found by walking the pair a few times -
 * each segment to the point nearest the other's current point - which converges in one step for
 * every pair that is not nearly parallel and is correct to the tolerance of this library for those
 * that are, because a nearly parallel pair has a nearly flat distance along its length.
 */
double DistanceBetweenSegments(const CR3Point &a1, const CR3Point &b1
	,const CR3Point &a2, const CR3Point &b2)
{
	double dT1 = 0.0, dT2 = 0.0;
	double dBest = -1.0;
	for(int i=0;i<16;i++){
		dT2 = NearestOnSegment(PointAt(a1,b1,dT1),a2,b2);
		dT1 = NearestOnSegment(PointAt(a2,b2,dT2),a1,b1);
		const double d = sqrt(LengthSquare(Minus(PointAt(a1,b1,dT1),PointAt(a2,b2,dT2))));
		if(dBest>=0.0 && dBest-d<dEPSILON)
			return d;
		dBest = d;
	}
	//and the four ends, which is what a parallel pair comes down to
	dBest = std::min(dBest,DistanceToSegment(a1,a2,b2));
	dBest = std::min(dBest,DistanceToSegment(b1,a2,b2));
	dBest = std::min(dBest,DistanceToSegment(a2,a1,b1));
	dBest = std::min(dBest,DistanceToSegment(b2,a1,b1));
	return dBest;
}

} // namespace

const char *SolidTypeName(SOLID_TYPE nType)
{
	return (nType>=0 && nType<SOLID_TYPE_COUNT) ? szTYPES[nType] : szTYPES[SOLID_BOX];
}

bool SolidTypeFromName(const std::string &sName, SOLID_TYPE &nType)
{
	for(int i=0;i<SOLID_TYPE_COUNT;i++)
		if(sName==szTYPES[i]){
			nType = (SOLID_TYPE)i;
			return true;
		}
	return false;
}

CSolid::CSolid()
	:m_nType(SOLID_BOX)
{
}

CSolid::CSolid(const CR3Box &oBox)
	:m_nType(SOLID_BOX)
	,m_oBox(oBox)
{
}

CSolid::CSolid(const CR3CylinderSegment &oCylinder)
	:m_nType(SOLID_CYLINDER)
	,m_oCylinder(oCylinder)
{
}

CSolid::CSolid(const CR3SphereSegment &oSphere)
	:m_nType(SOLID_SPHERE)
	,m_oSphere(oSphere)
{
}

void CSolid::SetBox(const CR3Box &oBox)
{
	m_nType = SOLID_BOX;
	m_oBox = oBox;
}

void CSolid::SetCylinder(const CR3CylinderSegment &oCylinder)
{
	m_nType = SOLID_CYLINDER;
	m_oCylinder = oCylinder;
}

void CSolid::SetSphere(const CR3SphereSegment &oSphere)
{
	m_nType = SOLID_SPHERE;
	m_oSphere = oSphere;
}

CR3Box CSolid::GetBoundingBox() const
{
	switch(m_nType){
	case SOLID_CYLINDER:	return m_oCylinder.GetBoundingBox();
	case SOLID_SPHERE:		return m_oSphere.GetBoundingBox();
	default:				return m_oBox;
	}
}

bool CSolid::IsEmpty() const
{
	switch(m_nType){
	case SOLID_CYLINDER:	return m_oCylinder.IsEmpty();
	case SOLID_SPHERE:		return m_oSphere.IsEmpty();
	default:				return m_oBox.IsEmpty();
	}
}

std::string CSolid::Problem() const
{
	switch(m_nType){
	case SOLID_CYLINDER:	return m_oCylinder.Problem();
	case SOLID_SPHERE:		return m_oSphere.Problem();
	default:
		return m_oBox.IsEmpty() ? "the box has no size: every one of its six limits is zero" : "";
	}
}

bool CSolid::Contains(const CR3Point &p) const
{
	switch(m_nType){
	case SOLID_CYLINDER:	return m_oCylinder.Contains(p);
	case SOLID_SPHERE:		return m_oSphere.Contains(p);
	default:				return !m_oBox.IsEmpty() && m_oBox.Contains(p);
	}
}

bool CSolid::Meets(const CSolid &oOther, const CTransformation &tr, const CTransformation &trInv) const
{
	if(IsEmpty() || oOther.IsEmpty())
		return false;
	//box against box is exact, and it is the whole of the test
	if(m_nType==SOLID_BOX && oOther.m_nType==SOLID_BOX)
		return m_oBox.HasIntersectionWithBox(oOther.m_oBox,tr,trInv);
	//the boxes of the two solids first: a cheap no, and the one place where the sector of a wedge
	//and the cut of a dome are taken into account
	if(!GetBoundingBox().HasIntersectionWithBox(oOther.GetBoundingBox(),tr,trInv))
		return false;

	if(m_nType==SOLID_BOX){
		if(oOther.m_nType==SOLID_CYLINDER){
			const CR3CylinderSegment &oC = oOther.m_oCylinder;
			return m_oBox.HasIntersectionWithOZCylinder(oC.GetLength(),oC.GetRadius()
				,tr*oC.GetCanonicalTransformation()
				,oC.GetInvertedCanonicalTransformation()*trInv);
		}
		return m_oBox.HasIntersectionWithSphere(tr*oOther.m_oSphere.GetCentre()
			,oOther.m_oSphere.GetRadius());
	}
	if(oOther.m_nType==SOLID_BOX){
		if(m_nType==SOLID_CYLINDER)
			return oOther.m_oBox.HasIntersectionWithOZCylinder(m_oCylinder.GetLength()
				,m_oCylinder.GetRadius()
				,trInv*m_oCylinder.GetCanonicalTransformation()
				,m_oCylinder.GetInvertedCanonicalTransformation()*tr);
		return oOther.m_oBox.HasIntersectionWithSphere(trInv*m_oSphere.GetCentre()
			,m_oSphere.GetRadius());
	}
	//neither is a box: the axes and the centres, in the frame of this solid
	if(m_nType==SOLID_SPHERE && oOther.m_nType==SOLID_SPHERE){
		const CR3Point pOther = tr*oOther.m_oSphere.GetCentre();
		const double dSum = m_oSphere.GetRadius()+oOther.m_oSphere.GetRadius();
		return is_less_or_equal(sqrt(LengthSquare(Minus(pOther,m_oSphere.GetCentre()))),dSum);
	}
	if(m_nType==SOLID_SPHERE){
		const CR3CylinderSegment &oC = oOther.m_oCylinder;
		const double d = DistanceToSegment(m_oSphere.GetCentre(),tr*oC.GetBasePoint()
			,tr*oC.GetTopPoint());
		return is_less_or_equal(d,m_oSphere.GetRadius()+oC.GetRadius());
	}
	if(oOther.m_nType==SOLID_SPHERE){
		const CR3Point pOther = tr*oOther.m_oSphere.GetCentre();
		const double d = DistanceToSegment(pOther,m_oCylinder.GetBasePoint()
			,m_oCylinder.GetTopPoint());
		return is_less_or_equal(d,m_oCylinder.GetRadius()+oOther.m_oSphere.GetRadius());
	}
	//two cylinders: the two axes, as segments
	const CR3CylinderSegment &oC = oOther.m_oCylinder;
	const double d = DistanceBetweenSegments(m_oCylinder.GetBasePoint(),m_oCylinder.GetTopPoint()
		,tr*oC.GetBasePoint(),tr*oC.GetTopPoint());
	return is_less_or_equal(d,m_oCylinder.GetRadius()+oC.GetRadius());
}

bool CSolid::operator==(const CSolid &o) const
{
	if(m_nType!=o.m_nType)
		return false;
	switch(m_nType){
	case SOLID_CYLINDER:	return m_oCylinder==o.m_oCylinder;
	case SOLID_SPHERE:		return m_oSphere==o.m_oSphere;
	default:				return m_oBox==o.m_oBox;
	}
}

} // namespace kin5ax
