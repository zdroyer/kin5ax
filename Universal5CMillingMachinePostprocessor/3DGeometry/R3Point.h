#pragma once

#include "double_tools.h"
#include "R3Point.h"
#include "R3Vector.h"


class CR3Point  
{
friend ostream & operator<<(ostream &out, const CR3Point &p);
friend class CTransformation;
friend CR3Point operator * (double d, const CR3Point &p);
friend CR3Point average(const CR3Point &p1, const CR3Point &p2);
friend CR3Point average1div3(const CR3Point &closer, const CR3Point &further);
friend CR3Point throwOver(const CR3Point &p1, const CR3Point &p2);
friend double d_NY(const CR3Point &p1,  const CR3Point &p2);
friend double d_E(const CR3Point &p1,  const CR3Point &p2);
friend double d_MAX(const CR3Point &p1,  const CR3Point &p2);
protected:
	double m_dV[3];

public:
	CR3Point(double dX, double dY, double dZ)
	{
		m_dV[0]=dX;
		m_dV[1]=dY;
		m_dV[2]=dZ;
	}
	CR3Point(double dInit=0.0)
	{
		m_dV[0]=dInit;
		m_dV[1]=dInit;
		m_dV[2]=dInit;
	}
/*
	CR3Point(const CR3Vector &r3vV)
	{
		m_dV[0]=r3vV.GetX();
		m_dV[1]=r3vV.GetY();
		m_dV[2]=r3vV.GetZ();
	}
//*/
	~CR3Point(){}

	inline double GetZ() const { return m_dV[2];	}
	inline double GetY() const { return m_dV[1];	}
	inline double GetX() const { return m_dV[0];	}

	inline void SetX(double dX) { m_dV[0] = dX; }
	inline void SetY(double dY) { m_dV[1] = dY; }
	inline void SetZ(double dZ) { m_dV[2] = dZ; }
	inline void SetPoint(const CR3Point &p)
	{ memcpy(m_dV,p.m_dV, 3*sizeof(double)); }

	inline void SetXYZ(double dX, double dY, double dZ)
	{ SetX(dX); SetY(dY); SetZ(dZ);}


	double DistanceSquareTo(const CR3Point &p) const {
		return (GetX()-p.GetX())*(GetX()-p.GetX())
			+(GetY()-p.GetY())*(GetY()-p.GetY())
			+(GetZ()-p.GetZ())*(GetZ()-p.GetZ());
	}
	double XYDistanceSquareTo(const double &dX,const double &dY) const {
		return (GetX()-dX)*(GetX()-dX)
			+(GetY()-dY)*(GetY()-dY);
	}
	double YZDistanceSquareTo(const double &dY,const double &dZ) const {
		return (GetY()-dY)*(GetY()-dY)
			+(GetZ()-dZ)*(GetZ()-dZ);
	}
	double XZDistanceSquareTo(const double &dX,const double &dZ) const {
		return (GetX()-dX)*(GetX()-dX)
			+(GetZ()-dZ)*(GetZ()-dZ);
	}

	double DistanceSquareTo(const double &dX, const double &dY,const double &dZ ) const{
		return (GetX()-dX)*(GetX()-dX)
			+(GetY()-dY)*(GetY()-dY)
			+(GetZ()-dZ)*(GetZ()-dZ);
	}

	void CopyCoordinates(double &dX, double &dY, double &dZ)const
	{
		dX=GetX();
		dY=GetY();
		dZ=GetZ();
	}

	inline CR3Point operator=(const CR3Point &p)
	{ 
		SetPoint(p);
		return CR3Point(p);
	}

	inline int operator==(const CR3Point &p) const
	{ return ( are_equal(GetX(),p.GetX())
			&& are_equal(GetY(),p.GetY())
			&& are_equal(GetZ(),p.GetZ())); 
	}
	
	inline int operator!=(const CR3Point &p) const
	{
		return ( are_not_equal(GetX(),p.GetX())
			|| are_not_equal(GetY(),p.GetY())
			|| are_not_equal(GetZ(),p.GetZ())); 
	}

	inline CR3Point operator + (const CR3Vector &r3vV)const
	{
		return CR3Point(GetX()+r3vV.GetX()
					,GetY()+r3vV.GetY()
					,GetZ()+r3vV.GetZ()) ;	
	}

	inline CR3Point &operator +=(const CR3Vector &r3vV)
	{
			SetXYZ(GetX()+r3vV.GetX()
					,GetY()+r3vV.GetY()
					,GetZ()+r3vV.GetZ()) ;	
		return (*this);
	}

	inline CR3Point operator + (const double &dD)const
	{
		return CR3Point(GetX()+dD
					,GetY()+dD
					,GetZ()+dD);
	}	

	inline CR3Point operator + (const CR3Point &p)const
	{
		return CR3Point(GetX()+p.GetX()
			,GetY()+p.GetY()
			,GetZ()+p.GetZ());
	}	

	inline CR3Point &operator +=(const double &dD)
	{
		SetXYZ(GetX()+dD
				,GetY()+dD
				,GetZ()+dD);
		return (*this);
	}	


	inline CR3Vector operator - (const CR3Point &r3vP)const
	{
		return CR3Vector(GetX()-r3vP.GetX()
						,GetY()-r3vP.GetY()
						,GetZ()-r3vP.GetZ());
	}
	inline CR3Point operator - (const CR3Vector &r3vV)const
	{
		return  CR3Point(GetX()-r3vV.GetX()
						,GetY()-r3vV.GetY()
						,GetZ()-r3vV.GetZ());
	
	}
	inline CR3Point &operator -=(const CR3Vector &r3vV){
		SetPoint((*this)-r3vV);
		return (*this);
	}

	inline CR3Point operator - (const double &dD)const
	{
		return  CR3Point(GetX()-dD
						,GetY()-dD
						,GetZ()-dD);
	}
	inline CR3Point &operator -=(const double &dD)
	{
		SetPoint((*this)-dD);
		return (*this);
	}
	
	inline CR3Point operator * (const double &dD) const 
	{	
		return CR3Point(dD*GetX(),dD*GetY(),dD*GetZ());
	}
	inline CR3Point &operator *= (const double &dD)
	{	
		SetPoint((*this)*dD);
		return (*this);
	}

	inline CR3Point operator / (const double &dD) const 
	{	
		if(is_zero(dD))
			MY_THROW(CDivByZeroMyException,"Attempt to division point coordinates by ZERO");
		return CR3Point(GetX()/dD,GetY()/dD,GetZ()/dD);
	}
	inline CR3Point &operator /= (const double &dD)
	{	
		SetPoint((*this)/dD);
		return (*this);
	}



	CR3Vector CR3Point::toR3Vector() const;
	string toString() const;
	string toStringInt() const;
	inline const double * toGL3dv(void) const
	{
		return m_dV;
	}


/*	{
		return CR3Point(p.GetX()+GetX(),p.GetY()+GetY(),p.GetZ()+GetZ());
	}
//*/


//	POINT toPOINT() const;


};

