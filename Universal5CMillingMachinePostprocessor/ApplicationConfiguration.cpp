#include "StdAfx.h"
//#include ".\applicationconfiguration.h"
#include "double_tools.h"



CApplicationConfiguration::CApplicationConfiguration(void)
:m_nAlpha1Method(ALPHA1_AXIS_MULTIPLICATION)
,m_nAlpha2Method(ALPHA2_AXIS_MULTIPLICATION)
,m_nAnglesSolvingMethod(ANGLES_SOLVING_ALPHA1_THEN_ALPHA2)
,m_bPerspectiveProjection(false)
//,m_dModelSize(2.0)
,m_dUnit(1.0)
,m_bFixedMaterialFrame(false)
,m_bShowMaterial(false)
,m_bShowMaterialFrame(false)
,m_bShowMaterialContour(true)
,m_bShowMaterialHolder(true)
,m_bShowTool(true)
,m_bShowToolFrame(false)
,m_bShowToolTrajectory(true)
,m_bShowLinksFrames(false)
,m_bShowLinks(true)
,m_bShowBase(false)
,m_bShowParts(true)
,m_bShowBaseFrame(true)
,m_dInputPrecision(dEPSILON)
,m_dMaxOrientationDifferenceDegrees(1.0)
,m_dMaxPositionDifference(1E-2)
,m_dAngleDistanceCoefficientRadians(1.0)
,m_dDisplacementCoefficient(10.0)
,m_nRefreshPeriod(20)
,m_bStopSimulation(false)
,m_nSimulationInterval(0)
,m_bShowToolBoundingBox(false)
{
	m_dMaxOrientationDifferenceRadians = degrees_to_radians(m_dMaxOrientationDifferenceDegrees);
}

CApplicationConfiguration::~CApplicationConfiguration(void)
{
}


