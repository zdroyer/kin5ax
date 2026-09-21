#pragma once
#include "chainparameter.h"
#include "string_tools.h"


class CChainConfiguration
{
public:
	CChainConfiguration(int nParameterCount=0);
	~CChainConfiguration(void);
	size_t GetParametersCount() const;
protected:
	tChainParameterVector m_vParametersVectors;
	double m_dPositionError;
	double m_dOrientationErrorVectorLength;
	bool m_bErrorsSet;
	
	tChainParameterVector GetAngleEquivalents(const CChainParameter &oParam)const;
public:
	double GetPositionError()const;
	double GetOrientationErrorInDistanceUnit()const;
	void SetErrors(double dPositionError, double dOrientationErrorVectorLength);
	double GetFirstAngleValue() const;
	void SetAmbiguousParameter(double dVal);
	bool IsAmbiguous() const;
	vector<CChainConfiguration> GetEquivalents()const;
	double Distance(const CChainConfiguration &c2) const; 
	void ZeroAll();
	string toString(bool bTwoLines = false,string sEndLine=sNL) const;
	CChainParameter & operator[](size_t i);
	const CChainParameter & operator[](size_t i) const;
};

typedef vector<CChainConfiguration> tChainConfigurationVector;