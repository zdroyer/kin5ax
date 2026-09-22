#pragma once
#include "R3Vector.h"
#include "R3Point.h"

class CR3CaughtVector
{
public:
	CR3CaughtVector();
	CR3CaughtVector(const CR3Vector &v, const CR3Point &p);
	~CR3CaughtVector(void);
	
	CR3Vector m_v;
	CR3Point m_p;
};
