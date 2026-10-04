// CSimpleSimulationSetup: a hemisphere and a material block sized for every test machine, and the
// hemisphere postprocessed on it.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SimpleSimulation.h"
#include "CutterLocationData.h"
#include "CLDataPostprocessor.h"
#include "MillingToolBall.h"

using namespace kin5ax;

class SimpleSimulationChain : public ::testing::TestWithParam<std::string> {};

struct SimpleSimulationChainName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST_P(SimpleSimulationChain, SizesSuitTheMachineAndTheTool)
{
	CChain oChain = LoadChain(GetParam());
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);	//the default tool of the application
	CSimpleSimulationSetup o = CSimpleSimulationSetup::ForMachine(oChain,oTool);
	EXPECT_GT(o.m_dWorkingRange,0.0);
	EXPECT_GE(o.m_dRadius,3.0*0.016)<<"at least three tool radii";
	EXPECT_GE(o.m_dRadius,o.m_dWorkingRange/8.0);
	EXPECT_LE(o.m_dRadius,std::max(o.m_dWorkingRange/8.0,3.0*0.016)+1E-12);
	EXPECT_DOUBLE_EQ(o.m_dRadius+0.016,o.m_dCLRadius)<<"the ball centre travels one ball radius outside the surface";
	EXPECT_GT(o.m_dLocationDistance,0.0);
	EXPECT_LT(o.m_dLocationDistance,o.m_dRadius);
	EXPECT_GE(o.m_dBoxSize,2.0*o.m_dRadius)<<"the block holds the hemisphere";
	EXPECT_GE(o.m_dBoxHeight,o.m_dRadius);
	EXPECT_DOUBLE_EQ(-0.5*o.m_dBoxSize,o.GetBoxStart());
	EXPECT_NEAR(50.0,o.m_dBoxSize/o.m_dVoxelSize,1E-9)<<"fifty voxels across";
	RecordProperty("radius_mm",(int)(1000.0*o.m_dRadius));
	RecordProperty("working_range_mm",(int)(1000.0*o.m_dWorkingRange));
}

TEST_P(SimpleSimulationChain, TheHemisphereIsPostprocessedOnTheMachine)
{
	CChain oChain = LoadChain(GetParam());
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);
	CSimpleSimulationSetup o = CSimpleSimulationSetup::ForMachine(oChain,oTool);
	std::string sFilePath = TempFilePath("kin5axTests_simple_"+GetTestName(GetParam())+".cld");
	size_t nWritten = o.WriteCLData(sFilePath);
	ASSERT_GT(nWritten,(size_t)50)<<"a hemisphere worth simulating";
	try{
		CCutterLocationData oData;
		oData.LoadFile(sFilePath,"hemisphere");
		EXPECT_EQ(nWritten,oData.GetLocationsCount());
		EXPECT_EQ((size_t)0,oData.GetErrorsCount());
		CSimulationData oSimulationData;
		CPostprocessorSettings oSettings;
		CCLDataPostprocessor oPostprocessor(oSettings);
		CCLDataPostprocessingResult oResult = oPostprocessor.Process(oChain,oTool,oData,oSimulationData,NULL,0,NULL);
		size_t nSolved = oResult.CountOf(SOLVE_STATUS_SOLVED);
		EXPECT_GT(nSolved,(size_t)0)<<"no location of the hemisphere can be solved on this machine";
		RecordProperty("locations",(int)nWritten);
		RecordProperty("solved",(int)nSolved);
		RecordProperty("limits_exceeded",(int)oResult.CountOf(SOLVE_STATUS_LIMITS_EXCEEDED));
		RecordProperty("collision",(int)oResult.CountOf(SOLVE_STATUS_COLLISION));
	} catch (CGeneratorException &e){
		FAIL()<<e.Message();
	}
	remove(sFilePath.c_str());
}

INSTANTIATE_TEST_CASE_P(AllChains, SimpleSimulationChain, ::testing::ValuesIn(GetChainFiles()), SimpleSimulationChainName());

TEST(SimpleSimulation, WorkingRangeIsTheShortestLimitedLinearTravel)
{
	//the JSON template of the reader tests, built here in code: one limited linear axis of 1 m
	CChain oChain;
	oChain.m_vToolPart.clear();		//the default chain comes with its two terminals
	oChain.m_vMaterialHolderPart.clear();
	CR3Vector r3vZ(0.0,0.0,1.0), r3vNone(0.0,0.0,0.0);
	CChainSegment oZ(CHAIN_SEGMENT_PRISMATIC_LINK,r3vZ,0.0,r3vNone,true,-0.5,0.5);
	CChainSegment oTool(CHAIN_SEGMENT_TOOL,r3vZ,0.0,r3vNone);
	CChainSegment oHolder(CHAIN_SEGMENT_MATERIAL_HOLDER,r3vZ,0.0,r3vNone);
	oChain.m_vToolPart.push_back(oZ);
	oChain.m_vToolPart.push_back(oTool);
	oChain.m_vMaterialHolderPart.push_back(oHolder);
	EXPECT_DOUBLE_EQ(1.0,CSimpleSimulationSetup::WorkingRange(oChain));
	oChain.m_vToolPart[0].m_dParameterValueMinimum = -0.2;
	oChain.m_vToolPart[0].m_dParameterValueMaximum = 0.2;
	EXPECT_DOUBLE_EQ(0.4,CSimpleSimulationSetup::WorkingRange(oChain));
	oChain.m_vToolPart.insert(oChain.m_vToolPart.begin(),CChainSegment(CHAIN_SEGMENT_PRISMATIC_LINK,r3vZ,0.0,r3vNone,true,0.0,0.1));
	EXPECT_DOUBLE_EQ(0.1,CSimpleSimulationSetup::WorkingRange(oChain))<<"the shortest travel";

	//no limited linear axis: the fallback
	CChain oFree;
	oFree.m_vToolPart.clear();
	oFree.m_vMaterialHolderPart.clear();
	oFree.m_vToolPart.push_back(oTool);
	oFree.m_vMaterialHolderPart.push_back(oHolder);
	EXPECT_DOUBLE_EQ(CSimpleSimulationSetup::dFALLBACK_RANGE,CSimpleSimulationSetup::WorkingRange(oFree));
}
