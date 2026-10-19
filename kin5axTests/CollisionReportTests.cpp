// Which solids met: the report itself, what a chain fills into it, and what a whole path adds up to.
// The fact behind the test of the path: a machine whose wrist cannot tilt far collides with itself,
// and a report that only says "it collides" leaves a user nowhere - the pair and the count are what
// can be acted on.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "CollisionReport.h"
#include "Chain.h"
#include "CLDataPostprocessor.h"
#include "MillingToolBall.h"
#include <cmath>

using namespace kin5ax;

namespace {

CMillingToolBall Tool()
{
	return CMillingToolBall(0.016,0.016,0.064,0.016,0.030,CToolHolder(0.050,0.030));
}

} // namespace

TEST(CollisionReport, APairIsTheSameWhicheverSolidWasLookedAtFirst)
{
	CCollisionReport oReport;
	EXPECT_TRUE(oReport.Empty());
	oReport.Add("B","A",COLLIDING_PARTS);
	oReport.Add("A","B",COLLIDING_PARTS);
	ASSERT_EQ((size_t)1,oReport.GetPairs().size())<<"two solids make one pair";
	EXPECT_EQ((size_t)2,oReport.GetPairs()[0].m_nCount);
	EXPECT_EQ("A",oReport.GetPairs()[0].m_sFirst)<<"in an order of their own";
	EXPECT_FALSE(oReport.Empty());

	//the tool is always the first side of its own pair, and the kind tells the pairs apart
	oReport.Add("the tool","A",COLLIDING_TOOL);
	oReport.Add("the shank or the holder","A",COLLIDING_TOOL_SHANK);
	EXPECT_EQ((size_t)3,oReport.GetPairs().size());
	EXPECT_EQ("the tool cuts into A",oReport.GetPairs()[1].Describe());
	EXPECT_EQ("the shank or the holder is in A",oReport.GetPairs()[2].Describe());

	oReport.Clear();
	EXPECT_TRUE(oReport.Empty());
}

TEST(CollisionReport, ALocationCostsOnePerPairHoweverManyOfItsConfigurationsCollided)
{
	CCollisionReport oLocation;
	oLocation.Add("A","B",COLLIDING_PARTS);
	oLocation.Add("A","B",COLLIDING_PARTS);
	oLocation.Add("A","C",COLLIDING_PARTS);
	ASSERT_EQ((size_t)2,oLocation.GetPairs().size());

	CCollisionReport oPath;
	oPath.MergeDistinct(oLocation);
	oPath.MergeDistinct(oLocation);
	ASSERT_EQ((size_t)2,oPath.GetPairs().size());
	for(size_t i=0;i<oPath.GetPairs().size();i++)
		EXPECT_EQ((size_t)2,oPath.GetPairs()[i].m_nCount)<<"two locations, whatever each cost inside";
}

TEST(CollisionReport, TheDescriptionPutsTheWorstPairFirst)
{
	CCollisionReport oReport;
	oReport.Add("A","B",COLLIDING_PARTS);
	oReport.Add("C","D",COLLIDING_PARTS);
	oReport.Add("C","D",COLLIDING_PARTS);
	const std::string s = oReport.Describe();
	EXPECT_EQ((size_t)0,s.find("C and D: 2"))<<s;
	EXPECT_NE(std::string::npos,s.find("A and B: 1"))<<s;
	EXPECT_LT(s.find("C and D"),s.find("A and B"));
}

TEST(CollisionReport, AChainSaysWhichOfItsSolidsMet)
{
	//the model of the spherical wrist: tilted far enough, the solid above the wrist meets the plate
	//below it, which is what that model is for
	CChain oChain = LoadChain(GetTestChainsDir()+"3CWithSphericalWrist_withBaseBox.kc.json");
	CMillingToolBall oTool = Tool();
	oChain.SetToolAssembly(oTool);

	CChainConfiguration oUpright((int)oChain.GetNumberOfRegularLinks());
	oChain.SetParameters(oUpright);
	std::string sLog;
	EXPECT_FALSE(oChain.HasCollision(oTool,sLog))<<"standing up, nothing meets";
	EXPECT_TRUE(oChain.GetCollisions().Empty());

	//the last two joints are the rotary ones of this wrist; tilt the second of them a long way
	CChainConfiguration oTilted = oUpright;
	bool bTilted = false;
	for(size_t i=0;i<oChain.GetNumberOfRegularLinks();i++)
		if(oChain[(int)i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
			oTilted[i].SetValue(2.0);		//radians: far past what the wrist allows
			bTilted = true;
		}
	ASSERT_TRUE(bTilted);
	oChain.SetParameters(oTilted);
	ASSERT_TRUE(oChain.HasCollision(oTool,sLog))<<"tilted that far, something meets";
	const CCollisionReport &oReport = oChain.GetCollisions();
	ASSERT_FALSE(oReport.Empty())<<"and the chain says which solids";
	for(size_t i=0;i<oReport.GetPairs().size();i++){
		EXPECT_FALSE(oReport.GetPairs()[i].m_sFirst.empty());
		EXPECT_FALSE(oReport.GetPairs()[i].m_sSecond.empty());
		EXPECT_NE(oReport.GetPairs()[i].m_sFirst,oReport.GetPairs()[i].m_sSecond);
		EXPECT_GT(oReport.GetPairs()[i].m_nCount,(size_t)0);
	}
	//and the next check starts afresh
	oChain.SetParameters(oUpright);
	EXPECT_FALSE(oChain.HasCollision(oTool,sLog));
	EXPECT_TRUE(oChain.GetCollisions().Empty());
}
