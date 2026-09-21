#include "../stdafx.h"
#include "SphereCLDataGenerator.h"
#include "r3vector.h"
#include "r3point.h"
#include "rotation.h"

size_t WriteSphereCLData(const string &sFileName, double dR, double dD)
{
	FILE * pFile = fopen(sFileName.c_str(),"w");
	if(pFile==NULL)
		return 0;

	size_t nLocations = 0;
	CRotation rZ,rX;
	double dPiDiv2 = acos(0.0);
	double dAlpha = (2.0*asin(dD/(2.0*dR)));
	size_t nStacks = abs((int)ceil(dPiDiv2/dAlpha));
	for(size_t i=0;i<=nStacks;i++){
		double dAX = dPiDiv2*((double)i/(double)nStacks);
		rX = CRotation(CR3Vector(1.0,0.0,0.0),dAX);
		double dR1 = dR * cos(dAX);
		if(is_zero(dR1)){
			fprintf(pFile,"GOTO/%lf,%lf,%lf,%lf,%lf,%lf\n",0.0,0.0,dR,0.0,0.0,1.0);
			nLocations++;
			break;
		}
		double dBeta = (2.0*asin(dD/(2.0*dR1)));
		size_t nSlices = abs((int)ceil((4.0*dPiDiv2)/dBeta));
		for(size_t j=0; j<=nSlices; j++){
			rZ = CRotation::OZRotation(4.0*dPiDiv2*(double)j/(double)nSlices);
			CR3Point p = rZ*rX*CR3Point(0.0,dR,0.0);
			CR3Vector v = (p-CR3Point()).GetVersor();
			fprintf(pFile,"GOTO/%lf,%lf,%lf,%lf,%lf,%lf\n",p.GetX(),p.GetY(),p.GetZ(),v.GetX(),v.GetY(),v.GetZ());
			nLocations++;
		}
	}
	fclose(pFile);
	return nLocations;
}
