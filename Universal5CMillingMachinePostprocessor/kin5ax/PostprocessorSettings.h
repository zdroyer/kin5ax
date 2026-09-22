#pragma once

namespace kin5ax {

enum ALPHA1_METHOD{
	ALPHA1_AXIS_MULTIPLICATION=0,
	ALPHA1_CONIC_METHOD=1
};
enum ALPHA2_METHOD{
	ALPHA2_AXIS_MULTIPLICATION=0,
	ALPHA2_SQUARE_EQUATION=1
};

enum ANGLES_SOLVING_METHOD{
	ANGLES_SOLVING_INDEPENDENTLY=0,
	ANGLES_SOLVING_ALPHA1_THEN_ALPHA2=1,
	ANGLES_SOLVING_ALPHA2_THEN_ALPHA1=2,
};

//Settings of the inverse kinematics solver and of the CL-data postprocessing
//(no GUI/view settings here: those stay in CApplicationConfiguration)
class CPostprocessorSettings
{
public:
	CPostprocessorSettings(void);
	~CPostprocessorSettings(void);

	ALPHA1_METHOD m_nAlpha1Method;
	ALPHA2_METHOD m_nAlpha2Method;
	ANGLES_SOLVING_METHOD m_nAnglesSolvingMethod;

	//linear interpolation limits between two cutter locations; a limit of zero (or less) switches
	//the interpolation by that criterion off
	double m_dMaxPositionDifference;
	double m_dMaxOrientationDifferenceDegrees;
	double m_dMaxOrientationDifferenceRadians;
	//the count of steps (1 = no intermediate location) into which a move of dDistance and dAngleRadians is divided
	//so that no step exceeds the limits above; never below 1 and never above nMAX_INTERPOLATION_STEPS
	double InterpolationSteps(double dDistance, double dAngleRadians) const;
	static const double dMAX_INTERPOLATION_STEPS;

	//weights of CChainConfiguration::Distance
	double m_dAngleDistanceCoefficientRadians;
	double m_dDisplacementCoefficient;

	std::string GetComputationMethod(const std::string &sEndLine="\n") const
	{
		std::string sResult;
		switch (m_nAnglesSolvingMethod){
			case ANGLES_SOLVING_INDEPENDENTLY:
				sResult+="Independent angles computation "+sEndLine;
				if(m_nAlpha1Method == ALPHA1_AXIS_MULTIPLICATION)
					sResult+=("\tAlpha1 method - axis multiplication"+sEndLine);
				if(m_nAlpha1Method == ALPHA1_CONIC_METHOD)
					sResult+=("\tAlpha1 method - conic method"+sEndLine);
				if(m_nAlpha2Method == ALPHA2_AXIS_MULTIPLICATION)
					sResult+=("\tAlpha2 method - axis multiplication"+sEndLine);
				if(m_nAlpha2Method == ALPHA2_SQUARE_EQUATION)
					sResult+=("\tAlpha2 method - quadric equation"+sEndLine);
				break;
			case ANGLES_SOLVING_ALPHA1_THEN_ALPHA2:
				sResult+="Alpha1 then alpha2 computation"+sEndLine;
				if(m_nAlpha1Method == ALPHA1_AXIS_MULTIPLICATION)
					sResult+=("\tAlpha1 method - axis multiplication"+sEndLine);
				if(m_nAlpha1Method == ALPHA1_CONIC_METHOD)
					sResult+=("\tAlpha1 method - conic method"+sEndLine);
				sResult+=("\tAlpha2 method - using alpha1 "+sEndLine);
				break;
			case ANGLES_SOLVING_ALPHA2_THEN_ALPHA1:
				sResult+="Alpha2 then alpha1 computation "+sEndLine;
				sResult+=("\tAlpha1 method - using alpha2 "+sEndLine);
				if(m_nAlpha2Method == ALPHA2_AXIS_MULTIPLICATION)
					sResult+=("\tAlpha2 method - axis multiplication"+sEndLine);
				if(m_nAlpha2Method == ALPHA2_SQUARE_EQUATION)
					sResult+=("\tAlpha2 method - quadric equation (quaternion revolute)"+sEndLine);
				break;
		}
		return sResult;
	}
};

} // namespace kin5ax
