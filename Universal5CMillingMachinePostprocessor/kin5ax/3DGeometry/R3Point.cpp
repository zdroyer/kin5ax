// Point.cpp: implementation of the CR3Point class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "R3Vector.h"
#include "R3Point.h"
#include "string_tools.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
template<class T>
T maxI(const T &a, const T &b)
{
	if(a>=b)
		return a;
	return b;
}

double d_NY(const CR3Point &p1,  const CR3Point &p2)
{
	return (fabs(p1.GetX()-p2.GetX())+fabs(p1.GetY()-p2.GetY())+fabs(p1.GetZ()-p2.GetZ()));
}

double d_E(const CR3Point &p1,  const CR3Point &p2)
{
	return (p1-p2).GetLength();
}

double d_ESq(const CR3Point &p1,  const CR3Point &p2)
{
	return (p1-p2).GetLengthSquare();
}

double d_MAX(const CR3Point &p1,  const CR3Point &p2)
{
	CR3Vector v = p1-p2;
	return maxI(maxI(v.GetX(),v.GetY()),v.GetZ());
}


CR3Point average(const CR3Point &p1, const CR3Point &p2)
{

	return CR3Point((p1.GetX()+p2.GetX())/2.0,(p1.GetY()+p2.GetY())/2.0,(p1.GetZ()+p2.GetZ())/2.0);
}
/*
CR3Point average1div3(const CR3Point &closer, const CR3Point &further)
{
	return closer+((1.0/3.0)*(further-closer));
}
//*/

CR3Point throwOver(const CR3Point &middle, const CR3Point &term)
{	
	return middle + (middle-term);
}

std::ostream & operator <<(std::ostream & out, const CR3Point &p)
{ 
	out<<p.toString();
	return out;
}


/*
POINT CR3Point::toPOINT() const
{
	POINT res;
	res.x=(int)x;
	res.y=(int)y;
	return res;
}
//*/

std::string CR3Point::toStringInt() const
{
	return string_printf("(%d, %d, %d)",(int)GetX(),(int)GetY(),(int)GetZ());
}


std::string CR3Point::toString() const
{
	return string_printf("(%.4f, %.4f, %.4f)",GetX(),GetY(),GetZ());
}


CR3Vector CR3Point::toR3Vector() const
{
	return CR3Vector(GetX(),GetY(),GetZ());
}
