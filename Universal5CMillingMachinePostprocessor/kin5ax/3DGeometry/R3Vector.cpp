#pragma once

#include "../stdafx.h"
#include "R3Vector.h"




namespace kin5ax {

const CR3Vector &CR3Vector::operator+=(const CR3Vector &v){
	m_dX+=v.m_dX;
	m_dY+=v.m_dY;
	m_dZ+=v.m_dZ;
	return (*this);
}


CR3Vector CR3Vector::operator+(const CR3Vector &v) const
	{				
		
		double rx=m_dX+v.m_dX;
		double ry=m_dY+v.m_dY;
		double rz=m_dZ+v.m_dZ;
		return CR3Vector(rx,ry,rz);
		
	}
CR3Vector CR3Vector::operator-(const CR3Vector &v) const
	{				
		
		double rx=m_dX-v.m_dX;
		double ry=m_dY-v.m_dY;
		double rz=m_dZ-v.m_dZ;
		return CR3Vector(rx,ry,rz);
		
	}


double CR3Vector::operator *(const CR3Vector &v) const
{
	return v.m_dX*m_dX+v.m_dY*m_dY+v.m_dZ*m_dZ;
}



CR3Vector operator *(double d, const CR3Vector &v)
{
	return v*d;
}




int CR3Vector::operator==(const CR3Vector &v) const
{	

	return (are_equal(m_dX,v.m_dX)&&
			are_equal(m_dY,v.m_dY)&&
			are_equal(m_dZ,v.m_dZ)); 
}

} // namespace kin5ax
