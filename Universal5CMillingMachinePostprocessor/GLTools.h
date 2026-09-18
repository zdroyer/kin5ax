#pragma once

#include "GeneratorException.h"

#include "3DGeometry/translation.h"
#include "3DGeometry/scale.h"
#include "3DGeometry/rotation.h"
#include "3DGeometry/r3caughtvector.h"


class CGLTools
{
public:

	static void DrawOZAxis(double dBrighness,double dAxesLength);
	static void DrawAxes(double d=1.0, double dAxesLength=1.0);
	static void Arrow(const CR3CaughtVector &cv, double r, double g, double b);
	static void Arrow(const CR3Point &tail,const CR3Point &head, double r, double g, double b);
	static void LineArrow(const CR3Point &p, const CR3Vector &v) ;
	
	CGLTools(void);
	~CGLTools(void);

} ;
