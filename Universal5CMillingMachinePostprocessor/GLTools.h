#pragma once

#include "GeneratorException.h"

#include "translation.h"
#include "scale.h"
#include "rotation.h"
#include "r3anchoredvector.h"


class CGLTools
{
public:

	static void DrawOZAxis(double dBrightness,double dAxesLength);
	static void DrawAxes(double d=1.0, double dAxesLength=1.0);
	static void Arrow(const CR3AnchoredVector &cv, double r, double g, double b);
	//a solid arrow (a thin cylinder with a cone) from tail to head in the colour r,g,b
	static void Arrow(const CR3Point &tail,const CR3Point &head, double r, double g, double b);
	static void LineArrow(const CR3Point &p, const CR3Vector &v) ;
	
	CGLTools(void);
	~CGLTools(void);

} ;
