#include "../stdafx.h"
#include "SphereCLDataGenerator.h"
#include "R3Vector.h"
#include "R3Point.h"
#include "Rotation.h"
#include "string_tools.h"

namespace kin5ax {

namespace {

//one cutter location, the point as the decimal separator whatever the locale (six decimals, as %lf had)
void WriteLocation(FILE *pFile, const CR3Point &p, const CR3Vector &v)
{
	std::string s = "GOTO/"+fixed_to_string(p.GetX(),6)+","+fixed_to_string(p.GetY(),6)+","+fixed_to_string(p.GetZ(),6)
		+","+fixed_to_string(v.GetX(),6)+","+fixed_to_string(v.GetY(),6)+","+fixed_to_string(v.GetZ(),6)+"\n";
	fputs(s.c_str(),pFile);
}

} // namespace

size_t WriteSphereCLData(const std::string &sFileName, double dR, double dD)
{
	FILE * pFile = fopen(sFileName.c_str(),"w");
	if(pFile==NULL)
		return 0;

	size_t nLocations = 0;
	CRotation rZ,rX;
	double dPiDiv2 = acos(0.0);
	double dAlpha = (2.0*asin(dD/(2.0*dR)));
	size_t nStacks = (size_t)fabs(ceil(dPiDiv2/dAlpha));
	for(size_t i=0;i<=nStacks;i++){
		double dAX = dPiDiv2*((double)i/(double)nStacks);
		rX = CRotation(CR3Vector(1.0,0.0,0.0),dAX);
		double dR1 = dR * cos(dAX);
		if(is_zero(dR1)){
			WriteLocation(pFile,CR3Point(0.0,0.0,dR),CR3Vector(0.0,0.0,1.0));
			nLocations++;
			break;
		}
		double dBeta = (2.0*asin(dD/(2.0*dR1)));
		size_t nSlices = (size_t)fabs(ceil((4.0*dPiDiv2)/dBeta));
		for(size_t j=0; j<=nSlices; j++){
			rZ = CRotation::OZRotation(4.0*dPiDiv2*(double)j/(double)nSlices);
			CR3Point p = rZ*rX*CR3Point(0.0,dR,0.0);
			CR3Vector v = (p-CR3Point()).GetUnitVector();
			WriteLocation(pFile,p,v);
			nLocations++;
		}
	}
	fclose(pFile);
	return nLocations;
}

size_t WriteMeridianCLData(const std::string &sFileName, double dR, size_t nPoints)
{
	if(nPoints<3 || nPoints>9 || nPoints%2==0)
		return 0;
	FILE * pFile = fopen(sFileName.c_str(),"w");
	if(pFile==NULL)
		return 0;
	double dPiDiv2 = acos(0.0);
	size_t nMiddle = nPoints/2;
	for(size_t i=0;i<nPoints;i++){
		//angle from the pole: -90 deg ... 0 (the middle point) ... +90 deg
		double dAngle = dPiDiv2*((double)i-(double)nMiddle)/(double)nMiddle;
		CR3Vector v(sin(dAngle),0.0,cos(dAngle));
		if(i==nMiddle)
			v = CR3Vector(0.0,0.0,1.0);	//exactly the pole
		WriteLocation(pFile,CR3Point(dR*v.GetX(),dR*v.GetY(),dR*v.GetZ()),v);
	}
	fclose(pFile);
	return nPoints;
}

} // namespace kin5ax
