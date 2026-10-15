// The tool mount: the TOOL terminal of every chain is the place a tool holder is clamped, and the tool
// point hangs below it by the gauge length of whatever tool was given, less the height of the programmed
// point above the tip. The solver solves for that point.
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

//Every chain hangs the tool below its terminal, whether or not the file suggests a holder. Until
//2026-10-15 a file without one was read as having its tool point at the terminal, so the same
//element meant two different places and the hosts disagreed about which.
TEST(ToolMount, EveryChainHangsTheToolBelowItsTerminal)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	CMillingToolBall oTool = Assembly();
	oChain.SetToolAssembly(oTool);
	EXPECT_DOUBLE_EQ(dASSEMBLY_DISTANCE,oChain.GetToolMountToToolPointDistance());

	//a chain built in code suggests no holder of its own, and that changes nothing: the holder a
	//file names is a default for a host to offer, not a switch on what the terminal means
	CChain oPlain;
	EXPECT_FALSE(oPlain.HasDefaultToolHolder());
	oPlain.SetToolAssembly(oTool);
	EXPECT_DOUBLE_EQ(dASSEMBLY_DISTANCE,oPlain.GetToolMountToToolPointDistance())
		<<"the tool hangs below the terminal of a chain that names no holder too";
	oPlain.SetDefaultToolHolder(CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
	oPlain.SetToolAssembly(oTool);
	EXPECT_DOUBLE_EQ(dASSEMBLY_DISTANCE,oPlain.GetToolMountToToolPointDistance())
		<<"and naming one changes nothing about where it hangs";

	//the bare machine, with nothing in the spindle
	CChain oBare;
	EXPECT_DOUBLE_EQ(0.0,oBare.GetToolMountToToolPointDistance());
}

TEST(ToolMount, TheToolPointMovesDownTheToolAxis)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	std::mt19937 oGenerator(5);
	for(int s=0;s<20;s++){
		oChain.SetParameters(RandomJointValues(oChain,oGenerator));
		oChain.SetToolMountToToolPointDistance(0.0);
		CR3AnchoredVector cvMount = oChain.GetActualToolPositionAndOrientation();
		oChain.SetToolMountToToolPointDistance(dASSEMBLY_DISTANCE);
		CR3AnchoredVector cvTool = oChain.GetActualToolPositionAndOrientation();
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

//---- the gauge length: the tool length a control is set with, given and not worked out

TEST(GaugeLength, IsGivenAndMovesTheToolPointWithIt)
{
	//Two tools of the same solids on the same machine, one gripped deep in its holder and one
	//standing far out of it. Nothing about the cutting part differs; where the tip sits does, and
	//that is the whole reason the length is measured rather than added up.
	const double dShort = 0.120, dLong = 0.260;
	CMillingToolBall oShort(dRADIUS,dRADIUS,dCUTTING,dRADIUS,dSHANK
		,CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS),dShort);
	CMillingToolBall oLong(dRADIUS,dRADIUS,dCUTTING,dRADIUS,dSHANK
		,CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS),dLong);
	EXPECT_DOUBLE_EQ(dShort,oShort.GetGaugeLength());
	EXPECT_DOUBLE_EQ(dLong,oLong.GetGaugeLength());
	EXPECT_DOUBLE_EQ(dShort-dRADIUS,oShort.GetMountToToolPointDistance());
	EXPECT_DOUBLE_EQ(dLong-dRADIUS,oLong.GetMountToToolPointDistance());

	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	oChain.SetToolAssembly(oShort);
	const CR3AnchoredVector cvShort = oChain.GetActualToolPositionAndOrientation();
	oChain.SetToolAssembly(oLong);
	const CR3AnchoredVector cvLong = oChain.GetActualToolPositionAndOrientation();
	EXPECT_NEAR(dLong-dShort,d_E(cvShort.m_p,cvLong.m_p),1E-12)
		<<"the longer tool reaches exactly its extra length further";
}

TEST(GaugeLength, NotGivenIsTheToolStandingAgainstItsHolder)
{
	//what a caller that has measured nothing means, and what every call of these constructors meant
	//before the length existed: the holder on top of the shank
	CMillingToolBall oTool(dRADIUS,dRADIUS,dCUTTING,dRADIUS,dSHANK
		,CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
	EXPECT_DOUBLE_EQ(dCUTTING+dSHANK+dHOLDER_LENGTH,oTool.GetGaugeLength());
	EXPECT_DOUBLE_EQ(oTool.GetGaugeLength(),oTool.GetAssemblyLength())<<"the older name of the same thing";
	EXPECT_DOUBLE_EQ(dASSEMBLY_DISTANCE,oTool.GetMountToToolPointDistance());

	//and it can be measured afterwards
	oTool.SetGaugeLength(0.200);
	EXPECT_DOUBLE_EQ(0.200,oTool.GetGaugeLength());
	oTool.SetGaugeLength(0.0);
	EXPECT_DOUBLE_EQ(dCUTTING+dSHANK+dHOLDER_LENGTH,oTool.GetGaugeLength())<<"back to the derived one";
}

TEST(GaugeLength, TheHolderHangsFromTheMountAndNotFromTheShank)
{
	//A tool standing 0.06 out of its holder: the holder occupies the 0.05 below the mount, which is
	//0.21 above the tip, and the gap between the top of the shank and the bottom of the holder is
	//the protrusion. The old model had the holder sitting on the shank, so it moved with the tool.
	const double dGauge = 0.260;
	CMillingToolBall oTool(dRADIUS,dRADIUS,dCUTTING,dRADIUS,dSHANK
		,CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS),dGauge);
	const double dHolderBottom = dGauge-dHOLDER_LENGTH;
	EXPECT_GT(dHolderBottom,oTool.GetToolLength())<<"this tool protrudes from its holder";

	//a point just inside the holder, and the same point lowered into the gap, which is air
	EXPECT_TRUE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,dHolderBottom+0.001)));
	EXPECT_TRUE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,dGauge-0.001)));
	EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,dHolderBottom-0.001)))
		<<"between the shank and the holder there is nothing";
	//the shank is still where it was, measured from the tip
	EXPECT_TRUE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,dCUTTING+0.001)));
	//and a radius only the holder has reaches no further down than the holder does
	EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(CR3Point(dHOLDER_RADIUS-0.001,0.0,dCUTTING+0.001)));
}
