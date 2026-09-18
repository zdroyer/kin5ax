// Transformation.cpp: implementation of the CM4x4 class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "M4x4.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CM4x4::CM4x4(double i)
{
	this->MakeDiag(i,i,i,i);
}

CM4x4::~CM4x4() {}

CM4x4 CM4x4::operator* (const CM4x4 &b) const
{
	CM4x4 res(0);
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
			for(int k=0;k<4;k++)
				res.M[i][j]+=M[k][j]*b.M[i][k];
	return res;
}


CM4x4::CM4x4(double d1, double d2, double d3, double d4)
{
	this->MakeDiag(d1,d2,d3,d4);
}

void CM4x4::MakeDiag(double d1, double d2, double d3, double d4)
{
	M[0][1]=M[0][2]=M[0][3]=0;
	M[1][0]=M[1][2]=M[1][3]=0;
	M[2][0]=M[2][1]=M[2][3]=0;
	M[3][0]=M[3][1]=M[3][2]=0;
	M[0][0]=d1;
	M[1][1]=d2;
	M[2][2]=d3;
	M[3][3]=d4;
}

CM4x4 CM4x4::T() const
{
	CM4x4 res(0);
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++)
			res.M[i][j]=this->M[j][i];
	return res;
}

void CM4x4::Standardize()
{
	if(M[3][3]==0)
		MY_THROW(CDivByZeroMyException,"Standarization by ZERO (M[3][3]==0)");
	if(M[3][3]!=1.0)
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++)
				M[i][j]=M[i][j]/M[3][3];
}

CM4x4 CM4x4::Inverted() const
{
	MY_THROW(CGeneratorException,"Inversion of matix not yet implemented");
}


