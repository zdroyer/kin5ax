// The tool mount: with a holder declared, the tool point hangs below the TOOL terminal by the length of the
// assembly, and the solver solves for that point. Without a holder the chain behaves as it always did.
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

TEST(ToolMount, IsNotDeclaredWithoutAHolder)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	EXPECT_FALSE(oChain.IsToolMountDeclared())<<"no chain file declares a holder yet";
	EXPECT_DOUBLE_EQ(0.0,oChain.GetToolMountToToolPointDistance());

	CMillingToolBall oTool = Assembly();
	oChain.SetToolAssembly(oTool);
	EXPECT_DOUBLE_EQ(0.0,oChain.GetToolMountToToolPointDistance())<<"an undeclared mount ignores the assembly";

	oChain.SetDefaultToolHolder(CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
	EXPECT_TRUE(oChain.IsToolMountDeclared());
	oChain.SetToolAssembly(oTool);
	EXPECT_DOUBLE_EQ(dASSEMBLY_DISTANCE,oChain.GetToolMountToToolPointDistance());
}

TEST(ToolMount, TheToolPointMovesDownTheToolAxis)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	std::mt19937 oGenerator(5);
	for(int s=0;s<20;s++){
		oChain.SetParameters(RandomJointValues(oChain,oGenerator));
		oChain.SetToolMountToToolPointDistance(0.0);
		CR3CaughtVector cvMount = oChain.GetActualToolPositionAndOrientation();
		oChain.SetToolMountToToolPointDistance(dASSEMBLY_DISTANCE);
		CR3CaughtVector cvTool = oChain.GetActualToolPositionAndOrientation();
		//the axis points from the tip towards the spindle, so the tool point is below the mount
		CR3Point pExpected = cvMount.m_p - (cvMount.m_v.GetUnitVector()*dASSEMBLY_DISTANCE);
		EXPECT_NEAR(0.0,d_E(pExpected,cvTool.m_p),1E-12)<<"sample "<<s;
		EXPECT_NEAR(0.0,(cvMount.m_v-cvTool.m_v).GetLength(),1E-12)<<"the axis is unchanged";
	}
}

class ToolMountChain : public ::testing::TestWithParam<std::string> {};

struct ToolMountChainName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

//the solver must solve for the tool point, not for the mount: the joints that put the tool point on a target
//are the joints the forward kinematics gives back for that target
TEST_P(ToolMountChain, TheSolverSolvesForTheToolPoint)
{
	CChain oChain = LoadChain(GetParam());
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool = Assembly();
	oChain.SetDefaultToolHolder(CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
	oChain.SetToolAssembly(oTool);
	ASSERT_DOUBLE_EQ(dASSEMBLY_DISTANCE,oChain.GetToolMountToToolPointDistance());

	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;	//the holder meets the head boxes of the models (KnownBugs #67)
	C5CPostprocessor oPostprocessor(oChain,oSettings);
	std::mt19937 oGenerator(11);
	size_t nSolved = 0;
	for(int s=0;s<50;s++){
		CChainConfiguration oSet = RandomJointValues(oChain,oGenerator);
		oChain.SetParameters(oSet);
		CR3CaughtVector cvTarget = oChain.GetActualToolPositionAndOrientation();
		std::string sLog;
		CSolveReport oReport;
		tChainConfigurationVector v = oPostprocessor.Solve(cvTarget.m_p,cvTarget.m_v,oChain,(CMillingTool*)NULL,sLog,oReport);
		if(v.empty())
			continue;
		nSolved++;
		bool bFound = false;
		for(size_t k=0;k<v.size();k++){
			oChain.SetParameters(v[k]);
			CR3CaughtVector cv = oChain.GetActualToolPositionAndOrientation();
			EXPECT_NEAR(0.0,d_E(cv.m_p,cvTarget.m_p),1E-9)<<"solution "<<k<<" of sample "<<s;
			EXPECT_NEAR(0.0,(cv.m_v-cvTarget.m_v).GetLength(),1E-9)<<"solution "<<k<<" of sample "<<s;
		}
		(void)bFound;
	}
	EXPECT_GT(nSolved,(size_t)10)<<"too few solved targets to judge the mount offset";
	RecordProperty("solved",(int)nSolved);
}

//and the whole round trip, forward and back, stays exact with an assembly
TEST_P(ToolMountChain, TheRoundTripIsExactWithAnAssembly)
{
	CChain oChain = LoadChain(GetParam());
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool = Assembly();
	oChain.SetDefaultToolHolder(CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
	oChain.SetToolAssembly(oTool);
	CSolverRoundTripResult oResult = RunSolverRoundTrip(oChain,oTool,300,1E-9);
	EXPECT_EQ((size_t)0,oResult.m_nWrongSolutions)<<oResult.m_sFirstProblem;
	EXPECT_EQ((size_t)0,oResult.m_nNotRecovered)<<oResult.m_sFirstProblem;
	RecordProperty("targets",(int)oResult.m_nTargets);
	RecordProperty("no_solution",(int)oResult.m_nNoSolution);
}

INSTANTIATE_TEST_CASE_P(AllChains, ToolMountChain, ::testing::ValuesIn(GetChainFiles()), ToolMountChainName());
