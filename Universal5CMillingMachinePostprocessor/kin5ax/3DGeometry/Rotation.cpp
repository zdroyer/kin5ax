// Rotation.cpp: implementation of the CRotation class.
//
//////////////////////////////////////////////////////////////////////
#include "../stdafx.h"
#include "Rotation.h"

//#include "../exceptions.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRotation::CRotation():CTransformation(1.0)
{}

CRotation CRotation::OZRotation(double dRadianAngle)
{
	CRotation result;
	double dSin = sin(dRadianAngle);
	double dCos = cos(dRadianAngle);
	result.SetAt(0,0,dCos);
	result.SetAt(1,1,dCos);
	result.SetAt(0,1,dSin);
	result.SetAt(1,0,-dSin);
	return result;
}
CRotation::~CRotation()
{}

CRotation::CRotation(const CTransformation &t):CTransformation(t)
{}

CRotation::CRotation(double alpha, double beta, double gamma):CTransformation(1)
{
	static double d = asin(1.0)/90.0;
	alpha*=d;
	beta*=d;
	gamma*=d;
	CRotation r1(CR3Vector(0,0,1),alpha);
	CRotation r2(CR3Vector(1,0,0),beta);
	CRotation r3(CR3Vector(0,0,1),gamma);
	CRotation r(r1*r2*r3);
	memcpy(M,r.M,16*sizeof(double));    	
}

void CRotation::GetEulerAngles(double &alpha, double &beta, double &gamma) const
{
	alpha = atan2(GetM(2,0),-GetM(2,1));
	beta = atan2(GetM(2,0)*sin(alpha)-GetM(2,1)*cos(alpha),GetM(2,2));
	gamma = atan2(-GetM(1,0)*cos(alpha)-GetM(1,1)*sin(alpha), GetM(0,0)*cos(alpha)+GetM(0,1)*sin(alpha));
	double d = 90.0/asin(1.0);
	alpha*=d;
	beta*=d;
	gamma*=d;
}


CRotation::CRotation(const CR3Vector &v, double angleDegree, double)
{
	CRotation r(v,degrees_to_radians( angleDegree ));
	memcpy(M,r.M,16*sizeof(double));    	
}




CRotation::CRotation(const CR3Vector &v, double angle):CTransformation(1)
{
	if(v.IsZero()){
		MakeDiag(1.0,1.0,1.0,1.0);
		return;
	}
	double s=sin(angle), c=cos(angle), t=1-c;
	CR3Vector v1=v.GetUnitVector();
	double x=v1.GetX(), y=v1.GetY(), z=v1.GetZ();

	M[0][0]=t*x*x+c;
	M[0][1]=t*x*y+s*z;
	M[0][2]=t*x*z-s*y; 

	M[1][0]=t*x*y-s*z;
	M[1][1]=t*y*y+c;
	M[1][2]=t*y*z+s*x;

	M[2][0]=t*x*z+s*y; 
	M[2][1]=t*y*z-s*x;
	M[2][2]=t*z*z+c;
}

CRotation::CRotation(const CR3Vector &r3vV0, const CR3Vector &r3vV1)
{
	CR3Vector v0 = r3vV0.GetUnitVector();
	CR3Vector v1 = r3vV1.GetUnitVector();
	//the vector product of two unit vectors is the rotation axis, but its length is the sine
	//of their angle, not 1: the (axis, angle) constructor normalises it
	CR3Vector axis = v0.vectorProduct(v1);
	double s=axis.GetLength();
	double c=v0.scalarProduct(v1);
	if(is_zero(s,1E-12)){
		if(c>0.0)
			return;	//the same direction: identity
		//opposite directions: half a turn about any axis perpendicular to v0
		axis = v0.vectorProduct(CR3Vector(1.0,0.0,0.0));
		if(is_zero(axis.GetLength(),1E-6))
			axis = v0.vectorProduct(CR3Vector(0.0,1.0,0.0));
	}
	CRotation r(axis,atan2(s,c));
	memcpy(M,r.M,16*sizeof(double));
}

CR3Vector CRotation::GetAxis() const 
{
	//double c = (M[0][0]+M[1][1]+M[2][2]-1)/2.0;
//	double s=2.0*sqrt(1-c*c);
	CR3Vector res( (M[1][2]-M[2][1]), ((M[2][0]-M[0][2])),((M[0][1]-M[1][0])));
/*	if(fabs(res.GetLength())<1E-12)
		return CR3Vector(0,0,1);
//*/
	return res;
}

double CRotation::GetAngle() const
{	
	static const double d = 90.0/asin(1.0);
	double ac = acos((M[0][0]+M[1][1]+M[2][2]-1)/2.0); 
	ac *= d;
	return ac;
}




CRotation CRotation::operator *(const CRotation &b) const
{	
	return ((CTransformation)(*this)*b);
}


CRotation::CRotation(const CR3Vector &v, int angleDegrees):CTransformation(1)
{
    angleDegrees%=360;
	CRotation r(v,degrees_to_radians( (double)angleDegrees ));
	memcpy(M,r.M,16*sizeof(double));    	
}

CTransformation CRotation::Inverted() const
{
	
	return T();
}
