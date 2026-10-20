// The nose of the tool spindle: the terminal of the tool branch, and the tool offset that hangs the
// programmed point below it. The offset is given to the chain, never derived from the file, and the
// solver solves for that point.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SolverRoundTrip.h"
#include "5CPostprocessor.h"
#include "MillingToolBall.h"

using namespace kin5ax;

namespace {

const double dRADIUS = 0.016;
const double dCUTTING = 0.064;
const double dSHANK = 0.030;
const double dHOLDER_LENGTH = 0.050;
const double dHOLDER_RADIUS = 0.030;

//the default tool of the application with a shank and a holder: assembly 0.144, programmed at the ball centre
CMillingToolBall Assembly()
{
	return CMillingToolBall(dRADIUS,dRADIUS,dCUTTING,dRADIUS,dSHANK,CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
}

const double dASSEMBLY_DISTANCE = dCUTTING+dSHANK+dHOLDER_LENGTH-dRADIUS;

} // namespace

//Every chain hangs the tool below its spindle nose, whether or not the file names a holder. Until
//2026-10-20 a file that named none was read as having its tool point at the terminal, so the same
//element meant two different places and the hosts disagreed about which (KnownBugs #68).
TEST(SpindleNose, EveryChainHangsTheToolBelowItsSpindleNose)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	EXPECT_TRUE(oChain.HasDefaultToolHolder())<<"every shipped model names a tool holder";
	CMillingToolBall oTool = Assembly();
	oChain.SetToolOffsetFromTool(oTool);
	EXPECT_DOUBLE_EQ(dASSEMBLY_DISTANCE,oChain.GetToolOffset());

	//a chain built in code names no holder, and that changes nothing: what the file names is a
	//default to offer, not a switch on what the terminal means
	CChain oPlain;
	EXPECT_FALSE(oPlain.HasDefaultToolHolder());
	oPlain.SetToolOffsetFromTool(oTool);
	EXPECT_DOUBLE_EQ(dASSEMBLY_DISTANCE,oPlain.GetToolOffset());
}

//The offset is a measurement of what is in the spindle, so nothing invents one: a chain says
//whether it was told, and the solver refuses to work on a chain that was not.
TEST(SpindleNose, AChainIsNotToldTheOffsetUntilSomebodyTellsIt)
{
	CChain oChain;
	EXPECT_FALSE(oChain.IsToolOffsetGiven());
	EXPECT_DOUBLE_EQ(0.0,oChain.GetToolOffset())<<"the nose itself, which is the bare machine";
	oChain.SetToolOffset(0.144);
	EXPECT_TRUE(oChain.IsToolOffsetGiven());
	EXPECT_DOUBLE_EQ(0.144,oChain.GetToolOffset());
	//a distance down the axis, and only down: a negative one would end the tool inside the spindle
	EXPECT_THROW(oChain.SetToolOffset(-0.2),CGeneratorException);
	EXPECT_DOUBLE_EQ(0.144,oChain.GetToolOffset())<<"a refused offset changes nothing";
}

//The rule with teeth: a chain that was never told how far the tool reaches refuses to solve, and
//says what to tell it. Solving for the nose itself would put the tool through the part.
TEST(SpindleNose, TheSolverRefusesAChainThatWasNotToldTheOffset)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	ASSERT_TRUE(oChain.Is5CChain());
	ASSERT_FALSE(oChain.IsToolOffsetGiven())<<"a chain is not told by reading a file";
	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;
	C5CPostprocessor oPostprocessor(oChain,oSettings);
	std::string sLog;
	CSolveReport oReport;
	std::string sError;
	try{
		oPostprocessor.Solve(CR3Point(0.0,0.0,0.05),CR3Vector(0.0,0.0,1.0),oChain
			,(CMillingTool*)NULL,sLog,oReport);
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	EXPECT_NE(std::string::npos,sError.find("tool offset"))<<sError;
	EXPECT_NE(std::string::npos,sError.find("SetToolOffset"))<<"it says what to call: "<<sError;

	//told, and it solves
	oChain.SetToolOffsetFromTool(Assembly());
	EXPECT_NO_THROW(oPostprocessor.Solve(CR3Point(0.0,0.0,0.05),CR3Vector(0.0,0.0,1.0),oChain
		,(CMillingTool*)NULL,sLog,oReport));
}

TEST(SpindleNose, TheToolPointMovesDownTheToolAxis)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	std::mt19937 oGenerator(5);
	for(int s=0;s<20;s++){
		oChain.SetParameters(RandomJointValues(oChain,oGenerator));
		oChain.SetToolOffset(0.0);
		CR3AnchoredVector cvNose = oChain.GetActualToolPositionAndOrientation();
		oChain.SetToolOffset(dASSEMBLY_DISTANCE);
		CR3AnchoredVector cvTool = oChain.GetActualToolPositionAndOrientation();
		//the axis points from the tip towards the spindle, so the programmed point is below the nose
		CR3Point pExpected = cvNose.m_p - (cvNose.m_v.GetUnitVector()*dASSEMBLY_DISTANCE);
		EXPECT_NEAR(0.0,d_E(pExpected,cvTool.m_p),1E-12)<<"sample "<<s;
		EXPECT_NEAR(0.0,(cvNose.m_v-cvTool.m_v).GetLength(),1E-12)<<"the axis is unchanged";
	}
}

class SpindleNoseChain : public ::testing::TestWithParam<std::string> {};

struct SpindleNoseChainName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

//the solver must solve for the programmed point, not for the nose: the joints that put that point on a target
//are the joints the forward kinematics gives back for that target
TEST_P(SpindleNoseChain, TheSolverSolvesForTheToolPoint)
{
	CChain oChain = LoadChain(GetParam());
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool = Assembly();
	oChain.SetDefaultToolHolder(CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
	oChain.SetToolOffsetFromTool(oTool);
	ASSERT_DOUBLE_EQ(dASSEMBLY_DISTANCE,oChain.GetToolOffset());

	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;	//the holder meets the head boxes of the models (KnownBugs #67)
	C5CPostprocessor oPostprocessor(oChain,oSettings);
	std::mt19937 oGenerator(11);
	size_t nSolved = 0;
	for(int s=0;s<50;s++){
		CChainConfiguration oSet = RandomJointValues(oChain,oGenerator);
		oChain.SetParameters(oSet);
		CR3AnchoredVector cvTarget = oChain.GetActualToolPositionAndOrientation();
		std::string sLog;
		CSolveReport oReport;
		tChainConfigurationVector v = oPostprocessor.Solve(cvTarget.m_p,cvTarget.m_v,oChain,(CMillingTool*)NULL,sLog,oReport);
		if(v.empty())
			continue;
		nSolved++;
		bool bFound = false;
		for(size_t k=0;k<v.size();k++){
			oChain.SetParameters(v[k]);
			CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();
			EXPECT_NEAR(0.0,d_E(cv.m_p,cvTarget.m_p),1E-9)<<"solution "<<k<<" of sample "<<s;
			EXPECT_NEAR(0.0,(cv.m_v-cvTarget.m_v).GetLength(),1E-9)<<"solution "<<k<<" of sample "<<s;
		}
		(void)bFound;
	}
	EXPECT_GT(nSolved,(size_t)10)<<"too few solved targets to judge the tool offset";
	RecordProperty("solved",(int)nSolved);
}

//and the whole round trip, forward and back, stays exact with an assembly
TEST_P(SpindleNoseChain, TheRoundTripIsExactWithAnAssembly)
{
	CChain oChain = LoadChain(GetParam());
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool = Assembly();
	oChain.SetDefaultToolHolder(CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
	oChain.SetToolOffsetFromTool(oTool);
	CSolverRoundTripResult oResult = RunSolverRoundTrip(oChain,oTool,300,1E-9);
	EXPECT_EQ((size_t)0,oResult.m_nWrongSolutions)<<oResult.m_sFirstProblem;
	EXPECT_EQ((size_t)0,oResult.m_nNotRecovered)<<oResult.m_sFirstProblem;
	RecordProperty("targets",(int)oResult.m_nTargets);
	RecordProperty("no_solution",(int)oResult.m_nNoSolution);
}

INSTANTIATE_TEST_CASE_P(AllChains, SpindleNoseChain, ::testing::ValuesIn(GetChainFiles()), SpindleNoseChainName());
