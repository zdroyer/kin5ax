// Rotation.cpp: implementation of the CRotation class.
//
//////////////////////////////////////////////////////////////////////
#include "../stdafx.h"
#include "Rotation.h"

//#include "../exceptions.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace kin5ax {

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

//The axis of the rotation as a unit vector, together with GetAngle a representation of it.
//A rotation by zero has no axis of its own: OZ is returned, and the angle is zero anyway.
CR3Vector CRotation::GetAxis() const
{
	//the antisymmetric part is 2*sin(angle)*axis, which vanishes at zero and at half a turn
	CR3Vector res( (M[1][2]-M[2][1]), ((M[2][0]-M[0][2])),((M[0][1]-M[1][0])));
	if(res.GetLength()>1E-9)
		return res.GetUnitVector();
	if(GetCosineOfAngle()>0.0)
		return CR3Vector(0.0,0.0,1.0);		//no rotation: every axis does
	//half a turn: the matrix is 2*n*n^T - I, so n[i]*n[i] = (M[i][i]+1)/2 and n[i]*n[j] = M[j][i]/2
	int nLargest = 0;
	for(int i=1;i<3;i++)
		if(M[i][i]>M[nLargest][nLargest])
			nLargest = i;
	double dLargest = sqrt(std::max(0.0,(M[nLargest][nLargest]+1.0)/2.0));
	if(dLargest<1E-12)
		return CR3Vector(0.0,0.0,1.0);		//not a rotation matrix
	double d[3] = {0.0,0.0,0.0};
	d[nLargest] = dLargest;
	for(int j=0;j<3;j++)
		if(j!=nLargest)
			d[j] = M[j][nLargest]/(2.0*dLargest);
	return CR3Vector(d[0],d[1],d[2]).GetUnitVector();
}

//the cosine of the angle of the rotation, clamped: rounding can push the trace of a product
//of rotations above 3, and acos of anything above 1 is not a number
double CRotation::GetCosineOfAngle() const
{
	double dCos = (M[0][0]+M[1][1]+M[2][2]-1.0)/2.0;
	if(dCos>1.0)
		return 1.0;
	if(dCos<-1.0)
		return -1.0;
	return dCos;
}

//the angle of the rotation in degrees, between 0 and 180
double CRotation::GetAngle() const
{
	static const double d = 90.0/asin(1.0);
	return acos(GetCosineOfAngle())*d;
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

} // namespace kin5ax
