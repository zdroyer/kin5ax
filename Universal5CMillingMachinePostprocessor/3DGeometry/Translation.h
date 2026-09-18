#pragma once
#include "Transformation.h"


class CTranslation : public CTransformation  
{
	
public:
	//CTranslation  operator* (const CTranslation &t) const;
	CTransformation Inverted() const;
	CTranslation(const double tx, const double ty, const double tz);
	virtual ~CTranslation();
	CTranslation(const CR3Vector & v);

};
