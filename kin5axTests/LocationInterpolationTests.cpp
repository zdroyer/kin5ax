// The densification of a path with no machine in sight: InterpolateLocation between two locations and
// DensifyLocations over a whole path, as the settings ask. The solver's own use of it is tested in
// InterpolationTests.cpp; this is the free function and the tags of a dense path.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "LocationInterpolation.h"
#include <cmath>

using namespace kin5ax;

namespace {

CPostprocessorSettings Settings(double dMaxStep, double dMaxTurnDegrees, bool bInterpolate = true)
{
	CPostprocessorSettings o;
	o.m_bInterpolateBetweenLocations = bInterpolate;
	o.m_dMaxPositionDifference = dMaxStep;
	o.m_dMaxOrientationDifferenceDegrees = dMaxTurnDegrees;
	o.m_dMaxOrientationDifferenceRadians = dMaxTurnDegrees*4.0*atan(1.0)/180.0;
	return o;
}

const CR3Vector vUP(0.0,0.0,1.0);

} // namespace

TEST(LocationInterpolation, AMoveIsCutIntoStepsNoLongerThanTheLimit)
{
	std::vector<std::pair<double,CR3AnchoredVector> > v;
	InterpolateLocation(CR3Point(0,0,0),vUP,CR3Point(100,0,0),vUP,Settings(10.0,0.0),v);
	ASSERT_EQ((size_t)10,v.size())<<"ten steps of ten, the first location itself not among them";
	for(size_t i=0;i<v.size();i++){
		EXPECT_NEAR(10.0*(double)(i+1),v[i].second.m_p.GetX(),1E-9);
		EXPECT_NEAR((double)(i+1)/10.0,v[i].first,1E-9);
	}
	EXPECT_EQ(1.0,v.back().first)<<"the last pair is the second location";
}

TEST(LocationInterpolation, TheAxisTurnsAtAConstantRateInThePlaneOfBoth)
{
	const double dRad = 4.0*atan(1.0)/180.0;
	const CR3Vector vLeaning(sin(30.0*dRad),0.0,cos(30.0*dRad));
	std::vector<std::pair<double,CR3AnchoredVector> > v;
	InterpolateLocation(CR3Point(0,0,0),vUP,CR3Point(0,0,0),vLeaning,Settings(0.0,1.0),v);
	ASSERT_EQ((size_t)30,v.size())<<"thirty degrees a degree at a time";
	for(size_t i=0;i<v.size();i++){
		const CR3Vector &u = v[i].second.m_v;
		EXPECT_NEAR(1.0,u.GetLength(),1E-9)<<"a unit vector";
		EXPECT_NEAR((double)(i+1)*dRad,vUP.UndirectedUnitVectorsAngle(u),1E-9)<<"step "<<i;
		EXPECT_NEAR(0.0,u.GetY(),1E-12)<<"in the plane of both";
	}
}

TEST(LocationInterpolation, NoInterpolationIsTheSecondLocationAlone)
{
	std::vector<std::pair<double,CR3AnchoredVector> > v;
	InterpolateLocation(CR3Point(0,0,0),vUP,CR3Point(100,0,0),vUP,Settings(10.0,1.0,false),v);
	ASSERT_EQ((size_t)1,v.size());
	EXPECT_NEAR(100.0,v[0].second.m_p.GetX(),1E-12);
	//and so is a move shorter than a step
	InterpolateLocation(CR3Point(0,0,0),vUP,CR3Point(5,0,0),vUP,Settings(10.0,1.0),v);
	ASSERT_EQ((size_t)1,v.size());
}

TEST(LocationInterpolation, ADensePathTagsEveryLocationWithTheOneItBelongsTo)
{
	std::vector<CR3AnchoredVector> vPath;
	vPath.push_back(CR3AnchoredVector(vUP,CR3Point(0,0,0)));
	vPath.push_back(CR3AnchoredVector(vUP,CR3Point(30,0,0)));
	vPath.push_back(CR3AnchoredVector(vUP,CR3Point(30,20,0)));
	const std::vector<CDenseLocation> v = DensifyLocations(vPath,Settings(10.0,0.0));
	ASSERT_EQ((size_t)6,v.size())<<"the first, three steps to the second, two to the third";
	EXPECT_EQ((size_t)0,v[0].m_nSource);
	EXPECT_FALSE(v[0].m_bInvented);
	EXPECT_EQ((size_t)0,v[1].m_nSource)<<"an invented location carries the one before it";
	EXPECT_TRUE(v[1].m_bInvented);
	EXPECT_EQ((size_t)0,v[2].m_nSource);
	EXPECT_EQ((size_t)1,v[3].m_nSource)<<"the arriving location carries its own";
	EXPECT_FALSE(v[3].m_bInvented);
	EXPECT_NEAR(30.0,v[3].m_cv.m_p.GetX(),1E-12);
	EXPECT_EQ((size_t)1,v[4].m_nSource);
	EXPECT_TRUE(v[4].m_bInvented);
	EXPECT_EQ((size_t)2,v[5].m_nSource);
	EXPECT_NEAR(20.0,v[5].m_cv.m_p.GetY(),1E-12);
}

TEST(LocationInterpolation, AnEmptyPathAndAPathOfOne)
{
	std::vector<CR3AnchoredVector> vPath;
	EXPECT_TRUE(DensifyLocations(vPath,Settings(10.0,1.0)).empty());
	vPath.push_back(CR3AnchoredVector(vUP,CR3Point(1,2,3)));
	const std::vector<CDenseLocation> v = DensifyLocations(vPath,Settings(10.0,1.0));
	ASSERT_EQ((size_t)1,v.size());
	EXPECT_NEAR(3.0,v[0].m_cv.m_p.GetZ(),1E-12);
	EXPECT_FALSE(v[0].m_bInvented);
}
