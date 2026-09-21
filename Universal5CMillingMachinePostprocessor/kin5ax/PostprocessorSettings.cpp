#include "stdafx.h"
#include "PostprocessorSettings.h"
#include "double_tools.h"

CPostprocessorSettings::CPostprocessorSettings(void)
:m_nAlpha1Method(ALPHA1_AXIS_MULTIPLICATION)
,m_nAlpha2Method(ALPHA2_AXIS_MULTIPLICATION)
,m_nAnglesSolvingMethod(ANGLES_SOLVING_ALPHA1_THEN_ALPHA2)
,m_dMaxOrientationDifferenceDegrees(1.0)
,m_dMaxPositionDifference(1E-2)
,m_dAngleDistanceCoefficientRadians(1.0)
,m_dDisplacementCoefficient(10.0)
{
	m_dMaxOrientationDifferenceRadians = degrees_to_radians(m_dMaxOrientationDifferenceDegrees);
}

CPostprocessorSettings::~CPostprocessorSettings(void)
{
}
