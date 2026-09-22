// CCLDataPostprocessor::InterpolateLocation and CPostprocessorSettings::InterpolationSteps:
// the limits of the settings decide how densely a move between two cutter locations is divided.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "CLDataPostprocessor.h"

using namespace kin5ax;

typedef std::vector<std::pair<double,CR3CaughtVector> > tLocations;

static CPostprocessorSettings Limits(double dPosition, double dOrientationDegrees)
{
	CPostprocessorSettings o;
	o.m_dMaxPositionDifference = dPosition;
	o.m_dMaxOrientationDifferenceDegrees = dOrientationDegrees;
	o.m_dMaxOrientationDifferenceRadians = degrees_to_radians(dOrientationDegrees);
	return o;
}

TEST(InterpolationSteps, FollowsTheStricterLimit)
{
	CPostprocessorSettings o = Limits(0.01,1.0);
	EXPECT_DOUBLE_EQ(1.0,o.InterpolationSteps(0.0,0.0));
	EXPECT_DOUBLE_EQ(1.0,o.InterpolationSteps(0.01,degrees_to_radians(1.0)))<<"exactly the limits: one step";
	EXPECT_DOUBLE_EQ(10.0,o.InterpolationSteps(0.1,0.0));
	EXPECT_DOUBLE_EQ(5.0,o.InterpolationSteps(0.0,degrees_to_radians(4.5)));
	EXPECT_DOUBLE_EQ(10.0,o.InterpolationSteps(0.1,degrees_to_radians(4.5)))<<"the position needs more steps";
	EXPECT_DOUBLE_EQ(10.0,o.InterpolationSteps(-0.1,0.0))<<"a sign does not matter";
}

//KnownBugs #17: a zero limit meant an infinite count of steps (endless loop, or memory exhausted)
TEST(InterpolationSteps, ZeroLimitSwitchesTheCriterionOff)
{
	EXPECT_DOUBLE_EQ(1.0,Limits(0.0,0.0).InterpolationSteps(5.0,3.0));
	EXPECT_DOUBLE_EQ(3.0,Limits(0.0,1.0).InterpolationSteps(5.0,degrees_to_radians(2.5)))<<"only the orientation counts";
	EXPECT_DOUBLE_EQ(50.0,Limits(0.1,0.0).InterpolationSteps(5.0,3.0))<<"only the position counts";
	EXPECT_DOUBLE_EQ(1.0,Limits(-0.1,-1.0).InterpolationSteps(5.0,3.0))<<"negative limits are off too";
}

TEST(InterpolationSteps, HugeCountsAreCapped)
{
	EXPECT_DOUBLE_EQ(CPostprocessorSettings::dMAX_INTERPOLATION_STEPS,Limits(1E-12,1.0).InterpolationSteps(1.0,0.0));
	EXPECT_DOUBLE_EQ(CPostprocessorSettings::dMAX_INTERPOLATION_STEPS,Limits(1E-300,1.0).InterpolationSteps(1E300,0.0))<<"an infinite ratio";
}

TEST(InterpolateLocation, DividesTheMoveEvenly)
{
	CCLDataPostprocessor o(Limits(0.01,1.0));
	tLocations v;
	CR3Point p1(0.0,0.0,0.0), p2(0.04,0.0,0.0);
	CR3Vector v1(0.0,0.0,1.0), v2(0.0,0.0,1.0);
	o.InterpolateLocation(p1,v1,p2,v2,v);
	ASSERT_EQ((size_t)4,v.size())<<"4 steps of 0.01";
	for(size_t i=0;i<v.size();i++){
		EXPECT_DOUBLE_EQ(0.25*(i+1),v[i].first);
		EXPECT_NEAR(0.01*(i+1),v[i].second.m_p.GetX(),1E-12);
		EXPECT_NEAR(1.0,v[i].second.m_v.GetLength(),1E-12);
	}
	EXPECT_DOUBLE_EQ(1.0,v.back().first);
	EXPECT_TRUE(v.back().second.m_p==p2);
}

TEST(InterpolateLocation, TheSameLocationTwiceGivesOneResult)
{
	CCLDataPostprocessor o(Limits(0.01,1.0));
	tLocations v;
	CR3Point p(0.1,0.2,0.3);
	CR3Vector n(0.0,0.0,1.0);
	o.InterpolateLocation(p,n,p,n,v);
	ASSERT_EQ((size_t)1,v.size());
	EXPECT_DOUBLE_EQ(1.0,v[0].first);
}

//KnownBugs #17: with a zero limit the loop never ended
TEST(InterpolateLocation, ZeroLimitsGiveNoIntermediateLocation)
{
	CCLDataPostprocessor o(Limits(0.0,0.0));
	tLocations v;
	o.InterpolateLocation(CR3Point(0.0,0.0,0.0),CR3Vector(0.0,0.0,1.0),CR3Point(1.0,2.0,3.0),CR3Vector(1.0,0.0,0.0),v);
	ASSERT_EQ((size_t)1,v.size());
	EXPECT_TRUE(v[0].second.m_p==CR3Point(1.0,2.0,3.0));
}

TEST(InterpolateLocation, OrientationIsInterpolatedAsUnitVectors)
{
	CCLDataPostprocessor o(Limits(0.0,45.0));
	tLocations v;
	o.InterpolateLocation(CR3Point(0.0,0.0,0.0),CR3Vector(0.0,0.0,1.0),CR3Point(0.0,0.0,0.0),CR3Vector(1.0,0.0,0.0),v);
	ASSERT_EQ((size_t)2,v.size())<<"90 deg in two steps of 45";
	EXPECT_NEAR(1.0,v[0].second.m_v.GetLength(),1E-12);
	EXPECT_NEAR(v[0].second.m_v.GetX(),v[0].second.m_v.GetZ(),1E-12)<<"half way";
}
