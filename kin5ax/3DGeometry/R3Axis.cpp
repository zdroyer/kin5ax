#include "../stdafx.h"
#include "R3Axis.h"

namespace kin5ax {

namespace {

const char *const szNAMES[R3_AXIS_COUNT] = {"x","y","z"};

char Lower(char c)
{
	return (c>='A' && c<='Z') ? (char)(c-'A'+'a') : c;
}

} // namespace

const char *R3AxisName(R3_AXIS nAxis)
{
	return (nAxis>=0 && nAxis<R3_AXIS_COUNT) ? szNAMES[nAxis] : szNAMES[R3_AXIS_X];
}

bool R3AxisFromName(const std::string &sName, R3_AXIS &nAxis)
{
	if(sName.size()!=1)
		return false;
	for(int i=0;i<R3_AXIS_COUNT;i++)
		if(Lower(sName[0])==szNAMES[i][0]){
			nAxis = (R3_AXIS)i;
			return true;
		}
	return false;
}

double R3AxisComponent(const CR3Point &p, R3_AXIS nAxis)
{
	switch(nAxis){
	case R3_AXIS_X:	return p.GetX();
	case R3_AXIS_Y:	return p.GetY();
	default:		return p.GetZ();
	}
}

double R3AxisComponent(const CR3Vector &v, R3_AXIS nAxis)
{
	switch(nAxis){
	case R3_AXIS_X:	return v.GetX();
	case R3_AXIS_Y:	return v.GetY();
	default:		return v.GetZ();
	}
}

CR3Vector R3AxisDirection(R3_AXIS nAxis)
{
	switch(nAxis){
	case R3_AXIS_X:	return CR3Vector(1.0,0.0,0.0);
	case R3_AXIS_Y:	return CR3Vector(0.0,1.0,0.0);
	default:		return CR3Vector(0.0,0.0,1.0);
	}
}

void R3CrossAxes(R3_AXIS nAxis, R3_AXIS &nFirst, R3_AXIS &nSecond)
{
	//cyclic, so that (first, second, axis) is right-handed and an angle about the axis runs from
	//the first towards the second
	switch(nAxis){
	case R3_AXIS_X:	nFirst = R3_AXIS_Y; nSecond = R3_AXIS_Z; break;
	case R3_AXIS_Y:	nFirst = R3_AXIS_Z; nSecond = R3_AXIS_X; break;
	default:		nFirst = R3_AXIS_X; nSecond = R3_AXIS_Y; break;
	}
}

CR3Point R3PointFromAxes(R3_AXIS nAxis, double dAlong, double dFirst, double dSecond)
{
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_Y;
	R3CrossAxes(nAxis,n1,n2);
	double d[R3_AXIS_COUNT] = {0.0,0.0,0.0};
	d[nAxis] = dAlong;
	d[n1] = dFirst;
	d[n2] = dSecond;
	return CR3Point(d[R3_AXIS_X],d[R3_AXIS_Y],d[R3_AXIS_Z]);
}

} // namespace kin5ax
