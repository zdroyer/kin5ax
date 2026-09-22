#pragma once

#include "GeneratorException.h"
#include "ArgsMyException.h"

#include "double_tools.h"
#include "string_tools.h"

enum CHAIN_PARAMETER_TYPE{
	CHAIN_PARAMETER_SINGLE_NUMBER=1,
	CHAIN_PARAMETER_ANY_NUMBER=-1,
};
enum CHAIN_PARAMETER_UNIT{
	RADIAN,
	DISTANCE_UNIT
};

class CChainParameter
{
public:
	CChainParameter(CHAIN_PARAMETER_UNIT unit, double dValue=0.0 ,CHAIN_PARAMETER_TYPE nType=CHAIN_PARAMETER_SINGLE_NUMBER){
		SetParameter(dValue,unit,nType);
	}

	/*
	CChainParameter(CHAIN_PARAMETER_UNIT unit = DISTANCE_UNIT,CHAIN_PARAMETER_TYPE nType=CHAIN_PARAMETER_SINGLE_NUMBER){
		SetParameter(0.0,unit,nType);
	}
//*/

	double GetValue() const {
		if(m_nType==CHAIN_PARAMETER_ANY_NUMBER)
			return 0.0;
		return m_dValue;
	}

	CHAIN_PARAMETER_UNIT GetUnit() const {
		return m_nUnit;
	}


	CHAIN_PARAMETER_TYPE GetType() const {
		return m_nType;
	}
	void ApplySign(double dSign){
		if(dSign<0){
			m_dValue = - m_dValue;
		}
	}
	
	~CChainParameter(void){};
/*	
	CChainParameter toRadians() const{
		CChainParameter oResult(*this);
		oResult.m_dValue1 = degrees_to_radians(oResult.m_dValue1);
		oResult.m_dValue2 = degrees_to_radians(oResult.m_dValue2);
		return oResult;
	}

	CChainParameter toDegrees() const{
		CChainParameter oResult(*this);
		oResult.m_dValue1 = radians_to_degrees(oResult.m_dValue1);
		oResult.m_dValue2 = radians_to_degrees(oResult.m_dValue2);
		return oResult;
	}

//*/
	bool IsAngle() const
	{return m_nUnit == RADIAN;}

	bool IsDisplacement() const
	{return m_nUnit == DISTANCE_UNIT;}


	std::string toString( ) const
	{
		if(m_nType==CHAIN_PARAMETER_ANY_NUMBER)
			return "ANY";
		return string_printf("%2.4G",m_dValue);
	}
	bool operator==(const CChainParameter &p2)const
	{
		if(m_nType == p2.m_nType
			&& m_nUnit == p2.m_nUnit
			&& are_equal(m_dValue,p2.m_dValue))
			return true;
		return false;
	}

	double distanceTo(const CChainParameter &p2) const{
		if(m_nUnit!=p2.m_nUnit)
			MY_THROW(CArgsMyException,"Cannot compute difference between chain parameters - incompatible units");
		if(m_nType == CHAIN_PARAMETER_SINGLE_NUMBER
			&& p2.m_nType == CHAIN_PARAMETER_SINGLE_NUMBER)
			return difference(m_dValue,p2.m_dValue);
//		if(m_nType == CHAIN_PARAMETER_ANY_NUMBER 
//			|| p2.m_nType == CHAIN_PARAMETER_ANY_NUMBER)
			return 0.0;
	}
	bool IsAnyNumber() const
	{ return m_nType == CHAIN_PARAMETER_ANY_NUMBER;}
	
	bool IsSingleNumber() const
	{ return m_nType == CHAIN_PARAMETER_SINGLE_NUMBER;}

	void SetParameter(double dValue,CHAIN_PARAMETER_UNIT unit,CHAIN_PARAMETER_TYPE nType=CHAIN_PARAMETER_SINGLE_NUMBER)
	{
		m_nType = nType;
		m_dValue = dValue;
		m_nUnit = unit;
	}
	void SetValue(double dValue)
	{
		m_dValue = dValue;
	}


protected:
	double difference(double d1, double d2) const{
		double d = fabs(d1-d2);
		if(m_nUnit == DISTANCE_UNIT) 
			return d;
		double dif1 = fabs(d2Pi-d);
		return std::min(fmod(dif1,d2Pi),fmod(d,d2Pi));
	}

	double m_dValue;
	CHAIN_PARAMETER_TYPE m_nType;
	CHAIN_PARAMETER_UNIT m_nUnit;


};

typedef std::vector<CChainParameter> tChainParameterVector;
