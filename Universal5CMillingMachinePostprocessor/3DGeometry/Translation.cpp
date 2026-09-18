// Translation.cpp: implementation of the CTranslation class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "Translation.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CTranslation::~CTranslation()
{

}

//CTranslation::CTranslation(const CTransformation &t):CTransformation(t)
//{}

CTranslation::CTranslation(double tx, double ty, double tz):CTransformation(1)
{
	M[3][0]=tx;
	M[3][1]=ty;
	M[3][2]=tz;
}


CTranslation::CTranslation(const CR3Vector &v):CTransformation(1)
{
	M[3][0]=v.GetX();
	M[3][1]=v.GetY();
	M[3][2]=v.GetZ();
}

//
/*
CTranslation CTranslation::operator *(const CTranslation &t) const
{
	
	CTranslation res(t.M[3][0]+M[3][0],
					 t.M[3][1]+M[3][1],
					 t.M[3][2]+M[3][2]);
	return res;	
}
//*/
CTransformation CTranslation::Inverted() const
{
	
	return CTranslation(-M[3][0],-M[3][1],-M[3][2]);
	
}


