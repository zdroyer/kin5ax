#pragma once
#include "ChainParameter.h"
#include "PostprocessorSettings.h"
#include "string_tools.h"


namespace kin5ax {

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
	std::vector<CChainConfiguration> GetEquivalents()const;
	double Distance(const CChainConfiguration &c2, const CPostprocessorSettings &oSettings) const; 
	void ZeroAll();
	std::string toString(bool bTwoLines = false,std::string sEndLine=sNL) const;
	CChainParameter & operator[](size_t i);
	const CChainParameter & operator[](size_t i) const;
};

typedef std::vector<CChainConfiguration> tChainConfigurationVector;

} // namespace kin5ax
