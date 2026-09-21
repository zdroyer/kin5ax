
#pragma once
#include "../exceptions/ArgsMyException.h"
#include "../exceptions/DivByZeroMyException.h"
//#include "R4Vector.h"

class CM4x4  
{

protected:
	
public:
	inline double GetM(int i, int j) const
	{	return GetAt(i,j);	}

	inline double GetAt(int i, int j) const
	{	if(i>3 || i<0 || j>3 || i<0)
			MY_THROW(CArgsMyException,"Invalid index scope");
		return M[i][j];
	}

	inline void SetAt(int i, int j, double d)
	{	if(i>3 || i<0 || j>3 || i<0)
			MY_THROW(CArgsMyException,"Invalid index scope");
		M[i][j] = d;
	}

	
	double M[4][4];
	
	void Standardize();
	CM4x4 T() const;
	void MakeDiag(double d1, double d2, double d3, double d4);
	CM4x4 Inverted() const;
	virtual CM4x4 operator * (const CM4x4 &b) const;

	CM4x4(double d1,double d2, double d3,double d4);
	CM4x4(double i=1.0);//identycznosc
	virtual ~CM4x4();

};

//-------------------------------------------------------

