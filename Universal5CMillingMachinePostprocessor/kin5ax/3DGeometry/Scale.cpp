// Scale.cpp: implementation of the CScale class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "Scale.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace kin5ax {

CScale::CScale():CTransformation(1)
{}

CScale::~CScale()
{}

CScale::CScale(double s):CTransformation(CM4x4(s,s,s,1))
{}

CScale::CScale(double sx, double sy, double sz):CTransformation(CM4x4(sx,sy,sz,1))
{}

/*
CScale CScale::operator *(const CScale &t) const
{
	CScale res(t.M[0][0]*M[0][0], 
			   t.M[1][1]*M[1][1],
			   t.M[2][2]*M[2][2]);	
	return res;
}
*/
CScale CScale::Inverted() const
{
	return CScale(1.0/M[0][0],1.0/M[1][1],1.0/M[2][2]);
}


} // namespace kin5ax
