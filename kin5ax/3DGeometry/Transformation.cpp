// Transformation.cpp: implementation of the CTransformation class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "Transformation.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace kin5ax {

CR3Vector r3vOX(1.0,0.0,0.0),r3vOY(0.0,1.0,0.0),r3vOZ(0.0,0.0,1.0);
CR3Point r3pZero(0.0);

CTransformation::CTransformation(double i):CM4x4(i)
{}
CTransformation::~CTransformation() 
{}

CTransformation CTransformation::Multiply(const CTransformation &b) const
{
	CTransformation res( (CTransformation)( (CM4x4)(*this)*(CM4x4)b) );
	res.Standardize();
	return res;
}


CTransformation CTransformation::operator*(const CTransformation &b) const
{
	CTransformation res( (CTransformation)( (CM4x4)(*this)*(CM4x4)b) );
	res.Standardize();
	return res;
}


std::ostream & operator << (std::ostream &out, const CTransformation &a)
{
	
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			out<<a.M[j][i]<<" ";
		}
		out<<std::endl;
	}
	//*/
	return out;
}

CR3Point CTransformation::operator * (const CR3Point &p) const
{
	CR3Point res;
	res.SetX(M[0][0]*(double)p.GetX()+M[1][0]*(double)p.GetY()+M[2][0]*(double)p.GetZ()+M[3][0]);
	res.SetY(M[0][1]*(double)p.GetX()+M[1][1]*(double)p.GetY()+M[2][1]*(double)p.GetZ()+M[3][1]);
	res.SetZ(M[0][2]*(double)p.GetX()+M[1][2]*(double)p.GetY()+M[2][2]*(double)p.GetZ()+M[3][2]);
	double dA =M[0][3]*(double)p.GetX()+M[1][3]*(double)p.GetY()+M[2][3]*(double)p.GetZ()+M[3][3];
	if(dA!=1.0 && dA!=0.0){
		res/=dA;
	}
	return res;
}



CR3Vector CTransformation::operator *(const CR3Vector &v) const 
{
	CR3Vector res(M[0][0]*v.GetX()+M[1][0]*v.GetY()+M[2][0]*v.GetZ(),
				  M[0][1]*v.GetX()+M[1][1]*v.GetY()+M[2][1]*v.GetZ(),
				  M[0][2]*v.GetX()+M[1][2]*v.GetY()+M[2][2]*v.GetZ());
	if(M[3][3]!=1){
		res.SetX(res.GetX()/M[3][3]);
		res.SetY(res.GetY()/M[3][3]);
		res.SetZ(res.GetZ()/M[3][3]);
	}
	return res;
}


CTransformation::CTransformation(const CM4x4 &rhs):CM4x4(rhs)
{}

CTransformation CTransformation::T() const
{
	return (CTransformation)(CM4x4::T());
}


CTransformation CTransformation::Inverted() const
{
	return CTransformation(1);
	//throw (CMyException("virtual CTransformation.Inverted() called"));
	return (CTransformation)(CM4x4::Inverted());
}



} // namespace kin5ax
