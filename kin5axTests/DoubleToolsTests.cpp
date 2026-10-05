// double_tools.h: the working tolerance of the library and the comparisons built on it.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "double_tools.h"
#include <cmath>
#include <limits>

using namespace kin5ax;

//the tolerance has to be far above the spacing of the doubles at the size of the biggest machine the library
//is used with, and far below anything a machine can mean; the models are in metres and travel up to 8 m
TEST(DoubleTools, TheToleranceIsMeaningfulAtTheSizeOfAMachine)
{
	const double dBiggest = 8.0;		//the travel of the largest shipped model, in metres
	EXPECT_GT(dEPSILON,100.0*std::nextafter(dBiggest,std::numeric_limits<double>::max())-100.0*dBiggest)
		<<"the tolerance must be worth more than a hundred of the last bits at that size";
	EXPECT_LT(dEPSILON,1E-9)<<"and stay far below the accuracy the solver demonstrates";
	//the comparisons do their job at that size: two values a tolerance apart are equal, ten apart are not
	EXPECT_TRUE(are_equal(dBiggest,dBiggest+0.5*dEPSILON));
	EXPECT_FALSE(are_equal(dBiggest,dBiggest+10.0*dEPSILON));
}

//is_zero and is_not_zero are complements, and both are inclusive at the tolerance like are_equal (KnownBugs #56)
TEST(DoubleTools, TheComparisonsAgreeAtTheBoundary)
{
	const double d = dEPSILON;
	EXPECT_TRUE(is_zero(d))<<"exactly at the tolerance is zero, as it is equal";
	EXPECT_FALSE(is_not_zero(d));
	EXPECT_TRUE(are_equal(0.0,d));
	EXPECT_FALSE(are_not_equal(0.0,d));

	const double dMore = 2.0*dEPSILON;
	EXPECT_FALSE(is_zero(dMore));
	EXPECT_TRUE(is_not_zero(dMore));
	EXPECT_FALSE(are_equal(0.0,dMore));

	//zero is zero whatever the tolerance, and the sign does not matter
	EXPECT_TRUE(is_zero(0.0));
	EXPECT_TRUE(is_zero(0.0,0.0));
	EXPECT_TRUE(is_zero(-d));
	EXPECT_FALSE(is_not_zero(0.0,0.0));
}

TEST(DoubleTools, TheOrderComparisonsAreInclusive)
{
	EXPECT_TRUE(is_less_or_equal(1.0,1.0));
	EXPECT_TRUE(is_less_or_equal(1.0+dEPSILON,1.0))<<"within the tolerance counts as not greater";
	EXPECT_FALSE(is_less_or_equal(1.0+10.0*dEPSILON,1.0));
	EXPECT_TRUE(is_more_or_equal(1.0,1.0));
	EXPECT_TRUE(is_more_or_equal(1.0-dEPSILON,1.0));
	EXPECT_FALSE(is_more_or_equal(1.0-10.0*dEPSILON,1.0));
	EXPECT_TRUE(is_between_or_equal(1.0,1.0,2.0));
	EXPECT_TRUE(is_between_or_equal(1.0-dEPSILON,1.0,2.0));
	EXPECT_TRUE(is_between_or_equal(2.0+dEPSILON,1.0,2.0));
	EXPECT_FALSE(is_between_or_equal(1.0-10.0*dEPSILON,1.0,2.0));
	EXPECT_FALSE(is_between_or_equal(2.0+10.0*dEPSILON,1.0,2.0));
}

TEST(DoubleTools, DegreesAndRadians)
{
	EXPECT_NEAR(dPi,degrees_to_radians(180.0),1E-15);
	EXPECT_NEAR(dPiDiv2,degrees_to_radians(90.0),1E-15);
	EXPECT_NEAR(d2Pi,degrees_to_radians(360.0),1E-15);
	EXPECT_NEAR(90.0,radians_to_degrees(dPiDiv2),1E-13);
	EXPECT_NEAR(-30.0,radians_to_degrees(degrees_to_radians(-30.0)),1E-13);
}
