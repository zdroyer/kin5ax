// CChainParameter::distanceTo: the plain difference of two distances, the shorter arc between two angles.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "ChainParameter.h"
#include "double_tools.h"

using namespace kin5ax;

//the distance of two revolute parameters, in degrees
static double AngleDistance(double dDegrees1, double dDegrees2)
{
	CChainParameter p1(RADIAN,degrees_to_radians(dDegrees1)), p2(RADIAN,degrees_to_radians(dDegrees2));
	return radians_to_degrees(p1.distanceTo(p2));
}

TEST(ChainParameter, DistanceUnitGivesThePlainDifference)
{
	CChainParameter p1(DISTANCE_UNIT,-350.0), p2(DISTANCE_UNIT,350.0);
	EXPECT_DOUBLE_EQ(700.0,p1.distanceTo(p2));
	EXPECT_DOUBLE_EQ(700.0,p2.distanceTo(p1));
}

//KnownBugs #45: min(fmod(|2pi-d|,2pi),fmod(d,2pi)) gave 340 deg for 350 deg against -350 deg
TEST(ChainParameter, AngleDistanceIsTheShorterArc)
{
	EXPECT_NEAR(20.0,AngleDistance(10.0,30.0),1E-9);
	EXPECT_NEAR(2.0,AngleDistance(359.0,1.0),1E-9);
	EXPECT_NEAR(180.0,AngleDistance(0.0,180.0),1E-9);
	EXPECT_NEAR(20.0,AngleDistance(350.0,-350.0),1E-9)<<"a difference of 700 deg";
	EXPECT_NEAR(90.0,AngleDistance(-90.0,540.0),1E-9)<<"a difference of 630 deg";
	EXPECT_NEAR(0.0,AngleDistance(0.0,720.0),1E-9);
	//every difference up to four turns, both ways round
	for(int d=0;d<=1440;d+=5){
		double dExpected = d%360;
		if(dExpected>180.0)
			dExpected = 360.0-dExpected;
		EXPECT_NEAR(dExpected,AngleDistance(100.0,100.0+d),1E-9)<<"difference "<<d;
		EXPECT_NEAR(dExpected,AngleDistance(100.0+d,100.0),1E-9)<<"difference "<<d;
	}
}

TEST(ChainParameter, AnyNumberIsAtNoDistance)
{
	CChainParameter pAny(RADIAN,0.0,CHAIN_PARAMETER_ANY_NUMBER), p(RADIAN,1.0);
	EXPECT_DOUBLE_EQ(0.0,pAny.distanceTo(p));
	EXPECT_DOUBLE_EQ(0.0,p.distanceTo(pAny));
}

TEST(ChainParameter, IncompatibleUnitsThrow)
{
	CChainParameter p1(RADIAN,1.0), p2(DISTANCE_UNIT,1.0);
	EXPECT_THROW(p1.distanceTo(p2),CGeneratorException);
}
