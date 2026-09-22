#include "StdAfx.h"
#include "gltools.h"



extern CR3Vector r3vOZ;

CGLTools::CGLTools(void)

{

}

CGLTools::~CGLTools(void)
{
}



void CGLTools::DrawAxes(double d,double dAxesLength)
{
	bool l = false;
	bool t = false;
//	if(glIsEnabled(GL_LIGHTING)){
//		glDisable(GL_LIGHTING);
//		l=true;
//	}

	if(glIsEnabled(GL_TEXTURE_2D)){
		glDisable(GL_TEXTURE_2D);
		t=true;
	}

	dAxesLength/=2.0;
	CR3Point vers(dAxesLength,0.0,0.0);
	Arrow(vers*-1.0,vers,d,0.0,0.0);

	vers.SetXYZ(0.0,dAxesLength,0.0);
	Arrow(vers*-1.0,vers,0.0,d,0.0);

	vers.SetXYZ(0.0,0.0,dAxesLength);
	Arrow(vers*-1.0,vers,0.0,0.0,d);
//	if(l)
//		glEnable(GL_LIGHTING);
	if(t)
		glEnable(GL_TEXTURE_2D);

}

void CGLTools::Arrow(const CR3CaughtVector &cv, double r, double g, double b)
{
	Arrow(cv.m_p,cv.m_p+cv.m_v,r,g,b);
}

void CGLTools::LineArrow(const CR3Point &p, const CR3Vector &v)
{
	double d = v.GetLength();
	if(is_zero(d))
		return;
	double r1 = 0.01*d;
	double d1 = 0.96*d;
	CR3Vector axis = r3vOZ.vectorProduct(v);
	double dAngle = r3vOZ.UndirectedAngle(v);

	glPushMatrix();
	glTranslated(p.GetX(),p.GetY(),p.GetZ());
	glRotated(radians_to_degrees(dAngle),axis.GetX(),axis.GetY(),axis.GetZ());
	glBegin(GL_LINES);
		glVertex3d(0.0,0.0,0.0);
		glVertex3d(0.0,0.0,d);
		glVertex3d(0.0,0.0,d);
		glVertex3d(0.0,r1,d1);
		glVertex3d(0.0,0.0,d);
		glVertex3d(0.0,-r1,d1);
		glVertex3d(0.0,0.0,d);
		glVertex3d(r1,0.0,d1);
		glVertex3d(0.0,0.0,d);
		glVertex3d(-r1,0.0,d1);
	glEnd();
	glPopMatrix();
	return;
	glRotated(radians_to_degrees(-dAngle),axis.GetX(),axis.GetY(),axis.GetZ());
	glTranslated(-p.GetX(),-p.GetY(),-p.GetZ());
	return;
}


void CGLTools::Arrow(const CR3Point &tail,const CR3Point &head, double dR, double dG, double dB)
{

	CR3Vector v = (head-tail);
	double d = v.GetLength();
	if(is_zero(d))
		return;
	double r1 = 0.005*d;
	double d1 = 0.04*d;
	v/=d;
	
	//the arrow is drawn along OZ and turned to v; for v = -OZ the axis of the turn is any horizontal one
	CR3Vector axis = r3vOZ.vectorProduct(v);
	double dAngle = r3vOZ.UndirectedAngle(v);
	if(axis.IsZero())
		axis = CR3Vector(1.0,0.0,0.0);
	glPushMatrix();
	glTranslated(tail.GetX(),tail.GetY(),tail.GetZ());
	glRotated(radians_to_degrees(dAngle),axis.GetX(),axis.GetY(),axis.GetZ());
	glColor3d(dR,dG,dB);
	static GLUquadricObj * pQ = gluNewQuadric();
	gluCylinder(pQ,r1,r1,d-d1,10,10);
	glTranslated(0.0,0.0,d-d1);
	gluCylinder(pQ,2.0*r1,0.0,d1,10,3);
	glPopMatrix();
	return;
	glTranslated(0.0,0.0,-d+d1);
	glRotated(radians_to_degrees(-dAngle),axis.GetX(),axis.GetY(),axis.GetZ());
	glTranslated(-tail.GetX(),-tail.GetY(),-tail.GetZ());
	return;

	/*
	CR3Vector v = (tail-head)* 0.04;
	double dv=v.GetLength();
	CR3Point h1 = head + v;
	CR3Vector vVers = v.GetUnitVector();

	CR3Vector t = h1-CR3Point(0.0);
	CR3Vector axis =CR3Vector(0.0,0.0,1).vectorProduct(vVers*(-1.0));
	CR3Point p1(0.0, dv/2.0, 0.0);
	CR3Point p2(dv/2.0, 0.0, 0.0);

	if(axis.GetLength()){
		CRotation rotate(axis,atan2(axis.GetLength(),-vVers.GetZ()));
		p1 = rotate * p1;
		p2 = rotate * p2;
	}
	p1 = p1 + t;
	p2 = p2 + t;
	glBegin(GL_LINES);
	glColor3d(r,g,b);
		glVertex3dv(head.toGL3dv());
		glVertex3dv(tail.toGL3dv());

		glVertex3dv(head.toGL3dv());
		glVertex3dv(p1.toGL3dv());

		glVertex3dv(head.toGL3dv());
		glVertex3dv(throwOver(h1,p1).toGL3dv());

		glVertex3dv(head.toGL3dv());
		glVertex3dv(p2.toGL3dv());

		glVertex3dv(head.toGL3dv());
		glVertex3dv(throwOver(h1,p2).toGL3dv());
	glEnd();
	//*/
}
