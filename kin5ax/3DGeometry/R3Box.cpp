#include "../stdafx.h"
#include "R3Box.h"
#include "R3Vector.h"

#include "string_tools.h"
#include "Transformation.h"


namespace kin5ax {

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

std::string CR3Box::toXML() const
{
	std::string sResult;
	sResult+="<box ";
	sResult+="x1=\""+double_to_string_exact(m_dMinX)+"\" ";
	sResult+="x2=\""+double_to_string_exact(m_dMaxX)+"\" ";
	sResult+="y1=\""+double_to_string_exact(m_dMinY)+"\" ";
	sResult+="y2=\""+double_to_string_exact(m_dMaxY)+"\" ";
	sResult+="z1=\""+double_to_string_exact(m_dMinZ)+"\" ";
	sResult+="z2=\""+double_to_string_exact(m_dMaxZ)+"\" ";
	sResult+=" />\n";
	return sResult;
}


CR3Box CR3Box::OuterBoxFromOZCylinder(double dRadius, double dZ1, double dZ2)
{
	return CR3Box(-dRadius,dRadius,-dRadius,dRadius,dZ1,dZ2);
}
CR3Box CR3Box::InnerBoxFromOZCylinder(double dRadius, double dZ1, double dZ2)
{
	double dA = dRadius/sqrt(2.0);	//half side of the square inscribed in the circle
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
	double dCosAlpha = v.GetUnitVector().scalarProduct(r3vOZ);
	if(is_zero(dCosAlpha)){
		box.ExpandXYZ(0.0,0.0,dR);
	} else {
		if(are_equal(fabs(dCosAlpha),1.0))
			box.ExpandXYZ(dR,dR,0.0);
		else{
			double dZ = fabs(dR * sin(acos(dCosAlpha)));
			double dY = sqrt(std::max(0.0,(v.GetX()*v.GetX()*dR*dR-v.GetX()*v.GetX()*dZ*dZ)/(v.GetX()*v.GetX()+v.GetY()*v.GetY())));
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
//the part of the segment p1+t*(p2-p1), t in [dT1,dT2], that lies in the slab dMin<=coordinate<=dMax;
//false when nothing of it is left. d1, d2 - the coordinate at p1 and p2.
static bool clip_to_slab(double d1, double d2, double dMin, double dMax, double &dT1, double &dT2)
{
	double dD = d2-d1;
	if(is_zero(dD))
		return is_between_or_equal(d1,dMin,dMax);
	double dTNear = (dMin-d1)/dD, dTFar = (dMax-d1)/dD;
	if(dTNear>dTFar)
		std::swap(dTNear,dTFar);
	if(dTNear>dT1) dT1 = dTNear;
	if(dTFar<dT2) dT2 = dTFar;
	return dT1<=dT2+dEPSILON;
}

bool CR3Box::HasIntersectionWithLineSegment(const CR3Point &p1, const CR3Point &p2) const
{
	if(IsEmpty())
		return false;
	if(this->Contains(p1) || this->Contains(p2))
		return true;
	if(p1==p2)
		return false;
	//the segment is cut by the three slabs of the box (Liang-Barsky); a slab in which the segment
	//is parallel just tests the coordinate. What is left, if anything, is the part inside the box.
	double dT1=0.0, dT2=1.0;
	return clip_to_slab(p1.GetX(),p2.GetX(),m_dMinX-dEPSILON,m_dMaxX+dEPSILON,dT1,dT2)
		&& clip_to_slab(p1.GetY(),p2.GetY(),m_dMinY-dEPSILON,m_dMaxY+dEPSILON,dT1,dT2)
		&& clip_to_slab(p1.GetZ(),p2.GetZ(),m_dMinZ-dEPSILON,m_dMaxZ+dEPSILON,dT1,dT2);
}

bool CR3Box::HasIntersectionWithBox(const CR3Box &box, const CTransformation &tr,const CTransformation &tr_inv) const
{
/*
	glColor3d(0.7,0.7,0.0);
	CR3BoxRendererGL::RenderSolid(*this);
	glColor3d(1.0,1.0,0.0);
	glPushMatrix();
	glMultMatrixd(CTransformation(tr).toGLMatrix());
	CR3BoxRendererGL::RenderWire(box);
	glPopMatrix();
	glPushMatrix();
	glColor3d(0.0,0.7,0.7);
	CR3BoxRendererGL::RenderSolid(box);
	glColor3d(1.0,1.0,0.0);
	glPushMatrix();
	glMultMatrixd(CTransformation(tr_inv).toGLMatrix());
	CR3BoxRendererGL::RenderWire(*this);
	glPopMatrix();
//*/
	if(IsEmpty() || box.IsEmpty())
		return false;
	return AsymmetricIntersection(box,tr) || box.AsymmetricIntersection((*this),tr_inv);

}
//twice the signed area of the triangle a,b,c in the XY plane: positive when a->b->c turns left
static double cross_xy(const std::pair<double,double> &a, const std::pair<double,double> &b, const std::pair<double,double> &c)
{
	return (b.first-a.first)*(c.second-a.second)-(b.second-a.second)*(c.first-a.first);
}

static double distance_from_origin_to_segment_xy(const std::pair<double,double> &a, const std::pair<double,double> &b)
{
	double dDX = b.first-a.first, dDY = b.second-a.second;
	double dL2 = dDX*dDX+dDY*dDY;
	double dT = is_zero(dL2) ? 0.0 : -(a.first*dDX+a.second*dDY)/dL2;
	if(dT<0.0) dT=0.0;
	if(dT>1.0) dT=1.0;
	double dX = a.first+dT*dDX, dY = a.second+dT*dDY;
	return sqrt(dX*dX+dY*dY);
}

//the 8 vertices of a box, transformed by tr; the order matches vBOX_EDGES
static std::vector<CR3Point> box_vertices(const CR3Box &box, const CTransformation &tr)
{
	const CR3Point r3pMin = box.GetMinPoint(), r3pMax = box.GetMaxPoint();
	std::vector<CR3Point> vV;
	vV.reserve(8);
	vV.push_back(tr*CR3Point(r3pMax.GetX(),r3pMax.GetY(),r3pMax.GetZ()));
	vV.push_back(tr*CR3Point(r3pMin.GetX(),r3pMax.GetY(),r3pMax.GetZ()));
	vV.push_back(tr*CR3Point(r3pMin.GetX(),r3pMin.GetY(),r3pMax.GetZ()));
	vV.push_back(tr*CR3Point(r3pMax.GetX(),r3pMin.GetY(),r3pMax.GetZ()));
	vV.push_back(tr*CR3Point(r3pMax.GetX(),r3pMax.GetY(),r3pMin.GetZ()));
	vV.push_back(tr*CR3Point(r3pMin.GetX(),r3pMax.GetY(),r3pMin.GetZ()));
	vV.push_back(tr*CR3Point(r3pMin.GetX(),r3pMin.GetY(),r3pMin.GetZ()));
	vV.push_back(tr*CR3Point(r3pMax.GetX(),r3pMin.GetY(),r3pMin.GetZ()));
	return vV;
}

//the 12 edges of a box as pairs of indexes into box_vertices: the top face, the bottom face, the vertical edges
static const int vBOX_EDGES[12][2] = {{0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {0,4},{1,5},{2,6},{3,7}};

bool CR3Box::AsymmetricIntersection(const CR3Box &box, const CTransformation &tr) const
{
	std::vector<CR3Point> vV = box_vertices(box,tr);
	for(int i=0;i<12;i++)
		if(HasIntersectionWithLineSegment(vV[vBOX_EDGES[i][0]],vV[vBOX_EDGES[i][1]]))
			return true;
	return false;
}

//the distance from the origin to the convex hull of the points projected on the XY plane; 0 when the origin is inside
static double distance_from_origin_to_convex_hull_xy(const std::vector<CR3Point> &vPoints)
{
	std::vector<std::pair<double,double> > v;
	for(size_t i=0;i<vPoints.size();i++)
		v.push_back(std::make_pair(vPoints[i].GetX(),vPoints[i].GetY()));
	std::sort(v.begin(),v.end());
	v.erase(unique(v.begin(),v.end()),v.end());
	//the convex hull, counterclockwise (Andrew's monotone chain)
	size_t n = v.size();
	std::vector<std::pair<double,double> > vHull(2*n);
	size_t k=0;
	for(size_t i=0;i<n;i++){
		while(k>=2 && cross_xy(vHull[k-2],vHull[k-1],v[i])<=0.0) k--;
		vHull[k++] = v[i];
	}
	for(size_t i=n-1, t=k+1; i>0; i--){
		while(k>=t && cross_xy(vHull[k-2],vHull[k-1],v[i-1])<=0.0) k--;
		vHull[k++] = v[i-1];
	}
	if(n>1)
		k--;	//the first point is repeated at the end
	vHull.resize(k);
	if(k==1)
		return sqrt(vHull[0].first*vHull[0].first+vHull[0].second*vHull[0].second);
	//inside: the origin lies on the left of every edge; otherwise the nearest edge decides
	bool bInside = k>=3;
	double dMin = -1.0;
	for(size_t i=0;i<k;i++){
		const std::pair<double,double> &a = vHull[i], &b = vHull[(i+1)%k];
		if(cross_xy(a,b,std::make_pair(0.0,0.0))<0.0)
			bInside = false;
		double d = distance_from_origin_to_segment_xy(a,b);
		if(dMin<0.0 || d<dMin)
			dMin = d;
	}
	return bInside ? 0.0 : dMin;
}

bool CR3Box::HasIntersectionWithOZCylinder(const double &dZ,const double &dR, const CTransformation &trToBox,const CTransformation &trToCylinder) const
{
	if(IsEmpty())
		return false;
	//the quick answers: the box misses the box around the cylinder / hits the box inside it
	if(!this->HasIntersectionWithBox(CR3Box::OuterBoxFromOZCylinder(dR,0.0,dZ),trToBox, trToCylinder))
		return false;
	if(this->HasIntersectionWithBox(CR3Box::InnerBoxFromOZCylinder(dR,0.0,dZ),trToBox, trToCylinder))
		return true;
	//exact: the part of the box between the planes z=0 and z=dZ (a convex solid: the vertices inside the slab and
	//the points where the edges cross the planes) meets the cylinder iff its projection on the XY plane, a convex
	//polygon, comes closer than dR to the axis
	std::vector<CR3Point> vV = box_vertices(*this,trToCylinder);
	std::vector<CR3Point> vSlab;
	for(size_t i=0;i<vV.size();i++)
		if(is_between_or_equal(vV[i].GetZ(),0.0,dZ))
			vSlab.push_back(vV[i]);
	for(int i=0;i<12;i++){
		const CR3Point &a = vV[vBOX_EDGES[i][0]], &b = vV[vBOX_EDGES[i][1]];
		double dDZ = b.GetZ()-a.GetZ();
		if(is_zero(dDZ))
			continue;
		for(int j=0;j<2;j++){
			double dPlane = (j==0) ? 0.0 : dZ;
			double dT = (dPlane-a.GetZ())/dDZ;
			if(dT>0.0 && dT<1.0)
				vSlab.push_back(a + (b-a)*dT);
		}
	}
	if(vSlab.empty())
		return false;
	return distance_from_origin_to_convex_hull_xy(vSlab)<=dR+dEPSILON;
}

} // namespace kin5ax
