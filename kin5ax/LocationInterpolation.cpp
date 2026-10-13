#include "stdafx.h"
#include "LocationInterpolation.h"
#include <cmath>

namespace kin5ax {

namespace {

//a unit vector perpendicular to the unit vector u: u x (the coordinate axis u is least aligned with)
static CR3Vector PerpendicularUnitVector(const CR3Vector &u)
{
	double dX = fabs(u.GetX()), dY = fabs(u.GetY()), dZ = fabs(u.GetZ());
	CR3Vector axis = (dX<=dY && dX<=dZ) ? CR3Vector(1.0,0.0,0.0) : (dY<=dZ ? CR3Vector(0.0,1.0,0.0) : CR3Vector(0.0,0.0,1.0));
	return u.vectorProduct(axis).GetUnitVector();
}

} // namespace

void InterpolateLocation
	( const CR3Point &p1
	, const CR3Vector &v1
	, const CR3Point &p2
	, const CR3Vector &v2
	, const CPostprocessorSettings &oSettings
	, std::vector<std::pair<double,CR3AnchoredVector> > &result)
{
	result.erase(result.begin(),result.end());
	if(!oSettings.m_bInterpolateBetweenLocations){
		//nothing between two locations of the file: the second one is the whole step
		result.push_back(std::pair<double,CR3AnchoredVector>(1.0,CR3AnchoredVector(v2,p2)));
		return;
	}
	if(p1==p2 && v1==v2){
		result.push_back(std::pair<double,CR3AnchoredVector>(1.0,CR3AnchoredVector(v2,p2)));
		return;
	}
	double dInv = oSettings.InterpolationSteps(d_E(p1,p2),v1.UndirectedUnitVectorsAngle(v2));
	if(dInv<=1.0){
		result.push_back(std::pair<double,CR3AnchoredVector>(1.0,CR3AnchoredVector(v2,p2)));
		return;
	}
	//the axis turns at a constant rate from u1 towards u2 in the plane of both: u(a) = u1 cos a + w sin a,
	//w the unit vector perpendicular to u1 in that plane (any perpendicular one when the axes are antiparallel,
	//KnownBugs #42: the normalised linear blend was the zero vector half way, and not uniform in between)
	CR3Vector u1 = v1.GetUnitVector(), u2 = v2.GetUnitVector();
	double dAngle = u1.UndirectedUnitVectorsAngle(u2);
	CR3Vector w = u2 - u1*(u1*u2);
	w = w.IsZero(1E-9) ? PerpendicularUnitVector(u1) : w.GetUnitVector();
	double t;
	result.reserve((size_t)fabs(dInv));
	for(double i=1.0;i<dInv-0.001; i++){
		t = i/dInv;
		CR3Vector u = u1*cos(t*dAngle) + w*sin(t*dAngle);
		result.push_back(std::pair<double,CR3AnchoredVector>(t,CR3AnchoredVector(u,(p2*t)+(p1*(1.0-t)))));
	}
	result.push_back(std::pair<double,CR3AnchoredVector>(1.0,CR3AnchoredVector(v2,p2)));
}

std::vector<CDenseLocation> DensifyLocations
	( const std::vector<CR3AnchoredVector> &vLocations
	, const CPostprocessorSettings &oSettings)
{
	std::vector<CDenseLocation> vDense;
	if(vLocations.empty())
		return vDense;
	CDenseLocation oFirst = {vLocations[0],0,false};
	vDense.push_back(oFirst);
	std::vector<std::pair<double,CR3AnchoredVector> > vBetween;
	for(size_t i=1;i<vLocations.size();i++){
		InterpolateLocation(vLocations[i-1].m_p,vLocations[i-1].m_v,vLocations[i].m_p,vLocations[i].m_v
			,oSettings,vBetween);
		for(size_t j=0;j<vBetween.size();j++){
			const bool bArrived = (j+1==vBetween.size());
			CDenseLocation o = {vBetween[j].second,bArrived ? i : i-1,!bArrived};
			vDense.push_back(o);
		}
	}
	return vDense;
}

} // namespace kin5ax
