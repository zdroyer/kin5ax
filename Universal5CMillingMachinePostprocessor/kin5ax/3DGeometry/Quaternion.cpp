// Qutaternion.cpp: implementation of the CQutaternion class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "Quaternion.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQuaternion::CQuaternion(const double init)
{ 	q[0]=q[1]=q[2]=q[3]=init; }

CQuaternion::CQuaternion(const double Re, const double Imi, const double Imj, const double Imk)
{
	q[0]=Re;
	q[1]=Imi;
	q[2]=Imj;
	q[3]=Imk;
	Normalize();
}

CQuaternion::CQuaternion(const CR3Vector &v, const double angle)
{
	q[0]=cos(angle/2.0);
	double s=sin(angle/2.0);
	CR3Vector vn(1.0,0.0,0.0);
	if(v.GetLength()>1E-12)
			vn = v.GetUnitVector();
	q[1]=s*vn.GetX();
	q[2]=s*vn.GetY();
	q[3]=s*vn.GetZ();
	Normalize();
}

CQuaternion::CQuaternion(const CR3Vector &first, const CR3Vector &second)
{
	double cosa = first.GetUnitVector() * second.GetUnitVector();
	CR3Vector v = first.GetUnitVector().vectorProduct(second.GetUnitVector());
	double sina = v.GetLength();
	double angle = atan2(sina,cosa);
	q[0]=cos(angle/2.0);
	double s=sin(angle/2.0);
	CR3Vector vn(1.0,0.0,0.0);
	if(v.GetLength()>1E-12)
			vn = v.GetUnitVector();
	q[1]=s * vn.GetX();
	q[2]=s * vn.GetY();
	q[3]=s * vn.GetZ();
	Normalize();
}



CQuaternion::~CQuaternion()
{ }


CQuaternion operator*(const CQuaternion &Q, const CR3Vector &w)
{
	CQuaternion Qt((-Q.q[1]*w.GetX()-Q.q[2]*w.GetY()-Q.q[3]*w.GetZ())
	              ,( Q.q[0]*w.GetX()-Q.q[3]*w.GetY()+Q.q[2]*w.GetZ())
		          ,( Q.q[3]*w.GetX()+Q.q[0]*w.GetY()-Q.q[1]*w.GetZ())
		          ,(-Q.q[2]*w.GetX()+Q.q[1]*w.GetY()+Q.q[0]*w.GetZ()));

    //Qt.Normalize();
	return Qt;
	//*/
}

CQuaternion operator*(const CQuaternion &a, const CQuaternion &b)
{
	CQuaternion res;
	res.q[0]=a.q[0]*b.q[0] - a.q[1]*b.q[1] - a.q[2]*b.q[2] - a.q[3]*b.q[3];
	res.q[1]=a.q[0]*b.q[1] + a.q[1]*b.q[0] + a.q[2]*b.q[3] - a.q[3]*b.q[2];
	res.q[2]=a.q[0]*b.q[2] - a.q[1]*b.q[3] + a.q[2]*b.q[0] + a.q[3]*b.q[1];
	res.q[3]=a.q[0]*b.q[3] + a.q[1]*b.q[2] - a.q[2]*b.q[1] + a.q[3]*b.q[0];
	return res;
}
CQuaternion operator*(const CQuaternion &a, const double &b)
{
	CQuaternion res;
	res.q[0]=a.q[0]*b;
	res.q[1]=a.q[1]*b;
	res.q[2]=a.q[2]*b;
	res.q[3]=a.q[3]*b;
	return res;
}
CQuaternion operator*(const double &a, const CQuaternion &b)
{  	return b*a;  }

CQuaternion operator/(const CQuaternion &a,const double &b )
{  	
	CQuaternion res;
	res.q[0]=a.q[0]/b;
	res.q[1]=a.q[1]/b;
	res.q[2]=a.q[2]/b;
	res.q[3]=a.q[3]/b;
	return res;
}

std::ostream & operator << (std::ostream &o, const CQuaternion &a)
{
	o<<"["<<a.q[0]<<", "<<a.q[1]<<", "<<a.q[2]<<", "<<a.q[3]<<"]";
	return o;
}

	 CR3Vector CQuaternion::AxisOfRotation(void) const 
	{
		return CR3Vector(q[1],q[2],q[3]);
	}

	 double CQuaternion::AngleOfRotation(void) const {
		double sign = 1.0;
		if(q[1]!=0.0){
			sign*=q[1];
		}else{
			if(q[2]!=0.0){
				sign*=q[2];
			} else {
				if(q[3]!=0.0)
					sign*=q[3];
			}
		}
		if(sign>0)
			sign=2.0;
		else
			sign=-2.0;
		double ac = atan2(sqrt(q[1]*q[1]+q[2]*q[2]+q[3]*q[3]),q[0]);
		//ac=(360.0/(4*asin(1)))*ac;
 		return fmod(2.0*ac,2.0*3.14/*360.0*/);
//*/
	}
