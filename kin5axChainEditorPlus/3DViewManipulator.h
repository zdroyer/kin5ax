#pragma once


#include "r3vector.h"
#include "transformation.h"

class C3DViewManipulator
{
protected:
	bool m_bLBD;
	bool m_bRBD;
	CR3Vector m_r3vFirst;
	CR3Vector m_r3vSecond;
	CPoint m_lastLBD,m_lastRBD;
	CSize m_windowSize;
	//the trackball: the radius drawn on the screen and the radius a drag is turned into a rotation
	//with are one and the same, so that the ball answers the mouse where it is seen
	int m_nRadius, m_nRadiusSquare;
	CPoint InnerCoordinates(const CPoint &point);
	int m_nZoomPower;
	double m_dZoomBase, m_dZoomBaseInv;
	double m_dTranslationCoefficient;

	CTransformation m_m4x4InvertedInitialViewTransformation;
	CR3Vector m_r3vInitialProjectionTranslation;
	CR3Point m_r3pEyePoint;
public:
	void SetOptimalScale(double dDimension) ;
	double GetPixelSize(double dZ=0) const;
	void UpdateProjectionMatrix();
	CR3Vector GetObserverVector(const CTransformation &invertedMaterialTransformation) const;
	C3DViewManipulator(void);
	~C3DViewManipulator(void);
	void OnLButtonDown(UINT nFlags, const CPoint &point);
	void OnLButtonUp(UINT nFlags, const CPoint &point);
	void OnRButtonDown(UINT nFlags, const CPoint &point);
	void OnRButtonUp(UINT nFlags, const CPoint &point);
	void OnSize(UINT nType, int cx, int cy);
	bool OnMouseMove(UINT nFlags, const CPoint &point);
	/// The circle of the trackball, or the cross of the panning, into the back buffer.
	void DrawIndicators();
	bool IsInAction()const;
protected:
	//the ball fills nine tenths of the shorter side of the view
	void SetSphereRadiusFromWindow();
	void SetSphereRadius(int nPixelCount);
	void CalculateRadialVector(CR3Vector & r3vV, const CPoint & point);
	void UpdateState(UINT nFlags, const CPoint & point);
	bool RotateGLScene(const CR3Vector & r3vFirst, const CR3Vector & r3vTransformated);
	bool TranslateGLScene(const CR3Vector & r3vV);
	void CalculateTranslationVector(CR3Vector & r3vV, const CSize &r2vV);

public:
	void ResetView(void);
	void ResetPosition(void);
	void ResetOrientation(void);
	void ResetScale(void);
	void ZoomIn(void);
	void ZoomOut(void);
};
