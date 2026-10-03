

#pragma once

#include "Transformation.h"


namespace kin5ax {

class CScale : public CTransformation  
{
public:
	void ToggleChangedObj();
	CScale Inverted() const;
	//CScale operator* (const CScale &t) const;
	CScale(double sx, double sy, double sz);
	CScale(double s);
	CScale();
	~CScale();
};


} // namespace kin5ax
