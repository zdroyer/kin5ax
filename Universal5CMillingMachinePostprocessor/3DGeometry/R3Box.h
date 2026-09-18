#pragma once


#include "r3point.h"

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


	bool AsimetricIntersection(const CR3Box &box, const CTransformation &tr) const;
	bool LineSegmentAndOZCylinderIntersection(const CR3Point &p1, const CR3Point &p2, const double &dZ, const double &dR) const;

public:

	bool IsEmpty() const;
	bool HasIntersectionWithBox(const CR3Box &box, const CTransformation &tr,const CTransformation &tr_inv) const;
	bool HasIntersectionWithLineSegment(const CR3Point &p1, const CR3Point &p2) const;
	bool HasIntersectionWithSphere(const CR3Point &p1, const double &dR) const;
	bool HasIntersectionWithOZCylinder(const double &dZ,const double &dR, const CTransformation &trToBox,const CTransformation &trToCylinder) const;
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

	static CR3Box InnerBoxFromOZCylinder(double dRadius, double dZ1, double dZ2);
	static CR3Box OuterBoxFromOZCylinder(double dRadius, double dZ1, double dZ2);
	static CR3Box BoxFromCylinder(const CR3Point &p1, const CR3Point &p2, double dR);
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
	
	void Render3DWire(double dR=1.0, double dG=1.0, double dB=1.0) const;
	void Render3DSolid() const;

	string toXML() const;
	
	~CR3Box(void){}
	
};
