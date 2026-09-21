#include "../stdafx.h"
#include "r3box.h"
#include "r3vector.h"

#include "../string_tools.h"
#include "transformation.h"


extern CR3Vector r3vOZ;

void CR3Box::ExpandXYZ(double dX, double dY, double dZ)
{
	m_dMinX-=fabs(dX);
	m_dMaxX+=fabs(dX);
	m_dMinY-=fabs(dY);
	m_dMaxY+=fabs(dY);
	m_dMinZ-=fabs(dZ);
	m_dMaxZ+=fabs(dZ);
}

CR3Box CR3Box::operator + (const CR3Box &b2) const
{
	return CR3Box(Min(m_dMinX,b2.m_dMinX)
		,Max(m_dMaxX,b2.m_dMaxX)
		,Min(m_dMinY,b2.m_dMinY)
		,Max(m_dMaxY,b2.m_dMaxY)
		,Min(m_dMinZ,b2.m_dMinZ)
		,Max(m_dMaxZ,b2.m_dMaxZ));
}

CR3Box::CR3Box(const CR3Point &r3pO, double dRadius)
{
	dRadius = fabs(dRadius);
	m_dMinX=r3pO.GetX()-dRadius;
	m_dMaxX=r3pO.GetX()+dRadius;
	m_dMinY=r3pO.GetY()-dRadius;
	m_dMaxY=r3pO.GetY()+dRadius;
	m_dMinZ=r3pO.GetZ()-dRadius;
	m_dMaxZ=r3pO.GetZ()+dRadius;
}

string CR3Box::toXML() const
{
	string sResult;
	sResult+="<box ";
	sResult+="x1=\""+double_to_string(m_dMinX)+"\" ";
	sResult+="x2=\""+double_to_string(m_dMaxX)+"\" ";
	sResult+="y1=\""+double_to_string(m_dMinY)+"\" ";
	sResult+="y2=\""+double_to_string(m_dMaxY)+"\" ";
	sResult+="z1=\""+double_to_string(m_dMinZ)+"\" ";
	sResult+="z2=\""+double_to_string(m_dMaxZ)+"\" ";
	sResult+=" />\n";
	return sResult;
}


CR3Box CR3Box::OuterBoxFromOZCylinder(double dRadius, double dZ1, double dZ2)
{
	return CR3Box(-dRadius,dRadius,-dRadius,dRadius,dZ1,dZ2);
}
CR3Box CR3Box::InnerBoxFromOZCylinder(double dRadius, double dZ1, double dZ2)
{
	double dA = dRadius*sqrt(2.0);
	return CR3Box(-dA,dA,-dA,dA,dZ1,dZ2);
}


CR3Box CR3Box::BoxFromOZBox(double dXSize, double dYSize, double dZ)
{
	dXSize/=2.0;
	dYSize/=2.0;
	return CR3Box(-dXSize,dXSize,-dYSize,dYSize,0.0,dZ);

}
CR3Box CR3Box::BoxFromCylinder(const CR3Point &p1, const CR3Point &p2,double dR )
{

	CR3Vector v = p2-p1;
	if(v.IsZero())
		return CR3Box(p1,dR);
	CR3Box box(p1,p2);	
	double dCosAlpha = v.GetVersor().scalarProduct(r3vOZ);
	if(is_zero(dCosAlpha)){
		box.ExpandXYZ(0.0,0.0,dR);
	} else {
		if(are_equal(fabs(dCosAlpha),1.0))
			box.ExpandXYZ(dR,dR,0.0);
		else{
			double dZ = fabs(dR * sin(acos(dCosAlpha)));
			double dXY = dR * dCosAlpha;
			double dY = sqrt(max(0.0,(v.GetX()*v.GetX()*dR*dR-v.GetX()*v.GetX()*dZ*dZ)/(v.GetX()*v.GetX()+v.GetY()*v.GetY())));
			double dX =dR;
			if(is_not_zero(v.GetX())){
				dX = v.GetY()*dY/v.GetX();
			}
			box.ExpandXYZ(dX,dY,dZ);
		}				
	}
	return box;
}
bool CR3Box::IsEmpty() const
{	
	return is_zero(m_dMaxX)
		&& is_zero(m_dMaxY)
		&& is_zero(m_dMaxZ)
		&& is_zero(m_dMinX)
		&& is_zero(m_dMinY)
		&& is_zero(m_dMinZ);
}
bool CR3Box::Contains(const CR3Point &p)const
{
	if(IsEmpty())
		return false;
	if(!is_between_or_equal(p.GetX(),m_dMinX,m_dMaxX))
		return false;
	if(!is_between_or_equal(p.GetY(),m_dMinY,m_dMaxY))
		return false;
	if(!is_between_or_equal(p.GetZ(),m_dMinZ,m_dMaxZ))
		return false;
	return true;
}

bool CR3Box::HasIntersectionWithSphere(const CR3Point &p, const double &dR) const
{
	if(IsEmpty())
		return false;
	if(Contains(p))
		return true;
	CR3Box box((*this));
	box.ExpandXYZ(dR,dR,dR);
	if(box.Contains(p)){
		box.SetBox((*this));
		box.ExpandXYZ(dR,0.0,0.0);
		if(box.Contains(p))
			return true;
		box.SetBox((*this));
		box.ExpandXYZ(0.0,dR,0.0);
		if(box.Contains(p))
			return true;
		box.SetBox((*this));
		box.ExpandXYZ(0.0,0.0,dR);
		if(box.Contains(p))
			return true;
		double r2 = dR*dR;
		if(p.DistanceSquareTo(m_dMaxX,m_dMaxY,m_dMaxZ)<r2)
			return true;
		if(p.DistanceSquareTo(m_dMinX,m_dMaxY,m_dMaxZ)<r2)
			return true;
		if(p.DistanceSquareTo(m_dMinX,m_dMinY,m_dMaxZ)<r2)
			return true;
		if(p.DistanceSquareTo(m_dMaxX,m_dMinY,m_dMaxZ)<r2)
			return true;
		if(p.DistanceSquareTo(m_dMaxX,m_dMaxY,m_dMinZ)<r2)
			return true;
		if(p.DistanceSquareTo(m_dMinX,m_dMaxY,m_dMinZ)<r2)
			return true;
		if(p.DistanceSquareTo(m_dMinX,m_dMinY,m_dMinZ)<r2)
			return true;
		if(p.DistanceSquareTo(m_dMaxX,m_dMinY,m_dMinZ)<r2)
			return true;
		if(is_between_or_equal(p.GetZ(),m_dMinZ,m_dMaxZ)){
			if( p.XYDistanceSquareTo(m_dMinX,m_dMinY)<r2
			  ||p.XYDistanceSquareTo(m_dMinX,m_dMaxY)<r2
			  ||p.XYDistanceSquareTo(m_dMaxX,m_dMaxY)<r2
			  ||p.XYDistanceSquareTo(m_dMaxX,m_dMinY)<r2)
				return true;
		}
		if(is_between_or_equal(p.GetX(),m_dMinX,m_dMaxX)){
			if( p.YZDistanceSquareTo(m_dMinY,m_dMinZ)<r2
			  ||p.YZDistanceSquareTo(m_dMinY,m_dMaxZ)<r2
			  ||p.YZDistanceSquareTo(m_dMaxY,m_dMaxZ)<r2
			  ||p.YZDistanceSquareTo(m_dMaxY,m_dMinZ)<r2)
				return true;
		}
		if(is_between_or_equal(p.GetY(),m_dMinY,m_dMaxY)){
			if( p.XZDistanceSquareTo(m_dMinX,m_dMinZ)<r2
			  ||p.XZDistanceSquareTo(m_dMinX,m_dMaxZ)<r2
			  ||p.XZDistanceSquareTo(m_dMaxX,m_dMaxZ)<r2
			  ||p.XZDistanceSquareTo(m_dMaxX,m_dMinZ)<r2)
				return true;
		}

	} 
	return false;
}
bool CR3Box::HasIntersectionWithLineSegment(const CR3Point &p1, const CR3Point &p2) const
{
	if(IsEmpty())
		return false;
	if(this->Contains(p1) || this->Contains(p2))
		return true;
	if(p1==p2)
		return false;
	if(p1.GetX()>m_dMaxX+dEPSILON && p2.GetX()>m_dMaxX+dEPSILON)
		return false;
	if(p1.GetX()<m_dMinX-dEPSILON && p2.GetX()<m_dMinX-dEPSILON)
		return false;
	if(p1.GetY()>m_dMaxY+dEPSILON && p2.GetY()>m_dMaxY+dEPSILON)
		return false;
	if(p1.GetY()<m_dMinY-dEPSILON && p2.GetY()<m_dMinY-dEPSILON)
		return false;
	if(p1.GetZ()>m_dMaxZ+dEPSILON && p2.GetZ()>m_dMaxZ+dEPSILON)
		return false;
	if(p1.GetZ()<m_dMinZ-dEPSILON && p2.GetZ()<m_dMinZ-dEPSILON)
		return false;
	CR3Point pp1(p1), pp2(p2);
//	CR3Vector v = (p2-p1).GetVersor();
	
	double dTMin,dTMax;
	//x plane
	if(are_not_equal(pp1.GetX(),pp2.GetX())){
		dTMin = (m_dMinX-pp1.GetX())/(pp2.GetX()-pp1.GetX());		
		dTMax = (m_dMaxX-pp1.GetX())/(pp2.GetX()-pp1.GetX());		
		CR3Point pTmp = pp1;
		if(is_in_0_1(dTMin))
			pp1.SetXYZ(m_dMinX
						,pTmp.GetY()*(1.0-dTMin)+pp2.GetY()*dTMin
						,pTmp.GetZ()*(1.0-dTMin)+pp2.GetZ()*dTMin);
		if(is_in_0_1(dTMax))
			pp2.SetXYZ(m_dMaxX
						,pTmp.GetY()*(1.0-dTMax)+pp2.GetY()*dTMax
						,pTmp.GetZ()*(1.0-dTMax)+pp2.GetZ()*dTMax);
	}
	if(are_not_equal(pp1.GetY(),pp2.GetY())){
		dTMin = (m_dMinY-pp1.GetY())/(pp2.GetY()-pp1.GetY());		
		dTMax = (m_dMaxY-pp1.GetY())/(pp2.GetY()-pp1.GetY());		
		CR3Point pTmp = pp1;
		if(is_in_0_1(dTMin))
			pp1.SetXYZ(pTmp.GetX()*(1.0-dTMin)+pp2.GetX()*dTMin
						,m_dMinY
						,pTmp.GetZ()*(1.0-dTMin)+pp2.GetZ()*dTMin);
		if(is_in_0_1(dTMax))
			pp2.SetXYZ(pTmp.GetX()*(1.0-dTMax)+pp2.GetX()*dTMax
						,m_dMaxY
						,pTmp.GetZ()*(1.0-dTMax)+pp2.GetZ()*dTMax);
	}
	if(are_not_equal(pp1.GetZ(),pp2.GetZ())){
		dTMin = (m_dMinZ-pp1.GetZ())/(pp2.GetZ()-pp1.GetZ());		
		dTMax = (m_dMaxZ-pp1.GetZ())/(pp2.GetZ()-pp1.GetZ());		
		CR3Point pTmp = pp1;
		if(is_in_0_1(dTMin))
			pp1.SetXYZ(pTmp.GetX()*(1.0-dTMin)+pp2.GetX()*dTMin
						,pTmp.GetY()*(1.0-dTMin)+pp2.GetY()*dTMin
						,m_dMinZ);
		if(is_in_0_1(dTMax))
			pp2.SetXYZ(pTmp.GetX()*(1.0-dTMax)+pp2.GetX()*dTMax
						,pTmp.GetY()*(1.0-dTMax)+pp2.GetY()*dTMax
						,m_dMaxZ);
	}
	if(this->Contains(pp1)||this->Contains(pp2)||this->Contains(average(pp1,pp2)))
		return true;
	return false;
}

bool CR3Box::HasIntersectionWithBox(const CR3Box &box, const CTransformation &tr,const CTransformation &tr_inv) const
{
/*
	glColor3d(0.7,0.7,0.0);
	CGLRendererR3Box::RenderSolid(*this);
	glColor3d(1.0,1.0,0.0);
	glPushMatrix();
	glMultMatrixd(CTransformation(tr).toGLMatrix());
	CGLRendererR3Box::RenderWire(box);
	glPopMatrix();
	glPushMatrix();
	glColor3d(0.0,0.7,0.7);
	CGLRendererR3Box::RenderSolid(box);
	glColor3d(1.0,1.0,0.0);
	glPushMatrix();
	glMultMatrixd(CTransformation(tr_inv).toGLMatrix());
	CGLRendererR3Box::RenderWire(*this);
	glPopMatrix();
//*/
	if(IsEmpty() || box.IsEmpty())
		return false;
	return AsimetricIntersection(box,tr) || box.AsimetricIntersection((*this),tr_inv);

}
bool CR3Box::AsimetricIntersection(const CR3Box &box, const CTransformation &tr) const
{
	vector<CR3Point> vV;
	vV.reserve(8);
	vV.push_back(CR3Point(box.m_dMaxX,box.m_dMaxY,box.m_dMaxZ));
	vV.push_back(CR3Point(box.m_dMinX,box.m_dMaxY,box.m_dMaxZ));
	vV.push_back(CR3Point(box.m_dMinX,box.m_dMinY,box.m_dMaxZ));
	vV.push_back(CR3Point(box.m_dMaxX,box.m_dMinY,box.m_dMaxZ));

	vV.push_back(CR3Point(box.m_dMaxX,box.m_dMaxY,box.m_dMinZ));
	vV.push_back(CR3Point(box.m_dMinX,box.m_dMaxY,box.m_dMinZ));
	vV.push_back(CR3Point(box.m_dMinX,box.m_dMinY,box.m_dMinZ));
	vV.push_back(CR3Point(box.m_dMaxX,box.m_dMinY,box.m_dMinZ));
	
	for(size_t i=0; i<vV.size();i++)
		vV[i] = tr*vV[i];

	if(HasIntersectionWithLineSegment(vV[0],vV[1]))
		return true;
	if(HasIntersectionWithLineSegment(vV[2],vV[1]))
		return true;
	if(HasIntersectionWithLineSegment(vV[2],vV[3]))
		return true;
	if(HasIntersectionWithLineSegment(vV[1],vV[3]))
		return true;

	if(HasIntersectionWithLineSegment(vV[4],vV[5]))
		return true;
	if(HasIntersectionWithLineSegment(vV[6],vV[5]))
		return true;
	if(HasIntersectionWithLineSegment(vV[6],vV[7]))
		return true;
	if(HasIntersectionWithLineSegment(vV[4],vV[7]))
		return true;

	if(HasIntersectionWithLineSegment(vV[4],vV[0]))
		return true;
	if(HasIntersectionWithLineSegment(vV[1],vV[5]))
		return true;
	if(HasIntersectionWithLineSegment(vV[6],vV[2]))
		return true;
	if(HasIntersectionWithLineSegment(vV[3],vV[7]))
		return true;

	return false;
}


bool CR3Box::HasIntersectionWithOZCylinder(const double &dZ,const double &dR, const CTransformation &trToBox,const CTransformation &trToCylinder) const
{
	/*
	glColor3d(1.0,1.0,0.0);
	GLUquadricObj *pQ = gluNewQuadric();
	gluCylinder(pQ,dR,dR,dZ,20,20);
	glPushMatrix();
	glMultMatrixd(CTransformation(trToCylinder).toGLMatrix());
	CGLRendererR3Box::RenderWire(*this);
	glPopMatrix();
	//*/

	if(!this->HasIntersectionWithBox(CR3Box::OuterBoxFromOZCylinder(dR,0.0,dZ),trToBox, trToCylinder))
		return false;
	if(this->HasIntersectionWithBox(CR3Box::InnerBoxFromOZCylinder(dR,0.0,dZ),trToBox, trToCylinder))
		return true;
	vector<CR3Point> vV;
	vV.reserve(8);
	vV.push_back(CR3Point(m_dMaxX,m_dMaxY,m_dMaxZ));
	vV.push_back(CR3Point(m_dMinX,m_dMaxY,m_dMaxZ));
	vV.push_back(CR3Point(m_dMinX,m_dMinY,m_dMaxZ));
	vV.push_back(CR3Point(m_dMaxX,m_dMinY,m_dMaxZ));

	vV.push_back(CR3Point(m_dMaxX,m_dMaxY,m_dMinZ));
	vV.push_back(CR3Point(m_dMinX,m_dMaxY,m_dMinZ));
	vV.push_back(CR3Point(m_dMinX,m_dMinY,m_dMinZ));
	vV.push_back(CR3Point(m_dMaxX,m_dMinY,m_dMinZ));
	
	for(size_t i=0; i<vV.size();i++)
		vV[i] = trToCylinder*vV[i];

	if(LineSegmentAndOZCylinderIntersection(vV[0],vV[1], dZ, dR))
		return true;
	if(LineSegmentAndOZCylinderIntersection(vV[2],vV[1], dZ, dR))
		return true;
	if(LineSegmentAndOZCylinderIntersection(vV[2],vV[3], dZ, dR))
		return true;
	if(LineSegmentAndOZCylinderIntersection(vV[1],vV[3], dZ, dR))
		return true;

	if(LineSegmentAndOZCylinderIntersection(vV[4],vV[5], dZ, dR))
		return true;
	if(LineSegmentAndOZCylinderIntersection(vV[6],vV[5], dZ, dR))
		return true;
	if(LineSegmentAndOZCylinderIntersection(vV[6],vV[7], dZ, dR))
		return true;
	if(LineSegmentAndOZCylinderIntersection(vV[4],vV[7], dZ, dR))
		return true;

	if(LineSegmentAndOZCylinderIntersection(vV[4],vV[0], dZ, dR))
		return true;
	if(LineSegmentAndOZCylinderIntersection(vV[1],vV[5], dZ, dR))
		return true;
	if(LineSegmentAndOZCylinderIntersection(vV[6],vV[2], dZ, dR))
		return true;
	if(LineSegmentAndOZCylinderIntersection(vV[3],vV[7], dZ, dR))
		return true;
	return false;
}

bool CR3Box::LineSegmentAndOZCylinderIntersection(const CR3Point &p1, const CR3Point &p2, const double &dZ, const double &dR) const
{
	double dR2 = dR*dR;
	if(is_between_or_equal(p1.GetZ(),0.0,dZ))
		if(p1.XYDistanceSquareTo(0.0,0.0)<dR2+dEPSILON)
			return true;
	if(is_between_or_equal(p2.GetZ(),0.0,dZ))
		if(p2.XYDistanceSquareTo(0.0,0.0)<dR2+dEPSILON)
			return true;

	CR3Vector v = p2-p1;

	double dA = v.GetXYLengthSquare();
	if(is_zero(dA)){
		if(p1.XYDistanceSquareTo(0.0,0.0)>dR2+dEPSILON)
			return false;
		if( is_between_or_equal(p1.GetZ(),0.0,dZ)
			|| is_between_or_equal(p2.GetZ(),0.0,dZ))
			return true;
		return false;
	}
	double dB = 2.0*(v.GetX()*p1.GetX()+v.GetY()+p1.GetY());
	double dC = p1.XYDistanceSquareTo(0.0,0.0)-dR2;
	double dDelta = dB*dB - 4.0*dA*dC;
	if(dDelta<-dEPSILON)
		return false;
	if(dDelta<0)
		dDelta=0;
	dDelta = sqrt(dDelta);
	double dT1= (-dB-dDelta)/2.0*dA;
	
	double dZ1 = p1.GetZ()+dT1*v.GetZ();
	if(is_in_0_1(dT1)){
		if(is_between_or_equal(dZ1,0.0,dZ))
			return true;
	}
	double dT2= (-dB+dDelta)/2.0*dA;
	double dZ2 = p1.GetZ()+dT2*v.GetZ();
	if(is_in_0_1(dT2)){
		if(is_between_or_equal(dZ2,0.0,dZ))
			return true;
	}
	if(dT1>1.0+dEPSILON && dT2>1.0+dEPSILON )
		return false;
	if(dT1<-dEPSILON && dT2<-dEPSILON )
		return false;
	if(dZ1*dZ2<dEPSILON)
		return true;
	return false;
}
