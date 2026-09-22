#include "stdafx.h"
#include "PostprocessorSettings.h"
#include "double_tools.h"

namespace kin5ax {

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

const double CPostprocessorSettings::dMAX_INTERPOLATION_STEPS = 1E6;

double CPostprocessorSettings::InterpolationSteps(double dDistance, double dAngleRadians) const
{
	double dSteps = 1.0;
	if(m_dMaxPositionDifference>0.0)
		dSteps = std::max(dSteps,ceil(fabs(dDistance)/m_dMaxPositionDifference));
	if(m_dMaxOrientationDifferenceRadians>0.0)
		dSteps = std::max(dSteps,ceil(fabs(dAngleRadians)/m_dMaxOrientationDifferenceRadians));
	//a huge count (a tiny limit, an infinite ratio) would only exhaust the memory
	if(!(dSteps<=dMAX_INTERPOLATION_STEPS))
		dSteps = dMAX_INTERPOLATION_STEPS;
	return dSteps;
}

} // namespace kin5ax
