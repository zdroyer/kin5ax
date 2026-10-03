// CMaterialSimulation: the trajectory replayed on a chain, the tool following, the material cut; progress and stop.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SolverRoundTrip.h"
#include "MaterialSimulation.h"
#include "MillingToolBall.h"

using namespace kin5ax;

namespace {

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

//the first test chain, a small ball tool, a trajectory of four locations (the third without a solution) and a
//2 cm material cube centred on the tool tip of the first location
struct CScene
{
	CChain m_oChain;
	CMillingToolBall m_oTool;
	CSimulationData m_oTrajectory;
	CMaterial m_oMaterial;
	CR3Point m_pFirstTip;
	CChainConfiguration m_oFirst;

	CScene() :m_oTool(0.002,0.002,0.01,0.0), m_oTrajectory(10)
	{
		m_oChain = LoadChain(GetChainFiles()[0]);
		std::mt19937 oGenerator(77);
		m_oFirst = RandomJointValues(m_oChain,oGenerator);
		CChainConfiguration oSecond = RandomJointValues(m_oChain,oGenerator);
		m_oChain.SetParameters(m_oFirst);
		m_pFirstTip = m_oChain.GetActualToolPositionAndOrientation().m_p;
		m_oMaterial.GenerateBox(0.02,0.02,0.02,0.002,m_pFirstTip.GetX()-0.01,m_pFirstTip.GetY()-0.01,m_pFirstTip.GetZ()-0.01);
		tChainConfigurationVector vFirst(1,m_oFirst), vSecond(1,oSecond), vEmpty;
		m_oTrajectory.AddConfigurationSet(vFirst);
		m_oTrajectory.AddConfigurationSet(vSecond);
		m_oTrajectory.AddConfigurationSet(vEmpty);
		m_oTrajectory.AddConfigurationSet(vFirst);
		//the chain starts somewhere else
		m_oChain.SetParameters(oSecond);
	}
	bool IsRemovedAtTheFirstTip() const
	{
		size_t i = (size_t)floor((m_pFirstTip.GetX()-(m_pFirstTip.GetX()-0.01))/0.002+1E-9);
		return m_oMaterial.Is(VOXEL_BLACK,i,i,i);	//5,5,5: the voxel at the centre of the cube
	}
};

} // namespace

TEST(MaterialSimulation, StepsThroughTheLocationsWithASolution)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CScene s;
	CMaterialSimulation oSimulation(s.m_oChain,s.m_oTool,s.m_oTrajectory,s.m_oMaterial);
	EXPECT_EQ((size_t)4,oSimulation.GetStepCount());
	EXPECT_EQ((size_t)0,oSimulation.GetStepIndex());
	EXPECT_FALSE(oSimulation.IsFinished());
	EXPECT_FALSE(s.IsRemovedAtTheFirstTip());

	EXPECT_TRUE(oSimulation.Step());
	EXPECT_EQ((size_t)1,oSimulation.GetStepIndex());
	EXPECT_TRUE(s.IsRemovedAtTheFirstTip())<<"the tool tip of the first location is inside the material";
	EXPECT_EQ(s.m_oFirst.toString(),s.m_oChain.GetParameters().toString())<<"the chain stands at the first location";

	EXPECT_TRUE(oSimulation.Step());
	EXPECT_EQ((size_t)2,oSimulation.GetStepIndex());
	EXPECT_TRUE(oSimulation.Step())<<"the empty third set is skipped, the fourth is done";
	EXPECT_EQ((size_t)4,oSimulation.GetStepIndex());
	EXPECT_TRUE(oSimulation.IsFinished());
	EXPECT_FALSE(oSimulation.Step())<<"exhausted";
	EXPECT_EQ((size_t)4,oSimulation.GetStepIndex());

	oSimulation.Reset();
	EXPECT_EQ((size_t)0,oSimulation.GetStepIndex());
	EXPECT_TRUE(oSimulation.Step());
}

TEST(MaterialSimulation, RunReportsAndFinishes)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CScene s;
	CMaterialSimulation oSimulation(s.m_oChain,s.m_oTool,s.m_oTrajectory,s.m_oMaterial);
	CRecordingProgress oProgress;
	EXPECT_TRUE(oSimulation.Run(&oProgress));
	EXPECT_TRUE(oSimulation.IsFinished());
	EXPECT_TRUE(s.IsRemovedAtTheFirstTip());
	ASSERT_GE(oProgress.m_vFractions.size(),(size_t)2);
	EXPECT_DOUBLE_EQ(0.0,oProgress.m_vFractions.front());
	EXPECT_DOUBLE_EQ(1.0,oProgress.m_vFractions.back());
	for(size_t i=1;i<oProgress.m_vFractions.size();i++)
		EXPECT_LE(oProgress.m_vFractions[i-1],oProgress.m_vFractions[i]);
	EXPECT_TRUE(oSimulation.Run(NULL))<<"nothing left to do, no callback needed";
}

TEST(MaterialSimulation, RunStopsWhenAsked)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CScene s;
	CMaterialSimulation oSimulation(s.m_oChain,s.m_oTool,s.m_oTrajectory,s.m_oMaterial);
	CRecordingProgress oProgress(1);
	EXPECT_FALSE(oSimulation.Run(&oProgress));
	EXPECT_EQ((size_t)1,oSimulation.GetStepIndex())<<"one step was done before the second question";
	EXPECT_FALSE(oSimulation.IsFinished());
	EXPECT_LT(oProgress.m_vFractions.back(),1.0);
	//and it can go on
	EXPECT_TRUE(oSimulation.Run(NULL));
	EXPECT_TRUE(oSimulation.IsFinished());
}
