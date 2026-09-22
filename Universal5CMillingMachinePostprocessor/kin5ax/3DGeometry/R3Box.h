#pragma once


#include "R3Point.h"

namespace kin5ax {

class CR3Box
{
protected:
	double m_dMinX, m_dMaxX;
	double m_dMinY, m_dMaxY;
	double m_dMinZ, m_dMaxZ;



	double Max(double d1, double d2) const 
	{
		if(d1>=d2)
			return d1;
		return d2;
	}
	double Min(double d1, double d2) const 
	{
		if(d1<=d2)
			return d1;
		return d2;
	}


	//One half of the box-box test: does an edge of the other box pass through this box?
	//box - the other box in its own frame; tr - from the frame of box to the frame of this box.
	bool AsymmetricIntersection(const CR3Box &box, const CTransformation &tr) const;

public:

	//A box with all six limits equal to zero (the default constructor) is empty: it never contains or meets anything.
	bool IsEmpty() const;
	//Does this box meet a box given in another frame (touching counts as meeting)?
	//box - the other box in its own frame; tr - from the frame of box to the frame of this box; tr_inv - the inverse of tr.
	//Two convex bodies meet iff an edge of one of them passes through the other, so both directions are tested.
	bool HasIntersectionWithBox(const CR3Box &box, const CTransformation &tr,const CTransformation &tr_inv) const;
	//Does the segment p1-p2 (in the frame of this box) have a point inside or on this box?
	bool HasIntersectionWithLineSegment(const CR3Point &p1, const CR3Point &p2) const;
	//Does the sphere with the centre p1 (in the frame of this box) and the radius dR meet this box?
	bool HasIntersectionWithSphere(const CR3Point &p1, const double &dR) const;
	//Does this box meet the cylinder x^2+y^2<=dR^2, 0<=z<=dZ given in its own (cylinder) frame?
	//trToBox - from the cylinder frame to the frame of this box; trToCylinder - the inverse of trToBox.
	bool HasIntersectionWithOZCylinder(const double &dZ,const double &dR, const CTransformation &trToBox,const CTransformation &trToCylinder) const;
	//Is the point p (in the frame of this box) inside or on this box (within dEPSILON)?
	bool Contains(const CR3Point &p)const;


	//Creates empty box
	CR3Box()
		:m_dMinX(0.0)
		,m_dMaxX(0.0)
		,m_dMinY(0.0)
		,m_dMaxY(0.0)
		,m_dMinZ(0.0)
		,m_dMaxZ(0.0)
	{}
	void ExpandToPoint(const CR3Point &p)
	{
		if(m_dMinX==m_dMaxX
		&& m_dMinX==0.0
		&& m_dMinY==m_dMaxY
		&& m_dMinY==0.0
		&& m_dMinZ==m_dMaxZ
		&& m_dMinZ==0.0)
		{
			m_dMinX = m_dMaxX = p.GetX();
			m_dMinY = m_dMaxY = p.GetY();
			m_dMinZ = m_dMaxZ = p.GetZ();
			return;
		}
			m_dMinX = Min(p.GetX(),m_dMinX);
			m_dMinY = Min(p.GetY(),m_dMinY);
			m_dMinZ = Min(p.GetZ(),m_dMinZ);
			
			m_dMaxX = Max(p.GetX(),m_dMaxX);
			m_dMaxY = Max(p.GetY(),m_dMaxY);
			m_dMaxZ = Max(p.GetZ(),m_dMaxZ);
	}
	void SetBox(const CR3Box &box)
	{
		m_dMaxX = box.m_dMaxX;
		m_dMaxY = box.m_dMaxY;
		m_dMaxZ = box.m_dMaxZ;
		m_dMinX = box.m_dMinX;
		m_dMinY = box.m_dMinY;
		m_dMinZ = box.m_dMinZ;
	}

	//The biggest box inside the cylinder x^2+y^2<=dRadius^2, dZ1<=z<=dZ2 (square side dRadius*sqrt(2)): a hit of it is a hit of the cylinder.
	static CR3Box InnerBoxFromOZCylinder(double dRadius, double dZ1, double dZ2);
	//The smallest box around the cylinder x^2+y^2<=dRadius^2, dZ1<=z<=dZ2: a miss of it is a miss of the cylinder.
	static CR3Box OuterBoxFromOZCylinder(double dRadius, double dZ1, double dZ2);
	//The bounding box of the cylinder with the axis p1-p2 and the radius dR.
	static CR3Box BoxFromCylinder(const CR3Point &p1, const CR3Point &p2, double dR);
	//The box dXSize x dYSize centred on the OZ axis, from z=0 to z=dZ.
	static CR3Box BoxFromOZBox(double dXSize, double dYSize, double dZ);

	void ExpandXYZ(double dX, double dY, double dZ);
	
	CR3Box operator + (const CR3Box &b2) const;
	CR3Box(const CR3Point &r3pO, double dRadius);

	CR3Box(double dMinX,double dMaxX,double dMinY,double dMaxY,double dMinZ,double dMaxZ)
	{
		m_dMinX=Min(dMinX,dMaxX);
		m_dMaxX=Max(dMaxX,dMinX);
		m_dMinY=Min(dMinY,dMaxY);
		m_dMaxY=Max(dMaxY,dMinY);
		m_dMinZ=Min(dMinZ,dMaxZ);
		m_dMaxZ=Max(dMaxZ,dMinZ);
	}

	CR3Box(const CR3Point &r3pP1, const CR3Point &r3pP2){
		m_dMinX=Min(r3pP1.GetX(),r3pP2.GetX());
		m_dMaxX=Max(r3pP1.GetX(),r3pP2.GetX());
		m_dMinY=Min(r3pP1.GetY(),r3pP2.GetY());
		m_dMaxY=Max(r3pP1.GetY(),r3pP2.GetY());
		m_dMinZ=Min(r3pP1.GetZ(),r3pP2.GetZ());
		m_dMaxZ=Max(r3pP1.GetZ(),r3pP2.GetZ());
	}
	CR3Point GetMinPoint()const
	{
		return CR3Point(m_dMinX,m_dMinY,m_dMinZ);
	}
	
	CR3Point GetMaxPoint()const
	{
		return CR3Point(m_dMaxX,m_dMaxY,m_dMaxZ);
	}

	double GetMaxSize()const
	{
		return Max(Max(fabs(m_dMaxX-m_dMinX),fabs(m_dMaxY-m_dMinY)),fabs(m_dMaxZ-m_dMinZ));
	}
	

	std::string toXML() const;
	
	~CR3Box(void){}
	
};

} // namespace kin5ax
