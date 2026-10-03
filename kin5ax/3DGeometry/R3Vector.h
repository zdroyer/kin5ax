#pragma once
#include "double_tools.h"
#include "DivByZeroMyException.h"
#include "string_tools.h"

namespace kin5ax {

class CR3Vector  
{
	double m_dX,m_dY,m_dZ;
	double m_vdV[3];

public:
	const CR3Vector & operator+=(const CR3Vector &v);
	CR3Vector operator+(const CR3Vector &v) const;
	CR3Vector operator-(const CR3Vector &v) const;
	int operator==(const CR3Vector &v) const;
	double operator * (const CR3Vector &v) const;

	CR3Vector operator-() const
	{return CR3Vector(-m_dX,-m_dY,-m_dZ);}

	int Colinearity(const CR3Vector &v) const{
		if(is_zero(GetLengthSquare()))
			return 1;
		if(is_zero(v.GetLengthSquare()))
			return 1;
		double dCos = this->GetUnitVector().scalarProduct(v.GetUnitVector());
		if(dCos>1.0-dEPSILON)
			return 1;
		if(dCos<-1.0+dEPSILON)
			return -1;
		return 0;		
	}
	bool IsZero(double dTolerance=dEPSILON) const {
		return (is_zero(GetX(),dTolerance)
			&& is_zero(GetY(),dTolerance)
			&& is_zero(GetZ(),dTolerance));
 	}
	void CopyCoordinates(double &dX, double &dY, double &dZ)const{
		dX=GetX();
		dY=GetY();
		dZ=GetZ();
	}

	bool IsEqual(const CR3Vector &v, double dTolerance=dEPSILON)const {
		return (are_equal(GetX(),v.GetX(),dTolerance)
				&& are_equal(GetY(),v.GetY(),dTolerance)
				&& are_equal(GetZ(),v.GetZ(),dTolerance));
	}
	bool IsOpposite(const CR3Vector &v, double dTolerance=dEPSILON) const{
		return (are_equal(GetX(),-v.GetX(),dTolerance)
				&& are_equal(GetY(),-v.GetY(),dTolerance)
				&& are_equal(GetZ(),-v.GetZ(),dTolerance));
	}
	CR3Vector operator /(double d) const
	{
		if(d==0)
			MY_THROW(CDivByZeroMyException,"Division vector by ZERO");
		return CR3Vector(m_dX/d,m_dY/d,m_dZ/d);
	}

	const CR3Vector &operator /=(double d)
	{
		if(d==0)
			MY_THROW(CDivByZeroMyException,"Division vector by ZERO");
		m_dX/=d;
		m_dY/=d;
		m_dZ/=d;
		return (*this);
	}

	CR3Vector( double init=0.0){
  		m_dX=m_dY=m_dZ=init;  
	}
	CR3Vector(double _x, double _y, double _z){
		m_dX=_x;
		m_dY=_y;
		m_dZ=_z;

	}
	

	~CR3Vector(){}
	double GetZ()const{
		return m_dZ;
	}
	double GetY()const{
		return m_dY;
	}
	
	double GetX() const {
		return m_dX;
	}
	double GetXYLength() const
	{
		return sqrt(m_dX*m_dX+m_dY*m_dY);
	}

	double GetXYLengthSquare() const
	{
		return m_dX*m_dX+m_dY*m_dY;
	}
	double GetXZLengthSquare() const
	{
		return m_dX*m_dX+m_dZ*m_dZ;
	}
	double GetYZLengthSquare() const
	{
		return m_dZ*m_dZ+m_dY*m_dY;
	}

	double GetLength() const
	{	return sqrt(m_dX*m_dX+m_dY*m_dY+m_dZ*m_dZ); }

	double GetLengthSquare() const
	{	return (m_dX*m_dX+m_dY*m_dY+m_dZ*m_dZ); }


	CR3Vector GetUnitVector() const 
	{
		double d=GetLength();
		if(d==0)
			MY_THROW(CDivByZeroMyException,"Attempt to normalize zero-length vector");
		return (*this)/d;
	}

	
	inline void SetXYZ(double _x, double _y, double _z)
	{
		m_dX=_x;
		m_dY=_y;
		m_dZ=_z;
	}

	inline void SetX(double _x)
	{
		m_dX=_x;
	}
	inline void SetY(double _y)
	{
		m_dY=_y;
	}
	inline void SetZ(double _z)
	{
		m_dZ=_z;
	}


	const double * toGL3dv(void)
	{
		m_vdV[0]=m_dX;
		m_vdV[1]=m_dY;
		m_vdV[2]=m_dZ;
		return m_vdV;
	}
	std::string toString() const
	{
		return string_printf("[%.4lf; %.4lf; %.4lf]",GetX(),GetY(),GetZ());
	}

	CR3Vector vectorProduct(const CR3Vector &b) const 
	{
	return CR3Vector((GetY()*b.GetZ()-GetZ()*b.GetY()),
					 (GetZ()*b.GetX()-GetX()*b.GetZ()),
					 (GetX()*b.GetY()-GetY()*b.GetX()));
	}
	double scalarProduct(const CR3Vector &v) const 
	{
		return (*this)*v;
	}

	CR3Vector operator *(double d) const {
		return CR3Vector(m_dX*d,m_dY*d,m_dZ*d);
	}

	//value in radians
	double UndirectedAngle(const CR3Vector &v2) const{
		if(IsZero() || v2.IsZero())
			return 0.0;//acos(0.0);
		return GetUnitVector().UndirectedUnitVectorsAngle(v2.GetUnitVector());
	}

	double UndirectedUnitVectorsAngle(const CR3Vector &v2) const{
		double dCos = scalarProduct(v2);
		if(dCos>1.0)
			dCos=1.0;
		if(dCos<-1.0)
			dCos=-1.0;
		return acos(dCos);
	}

} ;




CR3Vector operator *(double d, const CR3Vector &v);

} // namespace kin5ax
