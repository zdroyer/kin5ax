#pragma once
#include "R3Vector.h"

namespace kin5ax {

class CQuaternion  
{
friend  CQuaternion operator* (const CQuaternion &a, const CQuaternion &b);
friend   CQuaternion operator* (const CQuaternion &a, const double &b);
friend  CQuaternion operator* (const double &a, const CQuaternion &b);
friend   std::ostream & operator << (std::ostream &o, const CQuaternion &a);
friend  CQuaternion operator*(const CQuaternion &Q, const CR3Vector &w);
friend   CQuaternion operator/(const CQuaternion &a,const double &b );
protected:
	double q[4];

public:
	CQuaternion(const double init=0);
	CQuaternion(const double Re, const double Imi, const double Imj, const double Imk);
	CQuaternion(const CR3Vector & v, const double angle/*[rad]*/);
	CQuaternion(const CR3Vector &first, const CR3Vector &second);
	virtual ~CQuaternion();
	 CR3Vector AxisOfRotation(void)const ;
	 double AngleOfRotation(void) const ; //returns angle in radians
	 inline void Normalize(void)
	{
		double norm = sqrt(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3]);
		if(norm>1E-12){
			q[0]/=norm;
			q[1]/=norm;
			q[2]/=norm;
			q[3]/=norm;
		}
	}


	CQuaternion operator + (const CQuaternion &Q)
	{
		CQuaternion Q2(Q);
		//Q2.Normalize();
		//Normalize();
		CQuaternion result(q[0]+Q2.q[0],q[1]+Q2.q[1],q[2]+Q2.q[2],q[3]+Q2.q[3]);
		//result.Normalize();
		return result;
	}
	CQuaternion operator - (const CQuaternion &Q)
	{
		CQuaternion Q2(Q);
		//Q2.Normalize();
		//Normalize();
		CQuaternion result(q[0]-Q2.q[0],q[1]-Q2.q[1],q[2]-Q2.q[2],q[3]-Q2.q[3]);
		//result.Normalize();
		return result;
	}
 	CQuaternion Opposite() const
	{
		CQuaternion res(q[0],-q[1],-q[2],-q[3]);
			return res;
	}
	CR3Vector Rotate(const CR3Vector &r3vV){
		double d=r3vV.GetLength();
		if(d<1E-12)
			return r3vV;
		CQuaternion v(0,r3vV.GetX()/d,r3vV.GetY()/d,r3vV.GetZ()/d);	
		return (((*this)*v*(this->Opposite())).AxisOfRotation()).GetUnitVector()*d;
	}

};

} // namespace kin5ax
