#pragma once

#include "M4x4.h"
#include "R3Vector.h"
#include "R3Point.h"



namespace kin5ax {

class CTransformation : public CM4x4
{

protected:
	double glM[16];
	
public:
	CTransformation(const CM4x4 &rhs);
	CTransformation(double i=1.0);//identycznosc
	CTransformation Multiply(const CTransformation &b) const;
	CTransformation T() const;
	CTransformation Inverted() const;
	virtual CR3Point operator* (const CR3Point &p) const;
	virtual CR3Vector operator* (const CR3Vector &v) const;

	CTransformation operator*(const CTransformation &t) const;
	virtual ~CTransformation();

	double * toGLMatrix(void)
	{
		memcpy(glM,M[0],4*sizeof(double));
		memcpy(glM+4,M[1],4*sizeof(double));
		memcpy(glM+8,M[2],4*sizeof(double));
		memcpy(glM+12,M[3],4*sizeof(double));
		return glM;
	}

	CTransformation(const double * glM){
		memcpy(M[0],glM,4*sizeof(double));
		memcpy(M[1],glM+4,4*sizeof(double));
		memcpy(M[2],glM+8,4*sizeof(double));
		memcpy(M[3],glM+12,4*sizeof(double));
	}
	void ZeroTranslation() {
			M[3][0]=0;
			M[3][1]=0;
			M[3][2]=0;
	}
	void ZeroRotation(double dDiagValue = 1.0)
	{
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++)
				if(i==j)
					M[i][j]=dDiagValue;
				else
					M[i][j]=0.0;
 
	}


};


} // namespace kin5ax
