#pragma once


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

class CApplicationConfiguration
{
public:
	CApplicationConfiguration(void);
	~CApplicationConfiguration(void);

	ALPHA1_METHOD m_nAlpha1Method;
	ALPHA2_METHOD m_nAlpha2Method;
	ANGLES_SOLVING_METHOD m_nAnglesSolvingMethod;

	size_t m_nRefreshPeriod;

	bool m_bPerspectiveProjection;
	bool m_bFixedMaterialFrame;
	bool m_bShowMaterial;
	bool m_bShowMaterialFrame;
	bool m_bShowMaterialContour;
	bool m_bShowMaterialHolder;

	bool m_bShowTool;
	bool m_bShowToolFrame;
	bool m_bShowToolBoundingBox;
	bool m_bShowToolTrajectory;

	bool m_bShowLinksFrames;
	bool m_bShowLinks;
	bool m_bShowParts;

	bool m_bShowBase;
	bool m_bShowBaseFrame;

	bool m_bStopSimulation;

	double m_dInputPrecision;
	double m_dUnit;

	double m_dMaxPositionDifference;
	double m_dMaxOrientationDifferenceDegrees;
	double m_dMaxOrientationDifferenceRadians;

	DWORD m_nSimulationInterval;

	double m_dAngleDistanceCoefficientRadians;
	double m_dDisplacementCoefficient;

	string GetComputationMethod(const string &sEndLine="\n") const
	{
		string sResult;
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
