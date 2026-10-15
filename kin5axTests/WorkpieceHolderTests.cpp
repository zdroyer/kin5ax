// The workpiece holder: the vice, the chuck, the plate the part is bolted to. The terminal of the
// workpiece branch is where it is mounted, and the holder is the frame between that terminal and the
// part being cut. Everything the library answers about a location is in the frame of the part, so a
// vice that lifts the part by 80 mm moves the machine by 80 mm and nothing else changes.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SolverRoundTrip.h"
#include "5CPostprocessor.h"
#include "MillingToolBall.h"
#include "Translation.h"
#include "Rotation.h"
#include "Scale.h"
#include "GeneratorException.h"

#include <cmath>

using namespace kin5ax;

extern CR3Vector r3vOZ;	//the OZ of the library, as its own sources take it

namespace {
const double dQUARTER_TURN = 2.0*atan(1.0);	//a quarter turn in radians
const CR3Vector r3vUP(0.0,0.0,1.0);
}

namespace {

const double dRADIUS = 0.016;

CMillingToolBall Tool()
{
	return CMillingToolBall(dRADIUS,dRADIUS,0.064,dRADIUS,0.030,CToolHolder(0.050,0.030));
}

//a vice 0.08 tall, its jaws turned a quarter turn about OZ: a frame, as a vice is to this library
CTransformation Vice()
{
	return CTransformation(CTranslation(0.0,0.0,0.08)) * CTransformation(CRotation(r3vUP,dQUARTER_TURN));
}

} // namespace

TEST(WorkpieceHolder, IsTheIdentityUntilOneIsGiven)
{
	//the part sitting on the table with nothing in between, which is every chain ever solved so far
	CChain oChain;
	EXPECT_FALSE(oChain.HasWorkpieceHolder());
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
			EXPECT_DOUBLE_EQ((i==j) ? 1.0 : 0.0,oChain.GetWorkpieceHolder().M[i][j]);
}

TEST(WorkpieceHolder, AnIdentityGivenIsStillNoHolder)
{
	//a host that always sets one, with nothing to say, must not pay for it nor change an answer
	CChain oChain = LoadChain(GetChainFiles()[0]);
	oChain.SetWorkpieceHolder(CTransformation(1.0));
	EXPECT_FALSE(oChain.HasWorkpieceHolder());
}

TEST(WorkpieceHolder, AFrameThatIsNoWayOfHoldingAnythingLeavesTheChainAlone)
{
	//A vice that scales or shears is not a vice. The chain refuses it, and a chain that refused one
	//is the chain it was: it used to keep the frame and leave the inverse of the one before it.
	CChain oChain = LoadChain(GetChainFiles()[0]);
	oChain.SetWorkpieceHolder(CTransformation(CTranslation(0.0,0.0,0.08)));
	EXPECT_THROW(oChain.SetWorkpieceHolder(CTransformation(CScale(2.0,2.0,2.0))),CGeneratorException);
	EXPECT_TRUE(oChain.HasWorkpieceHolder());
	EXPECT_DOUBLE_EQ(0.08,oChain.GetWorkpieceHolder().M[3][2])<<"the vice it had, not half of another";
	EXPECT_DOUBLE_EQ(-0.08,oChain.GetWorkpieceHolderInverted().M[3][2]);
}

TEST(WorkpieceHolder, LiftingThePartLowersWhatTheMachineReports)
{
	CChain oChain = LoadChain(GetChainFiles()[0]);
	CMillingToolBall oTool = Tool();
	oChain.SetToolAssembly(oTool);
	std::mt19937 oGenerator(3);
	for(int s=0;s<20;s++){
		CChainConfiguration oSet = RandomJointValues(oChain,oGenerator);
		oChain.SetParameters(oSet);
		oChain.SetWorkpieceHolder(CTransformation(1.0));
		const CR3AnchoredVector cvTable = oChain.GetActualToolPositionAndOrientation();
		//the part is clamped 0.08 above the table: the same joints put the tool 0.08 lower in the
		//frame of the part, because the part came up to meet it
		oChain.SetWorkpieceHolder(CTransformation(CTranslation(0.0,0.0,0.08)));
		ASSERT_TRUE(oChain.HasWorkpieceHolder());
		const CR3AnchoredVector cvVice = oChain.GetActualToolPositionAndOrientation();
		EXPECT_NEAR(cvTable.m_p.GetX(),cvVice.m_p.GetX(),1E-12)<<"sample "<<s;
		EXPECT_NEAR(cvTable.m_p.GetY(),cvVice.m_p.GetY(),1E-12)<<"sample "<<s;
		EXPECT_NEAR(cvTable.m_p.GetZ()-0.08,cvVice.m_p.GetZ(),1E-12)<<"sample "<<s;
		EXPECT_NEAR(0.0,(cvTable.m_v-cvVice.m_v).GetLength(),1E-12)<<"a lift turns nothing";
	}
}

TEST(WorkpieceHolder, TurningTheViceTurnsTheFrameTheToolIsReportedIn)
{
	CChain oChain = LoadChain(GetChainFiles()[0]);
	oChain.SetToolAssembly(Tool());
	oChain.SetParameters(RandomJointValues(oChain,*(new std::mt19937(7))));
	oChain.SetWorkpieceHolder(CTransformation(1.0));
	const CR3AnchoredVector cvTable = oChain.GetActualToolPositionAndOrientation();
	//the jaws a quarter turn about OZ: the point of the table frame (x,y) is (y,-x) of the part frame
	oChain.SetWorkpieceHolder(CTransformation(CRotation(r3vUP,dQUARTER_TURN)));
	const CR3AnchoredVector cvVice = oChain.GetActualToolPositionAndOrientation();
	EXPECT_NEAR(cvTable.m_p.GetY(),cvVice.m_p.GetX(),1E-12);
	EXPECT_NEAR(-cvTable.m_p.GetX(),cvVice.m_p.GetY(),1E-12);
	EXPECT_NEAR(cvTable.m_p.GetZ(),cvVice.m_p.GetZ(),1E-12);
	EXPECT_NEAR(cvTable.m_v.GetLength(),cvVice.m_v.GetLength(),1E-12)<<"a turn is not a stretch";
}

//The holder has to reach the solver and not only the forward kinematics. A part lifted 80 mm needs
//the machine 80 mm lower to touch the same point of it, so the joints that solve the same location
//move by the lift. Were the solver working in the frame of the terminal while everything else works
//in the frame of the part, the two would differ by exactly the holder and nothing here would notice.
TEST(WorkpieceHolder, TheSolverPutsTheMachineElsewhereForAVice)
{
	CChain oChain = LoadChain(GetChainFiles()[0]);
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool = Tool();
	oChain.SetToolAssembly(oTool);
	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;
	C5CPostprocessor oPostprocessor(oChain,oSettings);

	//a location of the part, in the frame of the part: it says nothing about where the part is held
	std::mt19937 oGenerator(17);
	oChain.SetParameters(RandomJointValues(oChain,oGenerator));
	const CR3AnchoredVector cvTarget = oChain.GetActualToolPositionAndOrientation();

	std::string sLog;
	CSolveReport oReport;
	oChain.SetWorkpieceHolder(CTransformation(1.0));
	tChainConfigurationVector vTable = oPostprocessor.Solve(cvTarget.m_p,cvTarget.m_v,oChain
		,(CMillingTool*)NULL,sLog,oReport);
	ASSERT_FALSE(vTable.empty())<<sLog;
	const double dLIFT = 0.08;
	oChain.SetWorkpieceHolder(CTransformation(CTranslation(0.0,0.0,dLIFT)));
	tChainConfigurationVector vVice = oPostprocessor.Solve(cvTarget.m_p,cvTarget.m_v,oChain
		,(CMillingTool*)NULL,sLog,oReport);
	ASSERT_FALSE(vVice.empty())<<sLog;

	//the joints moved, and the sliding ones by the lift: the turning ones have nothing to do with it
	double dWorstLinear = 0.0;
	for(size_t i=0;i<vTable[0].GetParametersCount();i++){
		const double dApart = fabs(vVice[0][i].GetValue()-vTable[0][i].GetValue());
		if(oChain[(int)i].GetType()==CHAIN_SEGMENT_PRISMATIC_LINK)
			dWorstLinear = std::max(dWorstLinear,dApart);
		else
			EXPECT_NEAR(0.0,dApart,1E-9)<<"a lift is not a turn: joint "<<i;
	}
	EXPECT_NEAR(dLIFT,dWorstLinear,1E-9)<<"the machine stands the lift away from where it stood";
}

class WorkpieceHolderChain : public ::testing::TestWithParam<std::string> {};

struct WorkpieceHolderChainName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

//The oracle of the whole thing: the solver takes a location in the frame of the part, so the joints
//it finds must put the tool back on that location when the forward kinematics is asked in the same
//frame. A holder that reached one of the two and not the other would show here as an error of
//exactly the vice.
TEST_P(WorkpieceHolderChain, TheSolverWorksInTheFrameOfThePart)
{
	CChain oChain = LoadChain(GetParam());
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool = Tool();
	oChain.SetToolAssembly(oTool);
	oChain.SetWorkpieceHolder(Vice());

	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;	//the holder meets the head boxes of the models (KnownBugs #67)
	C5CPostprocessor oPostprocessor(oChain,oSettings);
	std::mt19937 oGenerator(13);
	size_t nSolved = 0;
	for(int s=0;s<40;s++){
		oChain.SetParameters(RandomJointValues(oChain,oGenerator));
		const CR3AnchoredVector cvTarget = oChain.GetActualToolPositionAndOrientation();
		std::string sLog;
		CSolveReport oReport;
		tChainConfigurationVector v = oPostprocessor.Solve(cvTarget.m_p,cvTarget.m_v,oChain
			,(CMillingTool*)NULL,sLog,oReport);
		if(v.empty())
			continue;
		nSolved++;
		for(size_t k=0;k<v.size();k++){
			oChain.SetParameters(v[k]);
			const CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();
			EXPECT_NEAR(0.0,d_E(cv.m_p,cvTarget.m_p),1E-9)<<"solution "<<k<<" of sample "<<s<<sLog;
			EXPECT_NEAR(0.0,(cv.m_v-cvTarget.m_v).GetLength(),1E-9)<<"solution "<<k<<" of sample "<<s;
		}
	}
	EXPECT_GT(nSolved,(size_t)8)<<"too few solved targets to judge the holder";
	RecordProperty("solved",(int)nSolved);
}

//and the whole round trip, with a vice on the table
TEST_P(WorkpieceHolderChain, TheRoundTripIsExactWithAVice)
{
	CChain oChain = LoadChain(GetParam());
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool = Tool();
	oChain.SetToolAssembly(oTool);
	oChain.SetWorkpieceHolder(Vice());
	CSolverRoundTripResult oResult = RunSolverRoundTrip(oChain,oTool,200,1E-9);
	EXPECT_EQ((size_t)0,oResult.m_nWrongSolutions)<<oResult.m_sFirstProblem;
	EXPECT_EQ((size_t)0,oResult.m_nNotRecovered)<<oResult.m_sFirstProblem;
	RecordProperty("targets",(int)oResult.m_nTargets);
	RecordProperty("no_solution",(int)oResult.m_nNoSolution);
}

INSTANTIATE_TEST_CASE_P(AllChains, WorkpieceHolderChain, ::testing::ValuesIn(GetChainFiles())
	,WorkpieceHolderChainName());
