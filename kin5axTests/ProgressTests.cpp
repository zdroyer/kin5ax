// CProgressCallback: the progress reported by CCLDataPostprocessor::Process and CSimulationData::FindShortestPath,
// and their cancellation through Continue().
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SphereCLDataGenerator.h"
#include "CutterLocationData.h"
#include "CLDataPostprocessor.h"
#include "MillingToolBall.h"

using namespace kin5ax;

namespace {

//records every fraction, says "go on" until the nStopAfter-th question
class CRecordingProgress : public CProgressCallback
{
public:
	CRecordingProgress(size_t nStopAfter = 0) :m_nStopAfter(nStopAfter), m_nQuestions(0) {}
	virtual void SetProgress(double dFraction) { m_vFractions.push_back(dFraction); }
	virtual bool Continue()
	{
		m_nQuestions++;
		return m_nStopAfter==0 || m_nQuestions<=m_nStopAfter;
	}
	std::vector<double> m_vFractions;
	size_t m_nStopAfter, m_nQuestions;
};

bool IsNonDecreasingWithin01(const std::vector<double> &v)
{
	for(size_t i=0;i<v.size();i++){
		if(v[i]<0.0 || v[i]>1.0)
			return false;
		if(i && v[i]<v[i-1])
			return false;
	}
	return true;
}

CChainConfiguration Config(double dValue)
{
	CChainConfiguration oConfig(1);
	oConfig[0].SetParameter(dValue,DISTANCE_UNIT);
	return oConfig;
}

//five sets of two solutions each; the shortest path is never the first solution of every set
CSimulationData FiveSets()
{
	CSimulationData oData(10);
	for(int i=0;i<5;i++){
		tChainConfigurationVector v;
		v.push_back(Config(5.0+i));
		v.push_back(Config(0.1*i));
		oData.AddConfigurationSet(v);
	}
	return oData;
}

} // namespace

TEST(Progress, DefaultCallbackAlwaysContinues)
{
	class CPlain : public CProgressCallback
	{
	public:
		virtual void SetProgress(double) {}
	} oPlain;
	EXPECT_TRUE(oPlain.Continue());
}

TEST(Progress, FindShortestPathReportsAndFinishes)
{
	CSimulationData oData = FiveSets();
	CRecordingProgress oProgress;
	EXPECT_TRUE(oData.FindShortestPath(CPostprocessorSettings(),&oProgress));
	ASSERT_GE(oProgress.m_vFractions.size(),(size_t)2);
	EXPECT_DOUBLE_EQ(0.0,oProgress.m_vFractions.front());
	EXPECT_DOUBLE_EQ(1.0,oProgress.m_vFractions.back());
	EXPECT_TRUE(IsNonDecreasingWithin01(oProgress.m_vFractions));
	EXPECT_GE(oProgress.m_nQuestions,(size_t)1);
	for(size_t i=0;i<5;i++)
		EXPECT_DOUBLE_EQ(0.1*i,oData[i][0].GetValue())<<"set "<<i<<": the second solution is on the shortest path";
}

TEST(Progress, FindShortestPathStopsWhenAskedAndLeavesTheFirstSolutions)
{
	CSimulationData oData = FiveSets();
	CRecordingProgress oProgress(2);
	EXPECT_FALSE(oData.FindShortestPath(CPostprocessorSettings(),&oProgress));
	EXPECT_EQ((size_t)3,oProgress.m_nQuestions)<<"the third question was answered with no";
	EXPECT_LT(oProgress.m_vFractions.back(),1.0)<<"never reported as finished";
	for(size_t i=0;i<5;i++)
		EXPECT_DOUBLE_EQ(5.0+i,oData[i][0].GetValue())<<"set "<<i<<": index 0 as before the search";
}

TEST(Progress, ProcessReportsAndFinishes)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	std::string sCLDataFile = TempFilePath("kin5axTests_progress.cld");
	size_t nWritten = WriteSphereCLData(sCLDataFile,0.05,0.01);
	ASSERT_GT(nWritten,(size_t)10);
	try{
		CChain oChain = LoadChain(GetChainFiles()[0]);
		CMillingToolBall oTool(0.016,0.016,0.064,0.016);
		CCutterLocationData oData;
		oData.LoadFile(sCLDataFile,"sphere");
		CSimulationData oSimulationData;
		CPostprocessorSettings oSettings;
		CCLDataPostprocessor oPostprocessor(oSettings);
		CRecordingProgress oProgress;
		CCLDataPostprocessingResult oResult = oPostprocessor.Process(oChain,oTool,oData,oSimulationData,NULL,0,&oProgress);
		EXPECT_FALSE(oResult.m_bCancelled);
		EXPECT_GE(oSimulationData.GetSize(),nWritten);
		EXPECT_EQ(oSimulationData.GetSize(),oResult.m_vLocations.size());
		ASSERT_GE(oProgress.m_vFractions.size(),(size_t)2);
		EXPECT_TRUE(IsNonDecreasingWithin01(oProgress.m_vFractions));
		EXPECT_DOUBLE_EQ(1.0,oProgress.m_vFractions.back());
		EXPECT_GE(oProgress.m_nQuestions,nWritten)<<"asked at every cutter location";
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message();
	}
	remove(sCLDataFile.c_str());
}

TEST(Progress, ProcessStopsWhenAsked)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	std::string sCLDataFile = TempFilePath("kin5axTests_progress_stop.cld");
	size_t nWritten = WriteSphereCLData(sCLDataFile,0.05,0.01);
	ASSERT_GT(nWritten,(size_t)10);
	try{
		CChain oChain = LoadChain(GetChainFiles()[0]);
		CMillingToolBall oTool(0.016,0.016,0.064,0.016);
		CCutterLocationData oData;
		oData.LoadFile(sCLDataFile,"sphere");
		CSimulationData oSimulationData;
		CPostprocessorSettings oSettings;
		CCLDataPostprocessor oPostprocessor(oSettings);
		CRecordingProgress oProgress(5);
		CCLDataPostprocessingResult oResult = oPostprocessor.Process(oChain,oTool,oData,oSimulationData,NULL,0,&oProgress);
		EXPECT_TRUE(oResult.m_bCancelled);
		EXPECT_EQ((size_t)6,oProgress.m_nQuestions);
		EXPECT_LT(oSimulationData.GetSize(),nWritten)<<"stopped long before the end";
		EXPECT_EQ(oSimulationData.GetSize(),oResult.m_vLocations.size())<<"what was solved is consistent";
		EXPECT_LT(oProgress.m_vFractions.empty() ? 0.0 : oProgress.m_vFractions.back(),1.0);
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message();
	}
	remove(sCLDataFile.c_str());
}
