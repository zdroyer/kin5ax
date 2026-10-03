// CMaterial: the voxel block (material, surface, removed) and the cut of a milling tool.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "Material.h"
#include "MillingToolBall.h"

using namespace kin5ax;

namespace {

//a 0.1 m cube from (-0.05,-0.05,0) in 5 mm voxels: 20 x 20 x 20
CMaterial Cube()
{
	return CMaterial(0.1,0.1,0.1,0.005,-0.05,-0.05,0.0);
}

//the voxel that holds the point (the corner convention of the grid: voxel i begins at start + i * precision)
size_t Index(double d, double dStart, double dPrecision)
{
	return (size_t)floor((d-dStart)/dPrecision+1E-9);
}

bool IsRemoved(const CMaterial &o, double x, double y, double z)
{
	return o.Is(VOXEL_BLACK,Index(x,-0.05,0.005),Index(y,-0.05,0.005),Index(z,0.0,0.005));
}

} // namespace

TEST(Material, BlockDimensionsAndParameters)
{
	CMaterial o = Cube();
	size_t nX, nY, nZ;
	o.GetDimensions(nX,nY,nZ);
	EXPECT_EQ((size_t)20,nX);
	EXPECT_EQ((size_t)20,nY);
	EXPECT_EQ((size_t)20,nZ);
	double dXSize, dYSize, dZSize, dPrecision, dXStart, dYStart, dZStart;
	o.GetParameters(dXSize,dYSize,dZSize,dPrecision,dXStart,dYStart,dZStart);
	EXPECT_DOUBLE_EQ(0.1,dXSize);
	EXPECT_DOUBLE_EQ(0.005,dPrecision);
	EXPECT_DOUBLE_EQ(-0.05,dXStart);
	EXPECT_DOUBLE_EQ(0.0,dZStart);
	EXPECT_DOUBLE_EQ(0.005,o.GetPrecision());

	//a size that is not a multiple of the voxel: one voxel more
	CMaterial o2(0.102,0.1,0.1,0.005,0.0,0.0,0.0);
	o2.GetDimensions(nX,nY,nZ);
	EXPECT_EQ((size_t)21,nX);
}

TEST(Material, AFreshBlockIsFullAndItsSurfaceIsTheShell)
{
	CMaterial o = Cube();
	EXPECT_EQ((size_t)0,o.GetVoxelCount(VOXEL_BLACK));
	EXPECT_EQ((size_t)8000,o.GetVoxelCount(VOXEL_WHITE)+o.GetVoxelCount(VOXEL_GRAY));
	const size_t nShell = 8000-18*18*18;
	EXPECT_EQ(nShell,o.GetVoxelCount(VOXEL_GRAY));
	EXPECT_EQ(nShell,o.GetSurfaceVoxels().size());
	EXPECT_TRUE(o.Is(VOXEL_GRAY,0,0,0));
	EXPECT_TRUE(o.Is(VOXEL_WHITE,10,10,10));
	EXPECT_TRUE(o.Is(VOXEL_BLACK,20,0,0))<<"outside the block counts as removed";
}

TEST(Material, AVerticalBallToolCutsTheBallAndTheShank)
{
	CMaterial o = Cube();
	CMillingToolBall oTool(0.01,0.01,0.05,0.0);			//R = Rz = 10 mm, 50 mm long, reference point on the tip
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.05),CR3Vector(0.0,0.0,1.0));	//tip in the middle of the cube
	o.MachineCut(oTool);
	EXPECT_TRUE(IsRemoved(o,0.0,0.0,0.06))<<"the centre of the ball";
	EXPECT_TRUE(IsRemoved(o,0.0,0.0,0.095))<<"inside the shank, below the end of the tool";
	EXPECT_FALSE(IsRemoved(o,0.0,0.0,0.03))<<"below the tip";
	EXPECT_FALSE(IsRemoved(o,0.03,0.0,0.06))<<"beside the tool";
	EXPECT_FALSE(IsRemoved(o,-0.04,-0.04,0.01))<<"a far corner";
	//the removed volume: half a ball and a 40 mm cylinder of radius 10 mm = 14.7 cm3 = 117 voxels of 125 mm3,
	//blurred by one voxel by the cut convention (the eight voxels around every sampled point)
	size_t nRemoved = o.GetVoxelCount(VOXEL_BLACK);
	EXPECT_GT(nRemoved,(size_t)117);
	EXPECT_LT(nRemoved,(size_t)1000);
	EXPECT_EQ((size_t)8000,nRemoved+o.GetVoxelCount(VOXEL_WHITE)+o.GetVoxelCount(VOXEL_GRAY));
	//the surface now lines the hole as well
	EXPECT_GT(o.GetSurfaceVoxels().size(),(size_t)(8000-18*18*18));
	for(const CR3Point &p : o.GetSurfaceVoxels())
		EXPECT_FALSE(IsRemoved(o,p.GetX(),p.GetY(),p.GetZ()))<<"a surface voxel is material";
}

TEST(Material, ATiltedToolCutsAlongItsAxis)
{
	CMaterial o = Cube();
	CMillingToolBall oTool(0.01,0.01,0.05,0.0);
	oTool.SetPositionAndOrientation(CR3Point(-0.04,0.0,0.05),CR3Vector(1.0,0.0,0.0));	//horizontal, pointing +X
	o.MachineCut(oTool);
	EXPECT_TRUE(IsRemoved(o,-0.03,0.0,0.05))<<"the centre of the ball, 10 mm along the axis";
	EXPECT_TRUE(IsRemoved(o,0.0,0.0,0.05))<<"the shank, 40 mm along the axis";
	EXPECT_FALSE(IsRemoved(o,0.02,0.0,0.05))<<"beyond the end of the tool (60 mm)";
	EXPECT_FALSE(IsRemoved(o,-0.03,0.0,0.08))<<"30 mm beside the axis";
}

TEST(Material, ResetRestoresTheBlock)
{
	CMaterial o = Cube();
	CMillingToolBall oTool(0.01,0.01,0.05,0.0);
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.05),CR3Vector(0.0,0.0,1.0));
	o.MachineCut(oTool);
	ASSERT_GT(o.GetVoxelCount(VOXEL_BLACK),(size_t)0);
	o.ResetMaterial();
	EXPECT_EQ((size_t)0,o.GetVoxelCount(VOXEL_BLACK));
	EXPECT_EQ((size_t)(8000-18*18*18),o.GetVoxelCount(VOXEL_GRAY));
}

TEST(Material, GenerateBoxReplacesTheBlock)
{
	CMaterial o = Cube();
	o.GenerateBox(0.05,0.05,0.05,0.01,0.0,0.0,0.0);
	size_t nX, nY, nZ;
	o.GetDimensions(nX,nY,nZ);
	EXPECT_EQ((size_t)5,nX);
	EXPECT_EQ((size_t)125,o.GetVoxelCount(VOXEL_WHITE)+o.GetVoxelCount(VOXEL_GRAY));
	EXPECT_EQ((size_t)125-27,o.GetSurfaceVoxels().size());
}

TEST(Material, VolumesAreWholeVoxels)
{
	CMaterial o = Cube();
	EXPECT_DOUBLE_EQ(1.25E-7,o.GetVoxelVolume());
	EXPECT_DOUBLE_EQ(1E-3,o.GetTotalVolume());
	EXPECT_DOUBLE_EQ(0.0,o.GetRemovedVolume());
	CMillingToolBall oTool(0.01,0.01,0.05,0.0);
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.05),CR3Vector(0.0,0.0,1.0));
	o.MachineCut(oTool);
	EXPECT_DOUBLE_EQ((double)o.GetVoxelCount(VOXEL_BLACK)*1.25E-7,o.GetRemovedVolume());
	EXPECT_GT(o.GetRemovedVolume(),117*1.25E-7)<<"half a ball and a 40 mm cylinder of radius 10 mm, blurred by a voxel";
	EXPECT_LT(o.GetRemovedVolume(),0.3*o.GetTotalVolume());
}
