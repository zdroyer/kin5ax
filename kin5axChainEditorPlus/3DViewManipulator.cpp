#include "StdAfx.h"
#include ".\3dviewmanipulator.h"
#include "rotation.h"
#include "translation.h"
#include "scale.h"

extern CApplicationConfiguration oConfiguration;
namespace kin5ax { extern CR3Vector r3vOZ; }

C3DViewManipulator::C3DViewManipulator(void)
:m_m4x4InvertedInitialViewTransformation(1.0)
{
	//a sensible ball until the view is given its first size
	SetSphereRadius(200);
	m_dZoomBase=1.1;
	m_dZoomBaseInv=1.0/m_dZoomBase;
	m_nZoomPower=0;
	m_bLBD=false;
	m_bRBD=false;
	m_dTranslationCoefficient=2.0;
	
	m_r3vInitialProjectionTranslation.SetXYZ(0.0,0.0,-10.0);
	m_r3pEyePoint-=m_r3vInitialProjectionTranslation;
	//m_r3pEyePoint.SetZ(10.);
	
}

C3DViewManipulator::~C3DViewManipulator(void)
{
}

CPoint C3DViewManipulator::InnerCoordinates(const CPoint &point)
{
	return CPoint(point.x-(m_windowSize.cx/2), (m_windowSize.cy/2)-point.y);
}


void C3DViewManipulator::OnLButtonDown(UINT nFlags, const CPoint &point)
{
	if(m_bRBD)
		return;
	m_lastLBD = point;
	m_bLBD=true;
}
void C3DViewManipulator::OnRButtonDown(UINT nFlags, const CPoint &point)
{
	if(m_bLBD)
		return;
	m_lastRBD = point;
	CalculateRadialVector(m_r3vFirst,point);
	//the handle is drawn where the ball is held, from the moment it is taken hold of
	m_r3vSecond = m_r3vFirst;
	m_bRBD=true;

}
void C3DViewManipulator::OnLButtonUp(UINT nFlags, const CPoint &point)
{
	m_bLBD=false;
}
void C3DViewManipulator::OnRButtonUp(UINT nFlags, const CPoint &point)
{
	m_bRBD=false;
}

void C3DViewManipulator::OnSize(UINT nType, int cx, int cy)
{
	double  dNear = 1.0;
	if ( 0 < cx && 0 < cy ){
		//m_windowSize.SetSize(cx,cy);
		m_windowSize = CSize(cx,cy);
	}
	else
		return;
	SetSphereRadiusFromWindow();
	UpdateProjectionMatrix();
}
void C3DViewManipulator::UpdateProjectionMatrix()
{
	double dAspectRatio=double(m_windowSize.cx)/double(m_windowSize.cy);
	glViewport(0,0,m_windowSize.cx,m_windowSize.cy);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	double d = 4.0;//(dAspectRatio > 1.0 ? 2.0*dAspectRatio : 2.0);
	if(oConfiguration.m_bPerspectiveProjection){
		glFrustum(-dAspectRatio,dAspectRatio,-1.0,1.0,5.0,105.0);
		glTranslated(0.0,0.0,-5.0-d/2.0);
		m_dTranslationCoefficient = 1.0 + d*0.1;
		//gluPerspective(15.0,dAspectRatio,dNear, 20.0+dNear);
		//glTranslated(m_r3vInitialProjectionTranslation.GetX()
		//,m_r3vInitialProjectionTranslation.GetY()
		//,m_r3vInitialProjectionTranslation.GetZ());
	} else {
		glOrtho(-dAspectRatio*1.4,dAspectRatio*1.4,-1.4,1.4,-50.0,50.0);
		m_dTranslationCoefficient = 1.4;
	}
	glMatrixMode(GL_MODELVIEW);
}
bool C3DViewManipulator::OnMouseMove(UINT nFlags, const CPoint &point)
{
	UpdateState(nFlags,point);
	if(m_bLBD){
		if(m_lastLBD==point)
			return false;

		static CR3Vector r3vT;
		CalculateTranslationVector(r3vT,(InnerCoordinates(point)-InnerCoordinates(m_lastLBD)));
		if(TranslateGLScene(r3vT)){
			m_lastLBD=point;
			return true;
		} 

	}
	if(m_bRBD){
		if(m_lastRBD==point)
			return false;
		//m_lastLBD=point;
		CalculateRadialVector(m_r3vSecond,point);
		if(RotateGLScene(m_r3vFirst,m_r3vSecond)){
			m_r3vFirst = m_r3vSecond;
			return true;
		}
			
	}

	return false;
}

void C3DViewManipulator::SetSphereRadius(int nPixelCount)
{
	if(nPixelCount>0)
		m_nRadius = nPixelCount;
	m_nRadiusSquare = m_nRadius*m_nRadius;
}

void C3DViewManipulator::SetSphereRadiusFromWindow()
{
	//the circle spans nine tenths of the shorter side, so its radius is 45 per cent of it; the
	//rotation reads that same radius, and so a drag from the middle of the view out to the circle
	//is a quarter of a turn whatever the size of the window
	const int nShorter = (m_windowSize.cx<m_windowSize.cy) ? m_windowSize.cx : m_windowSize.cy;
	SetSphereRadius((9*nShorter)/20);
}

void C3DViewManipulator::CalculateRadialVector(CR3Vector & r3vV, const CPoint & point)
{
	CPoint p = InnerCoordinates(point);
	int nR2 = p.x*p.x + p.y*p.y;
	int nZ2 = m_nRadiusSquare - nR2;
	if(nZ2>=0){
		r3vV.SetXYZ(p.x,p.y,sqrt((double)nZ2));
	} else {
		double d = sqrt((double)m_nRadiusSquare/(double)nR2);
		r3vV.SetXYZ(d*(double)p.x,d*(double)p.y,0.0);
	}
}

void C3DViewManipulator::UpdateState(UINT nFlags, const CPoint & point)
{
	if( nFlags&MK_LBUTTON ){
		if( !m_bLBD )
			OnLButtonDown(nFlags,point);
	} else {
		if( m_bLBD )
			OnLButtonUp(nFlags,point);
	}

	if( nFlags&MK_RBUTTON ){
		if( !m_bRBD )
			OnRButtonDown(nFlags,point);
	} else {
		if( m_bRBD )
			OnRButtonUp(nFlags,point);
	}
}

bool C3DViewManipulator::RotateGLScene(const CR3Vector & r3vFirst, const CR3Vector & r3vTransformated)
{

	CR3Vector axis = r3vFirst.vectorProduct(r3vTransformated);
	double dSin = axis.GetLength();
	double dCos = r3vFirst.scalarProduct(r3vTransformated);
	double dAngle = atan2(dSin,dCos);
	static double d2Pi = 2.0*acos(-1.0);
	if(dAngle<0.0)
		dAngle += d2Pi;
    if(is_zero(dSin))
		axis.SetZ(-1.0);
	static GLdouble m[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,m);
	CTransformation trGL(m);
	CRotation rot(axis,dAngle);
	glLoadIdentity();
	glMultMatrixd((rot*trGL).toGLMatrix());
	m_m4x4InvertedInitialViewTransformation = m_m4x4InvertedInitialViewTransformation * rot.Inverted();
	return true;
}

//A ring of line segments about a point of the view, in the pixel coordinates set up below.
static void DrawCircle(double dRadius, double dX=0.0, double dY=0.0)
{
	static const double d2Pi = 4.0*acos(0.0);
	const int nSides = 72;
	glBegin(GL_LINE_LOOP);
	for(int i=0;i<nSides;i++){
		const double dA = d2Pi*(double)i/(double)nSides;
		glVertex2d(dX+dRadius*cos(dA),dY+dRadius*sin(dA));
	}
	glEnd();
}

void C3DViewManipulator::DrawIndicators()
{
	if(!m_bLBD && !m_bRBD)
		return;

	//the indicators belong to the picture rather than over it: OpenGL draws them into the back
	//buffer before it is swapped. Drawn with GDI onto the window, as they once were, they lived
	//only until the next frame was swapped over them, which is the flash one sees instead
	glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LINE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glColor3d(1.0,1.0,1.0);
	glLineWidth(1.0f);

	//pixels, the origin in the middle of the view and OY upwards: what InnerCoordinates and the
	//radial vector of the trackball are already in
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-0.5*(double)m_windowSize.cx,0.5*(double)m_windowSize.cx,
			-0.5*(double)m_windowSize.cy,0.5*(double)m_windowSize.cy,-1.0,1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	const double dR = (double)m_nRadius;
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1,(GLushort)0x3333);
	if(m_bRBD)
		DrawCircle(dR);
	if(m_bLBD){
		glBegin(GL_LINES);
		glVertex2d(-dR,0.0);
		glVertex2d(dR,0.0);
		glVertex2d(0.0,-dR);
		glVertex2d(0.0,dR);
		glEnd();
	}
	glDisable(GL_LINE_STIPPLE);
	//and where the ball is held now
	if(m_bRBD)
		DrawCircle(3.0,m_r3vSecond.GetX(),m_r3vSecond.GetY());

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}

bool C3DViewManipulator::IsInAction()const
{
	return m_bLBD|| m_bRBD;
}
bool C3DViewManipulator::TranslateGLScene(const CR3Vector & r3vV)
{
	if(r3vV.IsZero())
		return false;
	static GLdouble m[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,m);
	CTransformation trGL(m);
	CTranslation t(r3vV);
	//glTranslated(r3vV.GetX(),r3vV.GetY(),r3vV.GetZ());
	glLoadIdentity();
	glMultMatrixd((t*trGL).toGLMatrix());
	m_m4x4InvertedInitialViewTransformation = m_m4x4InvertedInitialViewTransformation * t.Inverted();
	return true;
}

void C3DViewManipulator::CalculateTranslationVector(CR3Vector & r3vV, const CSize & r2vV)
{
	r3vV.SetXYZ(m_dTranslationCoefficient*(double)(2*r2vV.cx)/(double)m_windowSize.cy,
				m_dTranslationCoefficient*(double)(2*r2vV.cy)/(double)m_windowSize.cy,
				0.0);
}

void C3DViewManipulator::ResetView(void)
{
	glLoadIdentity();
	m_m4x4InvertedInitialViewTransformation.MakeDiag(1.0,1.0,1.0,1.0);
}

void C3DViewManipulator::ResetPosition(void)
{
	static GLdouble m[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,m);
	m[12]=m[13]=m[14]=0.0;
	m_m4x4InvertedInitialViewTransformation.ZeroTranslation();
	glLoadMatrixd(m);
}

void C3DViewManipulator::ResetOrientation(void)
{
	static GLdouble m[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,m);
	m[0]=m[5]=m[10]=pow(m_dZoomBase,(double)(m_nZoomPower));
	m[1]=m[2]=m[4]=m[6]=m[8]=m[9]=0.0;
	m_m4x4InvertedInitialViewTransformation.ZeroRotation(pow(m_dZoomBase,(double)(-m_nZoomPower)));
	glLoadMatrixd(m);
}

void C3DViewManipulator::ResetScale(void)
{
	static GLdouble m[16];
	double d1DivScale = pow(m_dZoomBase,(double)(-m_nZoomPower));
	glScaled(d1DivScale,d1DivScale,d1DivScale);

	double dScale = pow(m_dZoomBase,(double)(m_nZoomPower));
	CTransformation sM1(1.0);
	sM1.ZeroRotation(dScale);
	m_m4x4InvertedInitialViewTransformation =  sM1 * m_m4x4InvertedInitialViewTransformation;
	m_nZoomPower=0;
}



void C3DViewManipulator::ZoomIn(void)
{


	static GLdouble m[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,m);
	CTransformation tr(m);
	tr = CScale(m_dZoomBase)*tr;
	glLoadMatrixd(tr.toGLMatrix());
	m_m4x4InvertedInitialViewTransformation = m_m4x4InvertedInitialViewTransformation*CScale(m_dZoomBaseInv); 
	//	glScaled(m_dZoomBase,m_dZoomBase,m_dZoomBase);
	//	m_m4x4InvertedInitialViewTransformation = CScale(m_dZoomBaseInv)*m_m4x4InvertedInitialViewTransformation; 

	m_nZoomPower++;
}

void C3DViewManipulator::ZoomOut(void)
{
	static GLdouble m[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,m);
	CTransformation tr(m);
	tr = CScale(m_dZoomBaseInv)*tr;
	glLoadMatrixd(tr.toGLMatrix());
	m_m4x4InvertedInitialViewTransformation = m_m4x4InvertedInitialViewTransformation*CScale(m_dZoomBase); 

//	glScaled(m_dZoomBaseInv,m_dZoomBaseInv,m_dZoomBaseInv);
//	m_m4x4InvertedInitialViewTransformation = CScale(m_dZoomBase)*m_m4x4InvertedInitialViewTransformation; 

	m_nZoomPower--;
}

void C3DViewManipulator::SetOptimalScale(double dDimension) 
{
	

	if(is_zero( dDimension))
		return;
	ResetScale();
	double d = 2.0;
	if(dDimension<d){
		for(int i=0;i<1000;i++){
			dDimension*=m_dZoomBase;
			if(dDimension<d)
				ZoomIn();
			else
				break;
		}
	} else {
		for(int i=0;i<1000;i++){
			dDimension*=m_dZoomBaseInv;
			if(dDimension>d)
				ZoomOut();
			else
				break;
		}
	}
}
CR3Vector C3DViewManipulator::GetObserverVector(const CTransformation &invertedMaterialTransformation) const
{
	if(oConfiguration.m_bFixedMaterialFrame)
		return (m_m4x4InvertedInitialViewTransformation * r3vOZ);
	return  invertedMaterialTransformation * (m_m4x4InvertedInitialViewTransformation * r3vOZ);

}


double C3DViewManipulator::GetPixelSize(double dZ) const
{
	double d = (1.0/(double)m_windowSize.cy)*(2.0*m_dTranslationCoefficient);
	if(oConfiguration.m_bPerspectiveProjection){
		d = d*(7.0-dZ)/5.0;
	} 
	return d;
}