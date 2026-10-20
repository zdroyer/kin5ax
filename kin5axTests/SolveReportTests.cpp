// C5CPostprocessor::Solve with a CSolveReport: the counts of its stages, the status that follows from
// them, and the statuses of a batch (CCLDataPostprocessingResult).
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SolverRoundTrip.h"
#include "5CPostprocessor.h"
#include "MillingToolBall.h"
#include <random>

using namespace kin5ax;

TEST(SolveReport, StatusFollowsTheCounts)
{
	CSolveReport o;
	EXPECT_EQ(SOLVE_STATUS_NO_ORIENTATION,o.Status())<<"nothing found at all";
	o.m_nOrientationSolutions = 2;
	EXPECT_EQ(SOLVE_STATUS_NO_POSITION,o.Status());
	o.m_nTheoreticalSolutions = 2;
	EXPECT_EQ(SOLVE_STATUS_LIMITS_EXCEEDED,o.Status());
	o.m_nWithinLimits = 1;
	EXPECT_EQ(SOLVE_STATUS_COLLISION,o.Status());
	o.m_nCollisionFree = 1;
	EXPECT_EQ(SOLVE_STATUS_SOLVED,o.Status());
	o.Clear();
	EXPECT_EQ(SOLVE_STATUS_NO_ORIENTATION,o.Status());
	EXPECT_FALSE(o.m_bAmbiguous);
}

TEST(SolveReport, EveryStatusHasAName)
{
	for(int s=0;s<SOLVE_STATUS_COUNT;s++){
		EXPECT_STRNE("",SolveStatusName((SOLVE_STATUS)s));
		EXPECT_STRNE("unknown",SolveStatusName((SOLVE_STATUS)s));
	}
	EXPECT_STREQ("solved",SolveStatusName(SOLVE_STATUS_SOLVED));
	EXPECT_STREQ("unknown",SolveStatusName(SOLVE_STATUS_COUNT));
}

class SolveReportChain : public ::testing::TestWithParam<std::string>
{
protected:
	CChain Load()
	{
		try{
			return LoadChain(GetParam());
		} catch (CGeneratorException &e){
			ADD_FAILURE()<<"exception while loading: "<<e.Message();
		}
		return CChain();
	}
	//true when a prismatic link of the chain has limits: a point a kilometre away is then out of reach
	static bool HasLinearLimits(const CChain &oChain)
	{
		for(size_t i=0;i<oChain.GetNumberOfRegularLinks();i++)
			if(oChain[(int)i].GetType()==CHAIN_SEGMENT_PRISMATIC_LINK && oChain[(int)i].IsParameterConstrained())
				return true;
		return false;
	}
};

struct SolveReportChainName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

//the pose of a random joint setting is a solution of its own cutter location: solved when it does not
//collide; and the counts of the stages never grow
TEST_P(SolveReportChain, CountsAreConsistentOnRandomPoses)
{
	CChain oChain = Load();
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);
	oChain.SetToolOffsetFromTool(oTool);
	C5CPostprocessor oPostprocessor(oChain,CPostprocessorSettings());
	std::mt19937 oGenerator(2026);
	size_t nByStatus[SOLVE_STATUS_COUNT] = {0};
	for(int s=0;s<300;s++){
		CChainConfiguration oSet = RandomJointValues(oChain,oGenerator);
		oChain.SetParameters(oSet);
		std::string sLog;
		bool bCollides = oChain.HasCollision(oTool,sLog);
		CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();
		CSolveReport oReport;
		sLog = "";
		tChainConfigurationVector v;
		try{
			v = oPostprocessor.Solve(cv.m_p,cv.m_v,oChain,oTool,sLog,oReport);
		} catch (CGeneratorException &e){
			FAIL()<<"exception: "<<e.Message()<<" for the joints "<<oSet.toString();
		}
		EXPECT_GE(oReport.m_nOrientationSolutions,(size_t)1)<<"the pose exists, so does its orientation: "<<oSet.toString();
		EXPECT_GE(oReport.m_nTheoreticalSolutions,oReport.m_nWithinLimits);
		EXPECT_GE(oReport.m_nWithinLimits,oReport.m_nCollisionFree);
		EXPECT_EQ(v.size(),oReport.m_nCollisionFree);
		EXPECT_EQ(v.empty(),oReport.Status()!=SOLVE_STATUS_SOLVED);
		if(!bCollides){
			EXPECT_EQ(SOLVE_STATUS_SOLVED,oReport.Status())<<"a collision-free pose must be solved: "<<oSet.toString()<<"\n"<<sLog;
		}
		nByStatus[oReport.Status()]++;
	}
	EXPECT_GT(nByStatus[SOLVE_STATUS_SOLVED],(size_t)0);
	RecordProperty("solved",(int)nByStatus[SOLVE_STATUS_SOLVED]);
	RecordProperty("collision",(int)nByStatus[SOLVE_STATUS_COLLISION]);
	RecordProperty("limits_exceeded",(int)nByStatus[SOLVE_STATUS_LIMITS_EXCEEDED]);
}

//a vertical tool a kilometre away: every machine finds the angles and the shifts, the limits decide
TEST_P(SolveReportChain, AKilometreAwayIsDecidedByTheLinearLimits)
{
	CChain oChain = Load();
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);
	oChain.SetToolOffsetFromTool(oTool);
	C5CPostprocessor oPostprocessor(oChain,CPostprocessorSettings());
	CSolveReport oReport;
	std::string sLog;
	tChainConfigurationVector v = oPostprocessor.Solve(CR3Point(1000.0,0.0,0.0),CR3Vector(0.0,0.0,1.0),oChain,oTool,sLog,oReport);
	EXPECT_GE(oReport.m_nOrientationSolutions,(size_t)1)<<sLog;
	EXPECT_GE(oReport.m_nTheoreticalSolutions,(size_t)1)<<sLog;
	if(HasLinearLimits(oChain)){
		EXPECT_EQ(SOLVE_STATUS_LIMITS_EXCEEDED,oReport.Status())<<sLog;
		EXPECT_TRUE(v.empty());
	} else {
		EXPECT_EQ(SOLVE_STATUS_SOLVED,oReport.Status())<<sLog;
		EXPECT_EQ(v.size(),oReport.m_nCollisionFree);
	}
}

INSTANTIATE_TEST_CASE_P(AllChains, SolveReportChain, ::testing::ValuesIn(GetChainFiles()), SolveReportChainName());

//a tool axis the kinematics cannot produce: the nutating table tilts the workpiece normal at most to the
//horizontal, so a tool pointing straight down (axis -Z) has no orientation solution
TEST(SolveReport, AnUnreachableOrientationIsReported)
{
	std::vector<std::string> vFiles = GetChainFiles();
	std::string sFile;
	for(size_t i=0;i<vFiles.size();i++)
		if(vFiles[i].find("AC45_NutatingTable")!=std::string::npos)
			sFile = vFiles[i];
	if(sFile.empty()){
		RecordProperty("skipped","AC45_NutatingTable.kc not among the test chains");
		return;
	}
	CChain oChain = LoadChain(sFile);
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);
	oChain.SetToolOffsetFromTool(oTool);
	C5CPostprocessor oPostprocessor(oChain,CPostprocessorSettings());
	CSolveReport oReport;
	std::string sLog;
	tChainConfigurationVector v = oPostprocessor.Solve(CR3Point(0.0,0.0,0.1),CR3Vector(0.0,0.0,-1.0),oChain,oTool,sLog,oReport);
	EXPECT_TRUE(v.empty());
	EXPECT_EQ(SOLVE_STATUS_NO_ORIENTATION,oReport.Status())<<sLog;
	EXPECT_EQ((size_t)0,oReport.m_nOrientationSolutions);
}

//---- the toggles of the settings (C ABI item 3)
TEST_P(SolveReportChain, CollisionsOffNeedNoTool)
{
	CChain oChain = Load();
	ASSERT_TRUE(oChain.Is5CChain());
	oChain.SetToolOffset(0.144);			//a tool is still in the spindle, it is just not checked
	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;
	C5CPostprocessor oPostprocessor(oChain,oSettings);
	std::mt19937 oGenerator(3);
	for(int s=0;s<50;s++){
		CChainConfiguration oSet = RandomJointValues(oChain,oGenerator);
		oChain.SetParameters(oSet);
		CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();
		CSolveReport oReport;
		std::string sLog;
		tChainConfigurationVector v = oPostprocessor.Solve(cv.m_p,cv.m_v,oChain,(CMillingTool*)NULL,sLog,oReport);
		EXPECT_EQ(oReport.m_nWithinLimits,oReport.m_nCollisionFree)<<"nothing is rejected for a collision";
		EXPECT_EQ(SOLVE_STATUS_SOLVED,oReport.Status())<<"the drawn pose is inside the limits: "<<oSet.toString()<<"\n"<<sLog;
		EXPECT_EQ(v.size(),oReport.m_nCollisionFree);
	}
}

TEST_P(SolveReportChain, LimitsOffReachAKilometreAway)
{
	CChain oChain = Load();
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);
	oChain.SetToolOffsetFromTool(oTool);
	CPostprocessorSettings oSettings;
	oSettings.m_bCheckLimits = false;
	C5CPostprocessor oPostprocessor(oChain,oSettings);
	CSolveReport oReport;
	std::string sLog;
	tChainConfigurationVector v = oPostprocessor.Solve(CR3Point(1000.0,0.0,0.0),CR3Vector(0.0,0.0,1.0),oChain,oTool,sLog,oReport);
	EXPECT_EQ(oReport.m_nTheoreticalSolutions,oReport.m_nWithinLimits)<<"nothing is rejected for a limit";
	EXPECT_EQ(SOLVE_STATUS_SOLVED,oReport.Status())<<sLog;
	EXPECT_FALSE(v.empty());
}

TEST_P(SolveReportChain, CollisionsOnWithoutAToolThrow)
{
	CChain oChain = Load();
	ASSERT_TRUE(oChain.Is5CChain());
	oChain.SetToolOffset(0.144);			//a tool of some length: this test is about the lack of one
	C5CPostprocessor oPostprocessor(oChain,CPostprocessorSettings());
	CSolveReport oReport;
	std::string sLog;
	EXPECT_THROW(oPostprocessor.Solve(CR3Point(0.0,0.0,0.0),CR3Vector(0.0,0.0,1.0),oChain,(CMillingTool*)NULL,sLog,oReport),CGeneratorException);
}
