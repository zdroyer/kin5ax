// Transformation.cpp: implementation of the CTransformation class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "Transformation.h"
#include "GeneratorException.h"


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


//The rigid inverse: the transposed rotation block, with the translation carried back through it.
//
//It used to return the identity - a wrong answer for every transformation but one - and then, from
//2026-10-05, it threw, because nothing called it and the inverses the library needed were built where
//they were known: CRotation::Inverted (the transpose), CTranslation::Inverted, CScale::Inverted,
//CChainSegment::GetInverted...Transformation (KnownBugs #52). The caller that comment waited for is
//the workpiece holder of a chain: a frame somebody measured on the machine, which arrives as a whole
//transformation and not as the rotation and the translation it was built from.
//
//Anything that is not rigid is still refused, loudly. The inverse of a scaled or sheared frame is not
//the transpose, and answering as though it were would be wrong in a way nothing downstream could see.
CTransformation CTransformation::Inverted() const
{
	//p' = p*R + t, so p = (p'-t)*R^T. The rows of the block are the images of the basis vectors, and
	//for the transpose to be the inverse they have to be a unit length and at right angles.
	for(int i=0;i<3;i++)
		for(int j=i;j<3;j++){
			double d = 0.0;
			for(int k=0;k<3;k++)
				d += M[i][k]*M[j][k];
			if(fabs(d-((i==j) ? 1.0 : 0.0))>1E-9)
				MY_THROW(CGeneratorException,"CTransformation::Inverted takes a rigid transformation,"
					" and the rotation block of this one is not orthonormal: it scales or shears, and"
					" the inverse of that is not the transpose. Invert the rotation, the translation"
					" or the scale it is made of instead.");
		}
	if(fabs(M[0][3])>1E-12 || fabs(M[1][3])>1E-12 || fabs(M[2][3])>1E-12 || fabs(M[3][3]-1.0)>1E-12)
		MY_THROW(CGeneratorException,"CTransformation::Inverted takes a rigid transformation, and this"
			" one has a perspective part");
	CTransformation res(1.0);
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			res.M[i][j] = M[j][i];
	for(int i=0;i<3;i++){
		double d = 0.0;
		for(int k=0;k<3;k++)
			d += M[3][k]*M[i][k];
		res.M[3][i] = -d;
	}
	return res;
}



} // namespace kin5ax
