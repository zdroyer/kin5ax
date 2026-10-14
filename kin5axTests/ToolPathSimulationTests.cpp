// The cutting simulation with no machine: a path of programmed points and tool axes, made dense, and the
// tool stamped into the material at each of them. The facts a voxel block cannot argue with: a ball
// dragged along the top of a block with its centre on that top leaves a groove one radius deep under
// its path and nothing beside it - whichever way the shank leans - and a shank pushed into the block is
// a gouge.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "ToolPathSimulation.h"
#include "MillingToolBall.h"
#include "ArgsMyException.h"
#include <cmath>

using namespace kin5ax;

namespace {

const double dRADIUS = 0.016;

//a 16 mm ball nose with a cutting part this high, a shank and a holder
CMillingToolBall Tool(double dCutting = 0.064)
{
	return CMillingToolBall(dRADIUS,dRADIUS,dCutting,dRADIUS,0.030,CToolHolder(0.050,0.030));
}

//a block 0.12 x 0.06, 0.03 high, its top at z = 0, in 2 mm voxels
CMaterial Block()
{
	return CMaterial(0.12,0.06,0.03,0.002,-0.06,-0.03,-0.03);
}

CPostprocessorSettings Dense(double dStep)
{
	CPostprocessorSettings o;
	o.m_bInterpolateBetweenLocations = true;
	o.m_dMaxPositionDifference = dStep;
	o.m_dMaxOrientationDifferenceDegrees = 1.0;
	o.m_dMaxOrientationDifferenceRadians = atan(1.0)/45.0;
	return o;
}

//the top of what is left along a line of constant y, and beside it
void TopOf(CMaterial &oBlock, double dY, double &dTop)
{
	dTop = -1.0;
	const std::vector<CR3Point> &v = oBlock.GetSurfaceVoxels();
	for(size_t i=0;i<v.size();i++)
		if(fabs(v[i].GetX())<=0.03 && fabs(v[i].GetY()-dY)<0.0015)
			dTop = std::max(dTop,v[i].GetZ());
}

} // namespace

TEST(ToolPathSimulation, ALeaningBallCutsAGrooveOneRadiusDeepUnderItsPath)
{
	CMillingToolBall oTool = Tool();
	CMaterial oBlock = Block();
	const double dLean = 30.0*atan(1.0)/45.0;
	const CR3Vector v(sin(dLean),0.0,cos(dLean));
	std::vector<CR3AnchoredVector> vPath;
	vPath.push_back(CR3AnchoredVector(v,CR3Point(-0.04,0.0,0.0)));
	vPath.push_back(CR3AnchoredVector(v,CR3Point(0.04,0.0,0.0)));
	CToolPathSimulation oSimulation(oTool,vPath,Dense(0.002),oBlock);
	EXPECT_EQ((size_t)41,oSimulation.GetStepCount())<<"80 mm in 2 mm steps, and the first location";
	EXPECT_EQ((size_t)0,oSimulation.GetStepIndex());
	EXPECT_FALSE(oSimulation.IsFinished());
	EXPECT_TRUE(oSimulation.Run(NULL));
	EXPECT_TRUE(oSimulation.IsFinished());
	EXPECT_EQ(oSimulation.GetStepCount(),oSimulation.GetStepIndex());
	EXPECT_FALSE(oSimulation.Step())<<"nothing past the end";
	EXPECT_GT(oBlock.GetRemovedVolume(),0.0);

	double dUnder=0.0, dBeside=0.0;
	TopOf(oBlock,0.0,dUnder);
	TopOf(oBlock,0.025,dBeside);
	EXPECT_LT(dUnder,-0.5*dRADIUS)<<"a groove under the path";
	EXPECT_GT(dUnder,-dRADIUS-0.004)<<"one radius deep, lean or no lean";
	EXPECT_GT(dBeside,-0.004)<<"and nothing taken off beside it";
	EXPECT_FALSE(oSimulation.HasGouged())<<"the shank leans away above the block";
}

TEST(ToolPathSimulation, AShankInTheBlockIsAGougeAndTheFirstIsRemembered)
{
	//a cutting part of 20 mm, just taller than the ball: the shank starts 20 mm above the tip, and a
	//ball pushed 25 mm into the block has it in the material that is still there
	CMillingToolBall oTool = Tool(0.020);
	CMaterial oBlock = Block();
	std::vector<CR3AnchoredVector> vPath;
	vPath.push_back(CR3AnchoredVector(CR3Vector(0,0,1),CR3Point(-0.05,0.0,0.0)));
	vPath.push_back(CR3AnchoredVector(CR3Vector(0,0,1),CR3Point(0.0,0.0,-0.025)));
	CToolPathSimulation oSimulation(oTool,vPath,Dense(0.005),oBlock);
	EXPECT_TRUE(oSimulation.Step());
	EXPECT_FALSE(oSimulation.HasGouged())<<"the first location is on the top";
	oSimulation.Run(NULL);
	EXPECT_TRUE(oSimulation.HasGouged());
	EXPECT_GT(oSimulation.GetGougeCount(),(size_t)0);
	EXPECT_GT(oSimulation.GetFirstGougeStep(),(size_t)0);
	EXPECT_LT(oSimulation.GetFirstGougeStep(),oSimulation.GetStepCount());
	EXPECT_GT(oSimulation.GetFirstGougePoint().GetZ(),-0.03)<<"inside the block";
	EXPECT_LT(oSimulation.GetFirstGougePoint().GetZ(),0.001);

	//a reset forgets the gouges and starts again; the block stays as it is
	const double dRemoved = oBlock.GetRemovedVolume();
	oSimulation.Reset();
	EXPECT_FALSE(oSimulation.HasGouged());
	EXPECT_EQ((size_t)0,oSimulation.GetStepIndex());
	EXPECT_EQ(dRemoved,oBlock.GetRemovedVolume());
}

TEST(ToolPathSimulation, TheDensePathIsThereToBeRead)
{
	CMillingToolBall oTool = Tool();
	CMaterial oBlock = Block();
	std::vector<CR3AnchoredVector> vPath;
	vPath.push_back(CR3AnchoredVector(CR3Vector(0,0,1),CR3Point(0.0,0.0,0.0)));
	vPath.push_back(CR3AnchoredVector(CR3Vector(0,0,1),CR3Point(0.03,0.0,0.0)));
	CToolPathSimulation oSimulation(oTool,vPath,Dense(0.01),oBlock);
	const std::vector<CDenseLocation> &v = oSimulation.GetLocations();
	ASSERT_EQ((size_t)4,v.size());
	EXPECT_EQ((size_t)0,v[1].m_nSource);
	EXPECT_TRUE(v[1].m_bInvented);
	EXPECT_EQ((size_t)1,v[3].m_nSource);
	EXPECT_FALSE(v[3].m_bInvented);
}

TEST(ToolPathSimulation, AnAxisOfNoLengthIsRefused)
{
	CMillingToolBall oTool = Tool();
	CMaterial oBlock = Block();
	std::vector<CR3AnchoredVector> vPath;
	vPath.push_back(CR3AnchoredVector(CR3Vector(0,0,1),CR3Point(0.0,0.0,0.0)));
	vPath.push_back(CR3AnchoredVector(CR3Vector(0,0,0),CR3Point(0.03,0.0,0.0)));
	EXPECT_THROW(CToolPathSimulation(oTool,vPath,Dense(0.01),oBlock),CArgsMyException);
}
