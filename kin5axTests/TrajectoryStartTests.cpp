// CTrajectoryStart: which candidate of the first solution set a trajectory begins at. The rest of the
// path follows by the shortest route, but the start is a convention of the machine and its control.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SimulationData.h"
#include "ArgsMyException.h"
#include <algorithm>

using namespace kin5ax;

namespace {

//a configuration of a linear axis and a rotary axis: index 1 is the one a policy looks at
CChainConfiguration Config(double dLinear, double dAngle)
{
	CChainConfiguration o(2);
	o[0].SetParameter(dLinear,DISTANCE_UNIT);
	o[1].SetParameter(dAngle,RADIAN);
	return o;
}

//three candidates of one cutter location, all at the same place, telling the rotary axis apart
tChainConfigurationVector ThreeCandidates()
{
	tChainConfigurationVector v;
	v.push_back(Config(0.0,-0.5));
	v.push_back(Config(0.0, 0.2));
	v.push_back(Config(0.0, 0.9));
	return v;
}

CTrajectoryStart Policy(TRAJECTORY_START_CRITERION nCriterion, const CChainConfiguration *pHint = NULL)
{
	CTrajectoryStart o;
	o.m_nCriterion = nCriterion;
	o.m_nParameterIndex = 1;		//the rotary axis of Config
	o.m_pHint = pHint;
	return o;
}

size_t Chosen(const CTrajectoryStart &oStart, const tChainConfigurationVector &v)
{
	return oStart.ChooseIndex(v,CPostprocessorSettings());
}

} // namespace

TEST(TrajectoryStart, TheCriterionLooksAtTheAxisOfThePolicy)
{
	tChainConfigurationVector v = ThreeCandidates();
	EXPECT_EQ((size_t)0,Chosen(Policy(TRAJECTORY_START_MIN),v))<<"-0.5 is the smallest";
	EXPECT_EQ((size_t)2,Chosen(Policy(TRAJECTORY_START_MAX),v))<<"0.9 is the largest";
	EXPECT_EQ((size_t)1,Chosen(Policy(TRAJECTORY_START_MIN_ABS),v))<<"0.2 is the closest to zero";
	EXPECT_EQ((size_t)0,Chosen(Policy(TRAJECTORY_START_SHORTEST_PATH),v))
		<<"nothing is fixed, so the first candidate is handed back and the path decides";
	EXPECT_TRUE(Policy(TRAJECTORY_START_MIN).FixesTheStart());
	EXPECT_FALSE(Policy(TRAJECTORY_START_SHORTEST_PATH).FixesTheStart());
}

TEST(TrajectoryStart, ARandomStartIsDrawnFromTheSeedAndSaysSo)
{
	tChainConfigurationVector v = ThreeCandidates();
	CTrajectoryStart o = Policy(TRAJECTORY_START_RANDOM);
	EXPECT_TRUE(o.FixesTheStart())<<"a drawn candidate needs no axis";
	bool bSeen[3] = {false,false,false};
	for(unsigned int nSeed=0;nSeed<30;nSeed++){
		o.m_nRandomSeed = nSeed;
		size_t n = Chosen(o,v);
		ASSERT_LT(n,v.size());
		bSeen[n] = true;
		o.m_nRandomSeed = nSeed;
		EXPECT_EQ(n,Chosen(o,v))<<"the same seed draws the same candidate";
	}
	EXPECT_TRUE(bSeen[0] && bSeen[1] && bSeen[2])<<"30 seeds reach all three candidates";
	o.m_nRandomSeed = 7;
	EXPECT_NE(std::string::npos,o.Describe().find("random"))<<o.Describe();
	EXPECT_NE(std::string::npos,o.Describe().find("seed 7"))<<o.Describe();
}

TEST(TrajectoryStart, WithoutAnAxisNothingIsFixed)
{
	tChainConfigurationVector v = ThreeCandidates();
	CTrajectoryStart o;
	o.m_nCriterion = TRAJECTORY_START_MIN_ABS;		//asked for, but no axis to apply it to
	EXPECT_FALSE(o.IsResolved());
	EXPECT_FALSE(o.FixesTheStart());
	EXPECT_EQ((size_t)0,Chosen(o,v));
	EXPECT_NE(std::string::npos,o.Describe().find("no axis"))<<o.Describe();
	//and an axis that is not a parameter of these configurations is an error, not a silent 0
	o.m_nParameterIndex = 5;
	EXPECT_TRUE(o.FixesTheStart());
	EXPECT_THROW(Chosen(o,v),CArgsMyException);
}

//the point of the hint: it narrows, and it ties by nature, so something else has to decide
TEST(TrajectoryStart, TheHintNarrowsAndTheCriterionBreaksTheTie)
{
	tChainConfigurationVector v;
	v.push_back(Config(0.0, 0.3));
	v.push_back(Config(0.0,-0.3));
	CChainConfiguration oParked = Config(0.0,0.0);		//the machine at its origin: both are 0.3 away
	EXPECT_EQ((size_t)1,Chosen(Policy(TRAJECTORY_START_MIN,&oParked),v))<<"-0.3 is the smallest of the tie";
	EXPECT_EQ((size_t)0,Chosen(Policy(TRAJECTORY_START_MAX,&oParked),v))<<"0.3 is the largest";
	EXPECT_EQ((size_t)0,Chosen(Policy(TRAJECTORY_START_MIN_ABS,&oParked),v))
		<<"both are as close to zero, so the first of them stands";
	EXPECT_EQ((size_t)0,Chosen(Policy(TRAJECTORY_START_SHORTEST_PATH,&oParked),v))<<"the first of the tie";

	//a hint that prefers one candidate wins over the criterion: it narrows first
	CChainConfiguration oNear = Config(0.0,0.25);
	EXPECT_EQ((size_t)0,Chosen(Policy(TRAJECTORY_START_MIN,&oNear),v))
		<<"0.3 is the nearest to where the machine stands, although -0.3 is the smaller value";
}

TEST(TrajectoryStart, AHintThatIsNotAConfigurationOfThisChainIsRefused)
{
	tChainConfigurationVector v = ThreeCandidates();
	CChainConfiguration oShort(1);				//one parameter, the candidates have two
	oShort[0].SetParameter(0.0,DISTANCE_UNIT);
	EXPECT_THROW(Chosen(Policy(TRAJECTORY_START_MIN,&oShort),v),CArgsMyException);

	//a configuration whose rotary axis is free, as the solver returns them in a singular orientation
	CChainConfiguration oAmbiguous(2);
	oAmbiguous[0].SetParameter(0.0,DISTANCE_UNIT);
	oAmbiguous[1].SetParameter(0.0,RADIAN,CHAIN_PARAMETER_ANY_NUMBER);
	ASSERT_TRUE(oAmbiguous.IsAmbiguous());
	EXPECT_THROW(Chosen(Policy(TRAJECTORY_START_MIN,&oAmbiguous),v),CArgsMyException);
}

TEST(TrajectoryStart, TheDescriptionNamesTheAxisAndTheCriterion)
{
	EXPECT_STREQ("shortest path",TrajectoryStartCriterionName(TRAJECTORY_START_SHORTEST_PATH));
	EXPECT_STREQ("closest to zero",TrajectoryStartCriterionName(TRAJECTORY_START_MIN_ABS));
	EXPECT_STREQ("unknown",TrajectoryStartCriterionName((TRAJECTORY_START_CRITERION)42));
	std::string s = Policy(TRAJECTORY_START_MIN_ABS).Describe();
	EXPECT_NE(std::string::npos,s.find("axis 1"))<<s;
	EXPECT_NE(std::string::npos,s.find("closest to zero"))<<s;
	CChainConfiguration oParked = Config(0.0,0.0);
	EXPECT_NE(std::string::npos,Policy(TRAJECTORY_START_MIN,&oParked).Describe().find("nearest to"));
}

// ---------------------------------------------------------------- the first rotary axis of a machine

class TrajectoryStartChain : public ::testing::TestWithParam<std::string> {};

struct TrajectoryStartChainName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST_P(TrajectoryStartChain, TheFirstRotaryAxisFromBothEnds)
{
	CChain oChain = LoadChain(GetParam());
	ASSERT_TRUE(oChain.Is5CChain());
	const size_t nFromTool = oChain.GetFirstRotaryParameterIndex(CHAIN_PART_SPINDLE);
	const size_t nFromPart = oChain.GetFirstRotaryParameterIndex(CHAIN_PART_TABLE);
	EXPECT_NE(nFromTool,nFromPart)<<"the two rotary axes of a 5C machine are not the same axis";
	//the parameters run from the material terminal to the tool terminal, so the tool end comes second
	EXPECT_GT(nFromTool,nFromPart);
	EXPECT_LT(nFromTool,(size_t)5);
	EXPECT_EQ(CHAIN_SEGMENT_REVOLUTE_LINK,oChain[(int)nFromTool].GetType());
	EXPECT_EQ(CHAIN_SEGMENT_REVOLUTE_LINK,oChain[(int)nFromPart].GetType());

	//the policy of the machine: that axis, closest to zero
	CTrajectoryStart oStart = oChain.GetTrajectoryStart();
	EXPECT_EQ(nFromTool,oStart.m_nParameterIndex);
	EXPECT_EQ(TRAJECTORY_START_MIN_ABS,oStart.m_nCriterion);
	EXPECT_TRUE(oStart.FixesTheStart());
	EXPECT_EQ(nFromPart,oChain.GetTrajectoryStart(CHAIN_PART_TABLE).m_nParameterIndex);
}

INSTANTIATE_TEST_CASE_P(AllChains, TrajectoryStartChain, ::testing::ValuesIn(GetChainFiles()), TrajectoryStartChainName());

TEST(TrajectoryStart, AChainThatIsNotFiveAxisHasNoFirstRotaryAxis)
{
	CChain oPlain;			//the two terminals and nothing between them
	ASSERT_FALSE(oPlain.Is5CChain());
	EXPECT_THROW(oPlain.GetFirstRotaryParameterIndex(CHAIN_PART_SPINDLE),CGeneratorException);
	EXPECT_THROW(oPlain.GetTrajectoryStart(),CGeneratorException);
}

// ---------------------------------------------------------------- the path that follows

namespace {

//two cutter locations, two candidates each: the near pair is a short move, the far pair a long one
CSimulationData TwoLocations()
{
	CSimulationData oData(4);
	tChainConfigurationVector v1;
	v1.push_back(Config(0.0, 0.9));		//candidate 0: the axis far from zero
	v1.push_back(Config(0.0,-0.1));		//candidate 1: the axis close to zero
	tChainConfigurationVector v2;
	v2.push_back(Config(0.0, 1.0));
	v2.push_back(Config(0.0,-1.0));
	oData.AddConfigurationSet(v1);
	oData.AddConfigurationSet(v2);
	return oData;
}

} // namespace

TEST(FindShortestPath, TheCriterionFixesTheStartTheShortestPathWouldNotHavePicked)
{
	CPostprocessorSettings oSettings;
	//free: the shortest path is 0.9 -> 1.0, a tenth of a radian
	CSimulationData oFree = TwoLocations();
	ASSERT_TRUE(oFree.FindShortestPath(oSettings,NULL));
	EXPECT_NEAR(0.9,oFree[0][1].GetValue(),1E-12)<<"the start the shortest path wants";
	EXPECT_NEAR(1.0,oFree[1][1].GetValue(),1E-12);

	//the axis closest to zero: the start is -0.1, and the path follows from there
	CTrajectoryStart oStart;
	oStart.m_nCriterion = TRAJECTORY_START_MIN_ABS;
	oStart.m_nParameterIndex = 1;
	CSimulationData oFixed = TwoLocations();
	ASSERT_TRUE(oFixed.FindShortestPath(oSettings,NULL,oStart));
	EXPECT_NEAR(-0.1,oFixed[0][1].GetValue(),1E-12)<<"the start of the control, not of the shortest path";
	EXPECT_NEAR(-1.0,oFixed[1][1].GetValue(),1E-12)<<"and the nearest candidate after it";
}

TEST(FindShortestPath, AHintMeasuresTheMoveFromWhereTheMachineStands)
{
	CPostprocessorSettings oSettings;
	//the machine stands at -1.0: starting at -0.1 costs 0.9, starting at 0.9 costs 1.9
	CChainConfiguration oParked = Config(0.0,-1.0);
	CTrajectoryStart oStart;
	oStart.m_pHint = &oParked;		//no criterion: the shortest path, now counting the move to the start
	CSimulationData oData = TwoLocations();
	ASSERT_TRUE(oData.FindShortestPath(oSettings,NULL,oStart));
	EXPECT_NEAR(-0.1,oData[0][1].GetValue(),1E-12)<<"the near start wins once the first move is paid for";
	EXPECT_NEAR(-1.0,oData[1][1].GetValue(),1E-12);
}

TEST(FindShortestPath, TheLastChosenConfigurationIsWhereTheMachineIsLeft)
{
	CPostprocessorSettings oSettings;
	CSimulationData oData = TwoLocations();
	CChainConfiguration oLast;
	EXPECT_TRUE(oData.GetLastChosenConfiguration(oLast))<<"before the search: the first candidate";
	ASSERT_TRUE(oData.FindShortestPath(oSettings,NULL));
	ASSERT_TRUE(oData.GetLastChosenConfiguration(oLast));
	EXPECT_NEAR(oData[oData.GetSize()-1][1].GetValue(),oLast[1].GetValue(),1E-12);

	//the free search leaves the machine at 1.0, and the next path continues from there: starting at 0.9
	//costs a tenth, starting at -0.1 costs eleven tenths, so the near branch wins
	EXPECT_NEAR(1.0,oLast[1].GetValue(),1E-12);
	CTrajectoryStart oStart;
	oStart.m_pHint = &oLast;
	CSimulationData oNext = TwoLocations();
	ASSERT_TRUE(oNext.FindShortestPath(oSettings,NULL,oStart));
	EXPECT_NEAR(0.9,oNext[0][1].GetValue(),1E-12)<<"the machine carries on from where it was left";

	//nothing to be left standing at when no set has a solution
	CSimulationData oEmpty(2);
	oEmpty.AddConfigurationSet(tChainConfigurationVector());
	EXPECT_FALSE(oEmpty.GetLastChosenConfiguration(oLast));
}

TEST(FindShortestPath, OneSetIsStillAChoice)
{
	CPostprocessorSettings oSettings;
	CSimulationData oData(2);
	tChainConfigurationVector v;
	v.push_back(Config(0.0, 0.9));
	v.push_back(Config(0.0,-0.1));
	oData.AddConfigurationSet(v);
	CTrajectoryStart oStart;
	oStart.m_nCriterion = TRAJECTORY_START_MIN_ABS;
	oStart.m_nParameterIndex = 1;
	ASSERT_TRUE(oData.FindShortestPath(oSettings,NULL,oStart));
	EXPECT_NEAR(-0.1,oData[0][1].GetValue(),1E-12)<<"a single set has a start like any other";

	//and with two sets of which only one has solutions
	CSimulationData oWithEmpty(3);
	oWithEmpty.AddConfigurationSet(tChainConfigurationVector());
	oWithEmpty.AddConfigurationSet(v);
	ASSERT_TRUE(oWithEmpty.FindShortestPath(oSettings,NULL,oStart));
	EXPECT_NEAR(-0.1,oWithEmpty[1][1].GetValue(),1E-12);
}
