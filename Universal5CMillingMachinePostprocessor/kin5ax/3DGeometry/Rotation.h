#pragma once
#include "Transformation.h"

namespace kin5ax {

class CRotation : public CTransformation  
{

public:
	static CRotation OZRotation(double dRadianAngle);
	CTransformation Inverted() const;		
	CRotation(const CR3Vector &v, double angleRadian);
	CRotation(const CR3Vector &v, int angleDegree);
	CRotation(const CR3Vector &v, double angleDegree, double);
	//Euler  (angle in degrees)
	CRotation(double alpha, double beta, double gamma);
		
	//v0 is rotated to be v1
	CRotation(const CR3Vector &v0, const CR3Vector &v1);
	

	CRotation operator * (const CRotation &b) const;
	double GetAngle() const;
	CR3Vector GetAxis() const;
	void GetEulerAngles(double &alpha, double &beta, double &gamma) const; 
	CRotation();
	virtual ~CRotation();
	inline CR3Point operator *(const CR3Point&p){ return CTransformation::operator *(p);}
	inline CR3Vector operator *(const CR3Vector &v){ return CTransformation::operator *(v);}
	CRotation(const CTransformation &t);

};

} // namespace kin5ax
