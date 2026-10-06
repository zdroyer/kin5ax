// A gouge: the shank or the holder of the tool meets material that is still there. The cutting part may remove
// material, those two may not touch it. Only the simulation can tell, because it knows what is left.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SolverRoundTrip.h"
#include "MaterialSimulation.h"
#include "MillingToolBall.h"
#include "MillingToolCylinder.h"

using namespace kin5ax;

namespace {

const double dRADIUS = 0.016;
const double dCUTTING = 0.064;
const double dSHANK = 0.030;
const double dHOLDER_LENGTH = 0.050;
const double dHOLDER_RADIUS = 0.030;

CMillingToolBall Assembly()
{
	return CMillingToolBall(dRADIUS,dRADIUS,dCUTTING,dRADIUS,dSHANK,CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
}

//a block of the given height from z = 0, 0.3 wide, in 2 mm voxels
CMaterial Block(double dHeight)
{
	return CMaterial(0.3,0.3,dHeight,0.002,-0.15,-0.15,0.0);
}

} // namespace

TEST(MaterialGouge, TheHolderInsideTheBlockIsAGouge)
{
	//the tool stands upright with its ball centre at the bottom of the block: the shank runs from 0.048 to
	//0.078 above it and the holder from 0.078 to 0.128, so a block 0.2 high holds both
	CMaterial oTall = Block(0.2);
	CMillingToolBall oTool = Assembly();
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.0),CR3Vector(0.0,0.0,1.0));
	CR3Point r3pFirst;
	ASSERT_TRUE(oTall.HasMaterialInNonCuttingPart(oTool,r3pFirst));
	EXPECT_GT(r3pFirst.GetZ(),dCUTTING-dRADIUS-0.002)<<"the voxel is above the cutting part";
	EXPECT_LT(r3pFirst.GetZ(),dCUTTING+dSHANK+dHOLDER_LENGTH-dRADIUS+0.002);

	//a block only 0.04 high ends below the shank: nothing of the envelope touches it
	CMaterial oFlat = Block(0.04);
	EXPECT_FALSE(oFlat.HasMaterialInNonCuttingPart(oTool));

	//and a tool without a shank and a holder has no envelope to gouge with
	CMillingToolBall oBare(dRADIUS,dRADIUS,dCUTTING,dRADIUS);
	oBare.SetPositionAndOrientation(CR3Point(0.0,0.0,0.0),CR3Vector(0.0,0.0,1.0));
	EXPECT_FALSE(oTall.HasMaterialInNonCuttingPart(oBare));
}

TEST(MaterialGouge, RemovedMaterialDoesNotGouge)
{
	CMaterial oMaterial = Block(0.2);
	CMillingToolBall oTool = Assembly();
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.0),CR3Vector(0.0,0.0,1.0));
	ASSERT_TRUE(oMaterial.HasMaterialInNonCuttingPart(oTool));

	//a wide tool clears the whole column the assembly stands in, then nothing is left to gouge
	CMillingToolCylinder oWide(2.0*dHOLDER_RADIUS,0.0,0.3,0.0);
	oWide.SetPositionAndOrientation(CR3Point(0.0,0.0,0.0),CR3Vector(0.0,0.0,1.0));
	oMaterial.MachineCut(oWide);
	EXPECT_FALSE(oMaterial.HasMaterialInNonCuttingPart(oTool))<<"the column is gone";
}

TEST(MaterialGouge, TheEnvelopeFollowsTheToolAxis)
{
	CMaterial oMaterial = Block(0.04);		//low: nothing above the cutting part when the tool stands upright
	CMillingToolBall oTool = Assembly();
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.02),CR3Vector(0.0,0.0,1.0));
	EXPECT_FALSE(oMaterial.HasMaterialInNonCuttingPart(oTool))<<"upright, the holder is above the block";
	//laid over: the shank and the holder sweep through the block
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.02),CR3Vector(1.0,0.0,0.0));
	EXPECT_TRUE(oMaterial.HasMaterialInNonCuttingPart(oTool))<<"horizontal, both lie in the block";
}

// ---------------------------------------------------------------- through the simulation

TEST(MaterialGouge, TheSimulationCountsTheGouges)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	CMillingToolBall oTool = Assembly();
	oChain.SetDefaultToolHolder(CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS));
	oChain.SetToolAssembly(oTool);

	//three locations of the same pose: a block around the tool point, tall enough to hold the holder
	std::mt19937 oGenerator(23);
	CChainConfiguration oSet = RandomJointValues(oChain,oGenerator);
	oChain.SetParameters(oSet);
	CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();
	CMaterial oMaterial(0.4,0.4,0.4,0.004,cv.m_p.GetX()-0.2,cv.m_p.GetY()-0.2,cv.m_p.GetZ()-0.2);
	CSimulationData oTrajectory(10);
	tChainConfigurationVector v(1,oSet);
	oTrajectory.AddConfigurationSet(v);
	oTrajectory.AddConfigurationSet(v);
	oTrajectory.AddConfigurationSet(v);

	CMaterialSimulation oSimulation(oChain,oTool,oTrajectory,oMaterial);
	EXPECT_FALSE(oSimulation.HasGouged());
	EXPECT_EQ((size_t)0,oSimulation.GetGougeCount());
	EXPECT_TRUE(oSimulation.Run(NULL));
	EXPECT_TRUE(oSimulation.HasGouged())<<"the tool point sits in the middle of the block, the holder inside it";
	EXPECT_EQ((size_t)3,oSimulation.GetGougeCount())<<"every location of the trajectory";
	EXPECT_EQ((size_t)0,oSimulation.GetFirstGougeStep());
	EXPECT_TRUE(oMaterial.GetRemovedVolume()>0.0)<<"and it did cut";

	//Reset forgets the gouges
	oSimulation.Reset();
	EXPECT_FALSE(oSimulation.HasGouged());
}

TEST(MaterialGouge, ABareToolInTheSameSceneDoesNotGouge)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	CMillingToolBall oBare(dRADIUS,dRADIUS,dCUTTING,dRADIUS);
	oChain.SetToolAssembly(oBare);
	std::mt19937 oGenerator(23);
	CChainConfiguration oSet = RandomJointValues(oChain,oGenerator);
	oChain.SetParameters(oSet);
	CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();
	CMaterial oMaterial(0.4,0.4,0.4,0.004,cv.m_p.GetX()-0.2,cv.m_p.GetY()-0.2,cv.m_p.GetZ()-0.2);
	CSimulationData oTrajectory(10);
	tChainConfigurationVector v(1,oSet);
	oTrajectory.AddConfigurationSet(v);
	CMaterialSimulation oSimulation(oChain,oBare,oTrajectory,oMaterial);
	EXPECT_TRUE(oSimulation.Run(NULL));
	EXPECT_FALSE(oSimulation.HasGouged())<<"no shank, no holder, nothing to gouge with";
}
