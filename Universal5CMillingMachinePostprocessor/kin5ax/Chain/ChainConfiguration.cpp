#include "../stdafx.h"
#include "ChainConfiguration.h"
#include "ArgsMyException.h"
#include "ChainConfiguration.h"
#include "string_tools.h"


CChainConfiguration::CChainConfiguration(int nParametersCount)
:m_vParametersVectors(nParametersCount,CChainParameter(DISTANCE_UNIT))
,m_dOrientationErrorVectorLength(0.0)
,m_dPositionError(0.0)
,m_bErrorsSet(false)
{

}

double CChainConfiguration::GetPositionError()const
{
	return m_dPositionError;
}
double CChainConfiguration::GetOrientationErrorInDistanceUnit()const
{
	return m_dOrientationErrorVectorLength;
}

void CChainConfiguration::SetErrors(double dPositionError, double dOrientationErrorVectorLength)
{
	m_bErrorsSet=true;
	m_dPositionError=fabs(dPositionError);
	m_dOrientationErrorVectorLength = fabs(dOrientationErrorVectorLength);
}
CChainConfiguration::~CChainConfiguration(void)
{
}

vector<CChainConfiguration> CChainConfiguration::GetEquivalents()const
{
	size_t n=0;
	int nFirst=-1, nSecond=-1;
	for(size_t i=0; i<m_vParametersVectors.size();i++){
		if(m_vParametersVectors[i].IsAngle()){
			n++;
			if(nFirst<0){
				nFirst=(int)i;
			} else {
				nSecond=(int)i;
				break;
			}
		}
	}
	if(n!=2 || nFirst*nSecond<0)
		MY_THROW(CGeneratorException,"Attempt to obtain equivalents of parameter set\ncontaining not exactly 2 revolute joints.");
	tChainParameterVector vA1 = GetAngleEquivalents(m_vParametersVectors[nFirst]);
	tChainParameterVector vA2 = GetAngleEquivalents(m_vParametersVectors[nSecond]);
	vector<CChainConfiguration>	vRes(vA1.size()*vA2.size(),(*this));
	n=0;
	for(size_t i=0;i<vA1.size();i++)
		for(size_t j=0;j<vA2.size();j++){
            vRes[n][nFirst] = vA1[i];
			vRes[n][nSecond] = vA2[j];
			n++;
		}
	return vRes;
}

tChainParameterVector CChainConfiguration::GetAngleEquivalents(const CChainParameter &oParam)const
{
	tChainParameterVector vResult(1,oParam);
	assert(oParam.IsAngle());
	if(oParam.IsSingleNumber()){
		if(is_zero(oParam.GetValue())){
			vResult.push_back(CChainParameter(RADIAN,d2Pi));
			vResult.push_back(CChainParameter(RADIAN,-d2Pi));
		}else{
			if(oParam.GetValue()>0)
				vResult.push_back(CChainParameter(RADIAN,-d2Pi+oParam.GetValue()));
			else
				vResult.push_back(CChainParameter(RADIAN,d2Pi+oParam.GetValue()));
		}
	}
	return vResult;
	
}
double CChainConfiguration::Distance(const CChainConfiguration &c2, const CPostprocessorSettings &oSettings) const
{
	size_t n = GetParametersCount();
	if(n !=c2.GetParametersCount())
		MY_THROW(CArgsMyException,"Cannot compute distance between chain configurations with different sizes");
	double d = 0.0;
	double di = 0.0;
	for(size_t i=0; i<n; i++){
		di = c2.m_vParametersVectors[i].distanceTo(this->m_vParametersVectors[i]);
		if(c2.m_vParametersVectors[i].IsAngle())
			d+= (di * oSettings.m_dAngleDistanceCoefficientRadians);
		else 
			d+= (di * oSettings.m_dDisplacementCoefficient);
	}
	return d;
}

string CChainConfiguration::toString(bool bThreeLines,string sEndLine) const
{
	string sResult ="{";
	for(size_t i=0; i<m_vParametersVectors.size();i++){
		if(i>0)
			sResult+="; ";
		sResult+=m_vParametersVectors[i].toString();
	}
	sResult+="}";
	if(m_bErrorsSet){
		if(bThreeLines)
			sResult+=sEndLine;
		sResult +=(" position error="+double_to_string(m_dPositionError)+"; ");
		if(bThreeLines)
			sResult+=sEndLine;
		sResult +=(" orientation error="+double_to_string(radians_to_degrees( 2.0*asin(m_dOrientationErrorVectorLength/2.0)))+" deg. ~ "+double_to_string(m_dOrientationErrorVectorLength)+" units");
	}
	return sResult;
}

CChainParameter & CChainConfiguration::operator[](size_t i)
{
	if(i<0 || i >= m_vParametersVectors.size())
		MY_THROW(CArgsMyException, "Invalid index of parameter");
	return m_vParametersVectors[i];
}

const CChainParameter & CChainConfiguration::operator[](size_t i) const
{
	if(i<0 || i >= m_vParametersVectors.size())
		MY_THROW(CArgsMyException, "Invalid index of parameter");
	return m_vParametersVectors[i];
}

size_t CChainConfiguration::GetParametersCount() const
{
	return m_vParametersVectors.size();
}
bool CChainConfiguration::IsAmbiguous() const
{
	for(size_t i=0;i<m_vParametersVectors.size();i++)
		if(m_vParametersVectors[i].IsAnyNumber())
			return true;
	return false;
}

void CChainConfiguration::SetAmbiguousParameter(double dVal)
{	
	for(size_t i=0;i<m_vParametersVectors.size();i++){
		if(m_vParametersVectors[i].IsAnyNumber())
			m_vParametersVectors[i].SetParameter(dVal,m_vParametersVectors[i].GetUnit(),CHAIN_PARAMETER_SINGLE_NUMBER);
	}
}
double CChainConfiguration::GetFirstAngleValue() const
{
	for(size_t i=0;i<m_vParametersVectors.size();i++)
		if(m_vParametersVectors[i].IsAngle())
			return m_vParametersVectors[i].GetValue();
	MY_THROW(CGeneratorException,"Attempt to get angle value from strictly shift configuration");
}

void CChainConfiguration::ZeroAll()
{
	for(size_t i=0; i<m_vParametersVectors.size();i++){
		m_vParametersVectors[i].SetValue(0.0);
	}

}