// Tests run once for every kinematic chain (.kc) file of kin5axTests/TestChains.
#include "../stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SolverRoundTrip.h"
#include "MillingToolBall.h"

class ChainFileTest : public ::testing::TestWithParam<string>
{
protected:
	//the exceptions of kin5ax do not derive from std::exception: report their message
	CChain Load()
	{
		try{
			return LoadChain(GetParam());
		} catch (CGeneratorException &e){
			ADD_FAILURE()<<"exception while loading: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
		} catch (_com_error &e){
			ADD_FAILURE()<<"COM error while loading: 0x"<<std::hex<<(unsigned)e.Error();
		}
		return CChain();
	}
};

struct ChainFileTestName
{
	string operator()(const ::testing::TestParamInfo<string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST(ChainFiles, AreFound)
{
	EXPECT_GE(GetChainFiles().size(),(size_t)1)<<"no .kc file in "<<GetTestChainsDir()<<" (the build copies kin5axTests/TestChains there)";
}

TEST_P(ChainFileTest, LoadsAsValid5CChain)
{
	CChain oChain = Load();
	EXPECT_TRUE(oChain.Is5CChain());
	EXPECT_EQ((size_t)3,oChain.GetNumberOfPrismaticLinks());
	EXPECT_EQ((size_t)2,oChain.GetNumberOfRevoluteLinks());
}

TEST_P(ChainFileTest, SolverRoundTrip)
{
	CChain oChain = Load();
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);	//the default tool of the application
	try{
		CSolverRoundTripResult oResult = RunSolverRoundTrip(oChain,oTool,2000,1E-9);
		EXPECT_GT(oResult.m_nTargets,(size_t)50)<<"too few collision-free poses to judge the solver";
		EXPECT_EQ((size_t)0,oResult.m_nNoSolution)<<oResult.m_sFirstProblem;
		EXPECT_EQ((size_t)0,oResult.m_nWrongSolutions)<<oResult.m_sFirstProblem;
		EXPECT_EQ((size_t)0,oResult.m_nNotRecovered)<<oResult.m_sFirstProblem;
		RecordProperty("targets",(int)oResult.m_nTargets);
		RecordProperty("skipped_colliding_poses",(int)oResult.m_nCollisions);
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
}

INSTANTIATE_TEST_CASE_P(AllChains, ChainFileTest, ::testing::ValuesIn(GetChainFiles()), ChainFileTestName());
