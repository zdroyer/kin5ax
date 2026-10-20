#include "StdAfx.h"
#include "SolidRendererGL.h"
#include "R3BoxRendererGL.h"

#include <cmath>
#include <vector>

namespace {

const int nAROUND = 48;			//segments of a whole turn; a wedge takes its share of them
const int nALONG = 16;			//rings of a sphere from one cut to the other

//the steps of the sector, in radians, from the first angle to the last
std::vector<double> SectorSteps(double dFromDeg, double dToDeg)
{
	std::vector<double> v;
	const double dSweep = dToDeg-dFromDeg;
	int nSteps = (int)ceil(nAROUND*dSweep/360.0);
	if(nSteps<2)
		nSteps = 2;
	for(int i=0;i<=nSteps;i++)
		v.push_back(degrees_to_radians(dFromDeg+dSweep*i/nSteps));
	return v;
}

} // namespace

void CSolidRendererGL::RenderSolid(const CSolid &oSolid)
{
	switch(oSolid.GetType()){
	case SOLID_CYLINDER:	RenderCylinder(oSolid.GetCylinder(),false); break;
	case SOLID_SPHERE:		RenderSphere(oSolid.GetSphere(),false); break;
	default:				CR3BoxRendererGL::RenderSolid(oSolid.GetBox()); break;
	}
}

void CSolidRendererGL::RenderWire(const CSolid &oSolid, double dR, double dG, double dB)
{
	switch(oSolid.GetType()){
	case SOLID_CYLINDER:
		glColor3d(dR,dG,dB);
		RenderCylinder(oSolid.GetCylinder(),true);
		break;
	case SOLID_SPHERE:
		glColor3d(dR,dG,dB);
		RenderSphere(oSolid.GetSphere(),true);
		break;
	default:
		CR3BoxRendererGL::RenderWire(oSolid.GetBox(),dR,dG,dB);
		break;
	}
}

void CSolidRendererGL::ApplyCanonicalFrame(const CR3CylinderSegment &oCylinder)
{
	//the library knows the frame; OpenGL wants it as a matrix, and CTransformation hands one over
	CTransformation tr = oCylinder.GetCanonicalTransformation();
	glMultMatrixd(tr.toGLMatrix());
}

void CSolidRendererGL::RenderCylinder(const CR3CylinderSegment &oCylinder, bool bWire)
{
	if(oCylinder.IsEmpty())
		return;
	const double dRadius = oCylinder.GetRadius();
	const double dLength = oCylinder.GetLength();
	const std::vector<double> vAngles = SectorSteps(oCylinder.GetMinAngleDegrees()
		,oCylinder.GetMaxAngleDegrees());
	glPushMatrix();
	ApplyCanonicalFrame(oCylinder);
	if(bWire){
		glDisable(GL_LIGHTING);
		//the two rims, and a line up each end of the sector
		for(int nEnd=0;nEnd<2;nEnd++){
			const double dZ = nEnd ? dLength : 0.0;
			glBegin(oCylinder.IsWholeTurn() ? GL_LINE_LOOP : GL_LINE_STRIP);
			if(!oCylinder.IsWholeTurn())
				glVertex3d(0.0,0.0,dZ);			//the apex of the wedge
			for(size_t i=0;i<vAngles.size();i++)
				glVertex3d(dRadius*cos(vAngles[i]),dRadius*sin(vAngles[i]),dZ);
			glEnd();
		}
		glBegin(GL_LINES);
		for(size_t i=0;i<vAngles.size();i+=4){
			glVertex3d(dRadius*cos(vAngles[i]),dRadius*sin(vAngles[i]),0.0);
			glVertex3d(dRadius*cos(vAngles[i]),dRadius*sin(vAngles[i]),dLength);
		}
		glEnd();
		glPopMatrix();
		return;
	}
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	//the side
	glBegin(GL_QUAD_STRIP);
	for(size_t i=0;i<vAngles.size();i++){
		const double dC = cos(vAngles[i]), dS = sin(vAngles[i]);
		glNormal3d(dC,dS,0.0);
		glVertex3d(dRadius*dC,dRadius*dS,0.0);
		glVertex3d(dRadius*dC,dRadius*dS,dLength);
	}
	glEnd();
	//the two ends, as fans from the axis
	for(int nEnd=0;nEnd<2;nEnd++){
		const double dZ = nEnd ? dLength : 0.0;
		glBegin(GL_TRIANGLE_FAN);
		glNormal3d(0.0,0.0,nEnd ? 1.0 : -1.0);
		glVertex3d(0.0,0.0,dZ);
		for(size_t i=0;i<vAngles.size();i++){
			const size_t k = nEnd ? i : vAngles.size()-1-i;		//wound outwards on both ends
			glVertex3d(dRadius*cos(vAngles[k]),dRadius*sin(vAngles[k]),dZ);
		}
		glEnd();
	}
	//and the two flanks of a wedge, which a whole cylinder does not have
	if(!oCylinder.IsWholeTurn()){
		for(int nSide=0;nSide<2;nSide++){
			const double dA = nSide ? vAngles.back() : vAngles.front();
			const double dC = cos(dA), dS = sin(dA);
			glBegin(GL_QUADS);
			glNormal3d(nSide ? -dS : dS,nSide ? dC : -dC,0.0);
			glVertex3d(0.0,0.0,0.0);
			glVertex3d(dRadius*dC,dRadius*dS,0.0);
			glVertex3d(dRadius*dC,dRadius*dS,dLength);
			glVertex3d(0.0,0.0,dLength);
			glEnd();
		}
	}
	glPopMatrix();
}

void CSolidRendererGL::RenderSphere(const CR3SphereSegment &oSphere, bool bWire)
{
	if(oSphere.IsEmpty())
		return;
	const double dRadius = oSphere.GetRadius();
	//the drawing is done with the axis of the cut as Z, so the piece is a stack of rings
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_Y;
	R3CrossAxes(oSphere.GetAxis(),n1,n2);
	const double dMin = oSphere.GetMin(), dMax = oSphere.GetMax();
	const CR3Point pCentre = oSphere.GetCentre();

	//a point of the piece: at the height dT along the axis and the angle dA about it
	struct CLocal
	{
		static CR3Point Point(const CR3Point &pCentre, R3_AXIS nAxis, double dT, double dA
			,double dR)
		{
			const CR3Point p = R3PointFromAxes(nAxis,dT,dR*cos(dA),dR*sin(dA));
			return CR3Point(pCentre.GetX()+p.GetX(),pCentre.GetY()+p.GetY(),pCentre.GetZ()+p.GetZ());
		}
	};

	if(bWire)
		glDisable(GL_LIGHTING);
	else {
		glEnable(GL_LIGHTING);
		glShadeModel(GL_SMOOTH);
	}
	for(int i=0;i<nALONG;i++){
		const double dT1 = dMin+(dMax-dMin)*i/nALONG;
		const double dT2 = dMin+(dMax-dMin)*(i+1)/nALONG;
		const double dR1 = sqrt(dRadius*dRadius-dT1*dT1>0.0 ? dRadius*dRadius-dT1*dT1 : 0.0);
		const double dR2 = sqrt(dRadius*dRadius-dT2*dT2>0.0 ? dRadius*dRadius-dT2*dT2 : 0.0);
		if(bWire && i%4)
			continue;								//a few rings are enough for a wire frame
		glBegin(bWire ? GL_LINE_LOOP : GL_QUAD_STRIP);
		for(int k=0;k<=nAROUND;k++){
			const double dA = d2Pi*k/nAROUND;
			if(bWire){
				const CR3Point p = CLocal::Point(pCentre,oSphere.GetAxis(),dT1,dA,dR1);
				glVertex3d(p.GetX(),p.GetY(),p.GetZ());
				continue;
			}
			const CR3Point p1 = CLocal::Point(pCentre,oSphere.GetAxis(),dT1,dA,dR1);
			const CR3Point p2 = CLocal::Point(pCentre,oSphere.GetAxis(),dT2,dA,dR2);
			//the outward normal of a sphere is the radius itself
			glNormal3d((p1.GetX()-pCentre.GetX())/dRadius,(p1.GetY()-pCentre.GetY())/dRadius
				,(p1.GetZ()-pCentre.GetZ())/dRadius);
			glVertex3d(p1.GetX(),p1.GetY(),p1.GetZ());
			glNormal3d((p2.GetX()-pCentre.GetX())/dRadius,(p2.GetY()-pCentre.GetY())/dRadius
				,(p2.GetZ()-pCentre.GetZ())/dRadius);
			glVertex3d(p2.GetX(),p2.GetY(),p2.GetZ());
		}
		glEnd();
	}
	//the flat faces a cut leaves
	if(!oSphere.IsWholeSphere() && !bWire){
		for(int nEnd=0;nEnd<2;nEnd++){
			const double dT = nEnd ? dMax : dMin;
			const double dR = sqrt(dRadius*dRadius-dT*dT>0.0 ? dRadius*dRadius-dT*dT : 0.0);
			if(dR<=0.0)
				continue;							//the cut is at the pole: nothing flat there
			glBegin(GL_TRIANGLE_FAN);
			const CR3Vector vN = R3AxisDirection(oSphere.GetAxis());
			glNormal3d(nEnd ? vN.GetX() : -vN.GetX(),nEnd ? vN.GetY() : -vN.GetY()
				,nEnd ? vN.GetZ() : -vN.GetZ());
			const CR3Point pMiddle = CLocal::Point(pCentre,oSphere.GetAxis(),dT,0.0,0.0);
			glVertex3d(pMiddle.GetX(),pMiddle.GetY(),pMiddle.GetZ());
			for(int k=0;k<=nAROUND;k++){
				const double dA = d2Pi*(nEnd ? k : nAROUND-k)/nAROUND;
				const CR3Point p = CLocal::Point(pCentre,oSphere.GetAxis(),dT,dA,dR);
				glVertex3d(p.GetX(),p.GetY(),p.GetZ());
			}
			glEnd();
		}
	}
}
