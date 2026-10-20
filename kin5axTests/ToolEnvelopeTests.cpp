// The tool assembly: the cutting part, the shank above it and the holder above that (CToolHolder), the lengths
// they add up to, and the collision of the two non-cutting solids with a box of a machine part.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "MillingToolBall.h"
#include "MillingToolCylinder.h"
#include "Translation.h"

using namespace kin5ax;

namespace {

const double dRADIUS = 0.016;			//the ball and the shank of the default tool of the application
const double dCUTTING = 0.064;			//its cutting length
const double dSHANK = 0.030;			//the non-cutting part above the cutting part
const double dHOLDER_LENGTH = 0.050;
const double dHOLDER_RADIUS = 0.030;

CToolHolder Holder()
{
	return CToolHolder(dHOLDER_LENGTH,dHOLDER_RADIUS);
}

//a ball nose tool with a shank and a holder, placed with its reference point at the origin and the axis along OZ
CMillingToolBall BallAssembly(double dReferencePointZ = dRADIUS)
{
	CMillingToolBall oTool(dRADIUS,dRADIUS,dCUTTING,dReferencePointZ,dSHANK,Holder());
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.0),CR3Vector(0.0,0.0,1.0));
	return oTool;
}

//a point of the tip frame as a point of the world frame, for a tool placed as above
CR3Point InTipFrame(double dZ, double dRadial, double dReferencePointZ = dRADIUS)
{
	return CR3Point(dRadial,0.0,dZ-dReferencePointZ);
}

} // namespace

TEST(ToolHolder, LengthAndRadiusAreNeverNegative)
{
	CToolHolder o(-0.05,-0.03);
	EXPECT_DOUBLE_EQ(0.05,o.GetLength());
	EXPECT_DOUBLE_EQ(0.03,o.GetRadius());
	EXPECT_TRUE(o.HasBody());
	EXPECT_TRUE(o==CToolHolder(0.05,0.03));
	EXPECT_TRUE(o!=CToolHolder());

	CToolHolder oNone;
	EXPECT_DOUBLE_EQ(0.0,oNone.GetLength());
	EXPECT_FALSE(oNone.HasBody())<<"no holder at all";
	EXPECT_FALSE(CToolHolder(0.05,0.0).HasBody())<<"a distance to the tool, but no solid";
	CToolHolder oSet;
	oSet.SetLength(0.1);
	oSet.SetRadius(0.02);
	EXPECT_TRUE(oSet==CToolHolder(0.1,0.02));
}

TEST(ToolAssembly, LengthsAddUp)
{
	CMillingToolBall oBall(dRADIUS,dRADIUS,dCUTTING,dRADIUS,dSHANK,Holder());
	EXPECT_DOUBLE_EQ(dCUTTING+dSHANK,oBall.GetToolLength());
	EXPECT_DOUBLE_EQ(dCUTTING+dSHANK+dHOLDER_LENGTH,oBall.GetAssemblyLength());
	EXPECT_DOUBLE_EQ(dCUTTING+dSHANK+dHOLDER_LENGTH-dRADIUS,oBall.GetToolOffset())
		<<"the ball centre is the programmed point, one ball radius above the tip";
	EXPECT_DOUBLE_EQ(dRADIUS,oBall.DefaultZeroPointZ())<<"a ball nose tool is programmed at the centre of its ball";
	EXPECT_TRUE(oBall.HasNonCuttingPart());

	CMillingToolCylinder oFlat(dRADIUS,0.0,dCUTTING,0.0,dSHANK,Holder());
	EXPECT_DOUBLE_EQ(0.0,oFlat.DefaultZeroPointZ())<<"a flat end mill is programmed at its tip";
	EXPECT_DOUBLE_EQ(dCUTTING+dSHANK+dHOLDER_LENGTH,oFlat.GetToolOffset());

	//the tool of the application before the holder existed: no shank, no holder, nothing changes
	CMillingToolBall oBare(dRADIUS,dRADIUS,dCUTTING,dRADIUS);
	EXPECT_DOUBLE_EQ(dCUTTING,oBare.GetToolLength());
	EXPECT_DOUBLE_EQ(dCUTTING,oBare.GetAssemblyLength());
	EXPECT_DOUBLE_EQ(dCUTTING-dRADIUS,oBare.GetToolOffset());
	EXPECT_FALSE(oBare.HasNonCuttingPart());
	EXPECT_TRUE(oBare.GetNonCuttingPartBoundingBox().IsEmpty())<<"no solid, no box";
}

TEST(ToolAssembly, TheNonCuttingEnvelopeIsTheShankAndTheHolder)
{
	CMillingToolBall oTool = BallAssembly();
	const double dTool = dCUTTING+dSHANK;
	//the shank: from the end of the cutting part up to the end of the tool, the radius of the shank
	EXPECT_TRUE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,dCUTTING+0.5*dSHANK)));
	EXPECT_TRUE(oTool.IsInOZNonCuttingSpace(CR3Point(0.9*dRADIUS,0.0,dCUTTING+0.5*dSHANK)));
	EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(CR3Point(1.1*dRADIUS,0.0,dCUTTING+0.5*dSHANK)))<<"beside the shank";
	//the holder: wider, from the end of the tool up to the nose
	EXPECT_TRUE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,dTool+0.5*dHOLDER_LENGTH)));
	EXPECT_TRUE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.9*dHOLDER_RADIUS,dTool+0.5*dHOLDER_LENGTH)));
	EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,1.1*dHOLDER_RADIUS,dTool+0.5*dHOLDER_LENGTH)))<<"beside the holder";
	//neither below the cutting part nor above the nose
	EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,0.5*dCUTTING)))<<"the cutting part is not part of it";
	EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,-0.01)))<<"below the tip";
	EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(CR3Point(0.0,0.0,dTool+dHOLDER_LENGTH+0.01)))<<"above the nose";
	//the cutting part and the non-cutting envelope do not overlap
	for(int i=1;i<10;i++){
		CR3Point p(0.0,0.0,0.1*i*dCUTTING);
		EXPECT_FALSE(oTool.IsInOZNonCuttingSpace(p))<<"z = "<<p.GetZ();
	}
}

TEST(ToolAssembly, TheEnvelopeFollowsThePlacementOfTheTool)
{
	CMillingToolBall oTool = BallAssembly();
	//placed at the origin with the axis along OZ: the tip frame is the world frame lowered by the reference point
	EXPECT_TRUE(oTool.IsInNonCuttingPart(InTipFrame(dCUTTING+0.5*dSHANK,0.0)));
	EXPECT_FALSE(oTool.IsInNonCuttingPart(InTipFrame(0.5*dCUTTING,0.0)));
	EXPECT_TRUE(is_point_cut(oTool.IsCut(InTipFrame(0.5*dCUTTING,0.0))))<<"and the cut volume is unchanged";

	//the same assembly lying along +X: the solids follow the axis
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.0),CR3Vector(1.0,0.0,0.0));
	EXPECT_TRUE(oTool.IsInNonCuttingPart(CR3Point(dCUTTING+0.5*dSHANK-dRADIUS,0.0,0.0)));
	EXPECT_FALSE(oTool.IsInNonCuttingPart(CR3Point(0.0,0.0,dCUTTING+0.5*dSHANK-dRADIUS)))<<"not along OZ any more";

	//the bounding box holds the shank and the holder and nothing of the cutting part below the tip
	oTool.SetPositionAndOrientation(CR3Point(0.0,0.0,0.0),CR3Vector(0.0,0.0,1.0));
	CR3Box oBox = oTool.GetNonCuttingPartBoundingBox();
	EXPECT_FALSE(oBox.IsEmpty());
	EXPECT_TRUE(oBox.Contains(InTipFrame(dCUTTING+0.5*dSHANK,0.0)));
	EXPECT_TRUE(oBox.Contains(InTipFrame(dCUTTING+dSHANK+0.5*dHOLDER_LENGTH,0.9*dHOLDER_RADIUS)));
	EXPECT_FALSE(oBox.Contains(InTipFrame(0.5*dCUTTING,0.0)))<<"the cutting part is outside the box";
}

// ---------------------------------------------------------------- collisions with a machine part

//the boxes are given in the tip frame, so the box frame is the tool frame lifted by the reference point
class ToolAssemblyCollisionTest : public ::testing::TestWithParam<double>	//the reference point height
{
protected:
	bool NonCuttingCollides(const CMillingTool &oTool, const CR3Box &oBoxInTipFrame) const
	{
		CTranslation oToolToTip(0.0,0.0,GetParam()), oTipToTool(0.0,0.0,-GetParam());
		return oTool.CauseNonCuttingCollision(oBoxInTipFrame,oToolToTip,oTipToTool);
	}
	bool CuttingCollides(const CMillingTool &oTool, const CR3Box &oBoxInTipFrame) const
	{
		CTranslation oToolToTip(0.0,0.0,GetParam()), oTipToTool(0.0,0.0,-GetParam());
		return oTool.CauseCollision(oBoxInTipFrame,oToolToTip,oTipToTool);
	}
	//a small box around the point (dRadial, 0, dZ) of the tip frame
	static CR3Box BoxAt(double dZ, double dRadial)
	{
		const double d = 0.02*dRADIUS;
		return CR3Box(dRadial-d,dRadial+d,-d,d,dZ-d,dZ+d);
	}
};

TEST_P(ToolAssemblyCollisionTest, TheShankAndTheHolderCollide)
{
	CMillingToolBall oTool(dRADIUS,dRADIUS,dCUTTING,GetParam(),dSHANK,Holder());
	const double dTool = dCUTTING+dSHANK;
	EXPECT_TRUE(NonCuttingCollides(oTool,BoxAt(dCUTTING+0.5*dSHANK,0.0)))<<"the shank";
	EXPECT_TRUE(NonCuttingCollides(oTool,BoxAt(dCUTTING+0.5*dSHANK,0.9*dRADIUS)))<<"the side of the shank";
	EXPECT_TRUE(NonCuttingCollides(oTool,BoxAt(dTool+0.5*dHOLDER_LENGTH,0.0)))<<"the holder";
	EXPECT_TRUE(NonCuttingCollides(oTool,BoxAt(dTool+0.5*dHOLDER_LENGTH,0.9*dHOLDER_RADIUS)))<<"the side of the holder";
	EXPECT_FALSE(NonCuttingCollides(oTool,BoxAt(dCUTTING+0.5*dSHANK,1.5*dRADIUS)))<<"beside the shank, inside the holder radius";
	EXPECT_FALSE(NonCuttingCollides(oTool,BoxAt(0.5*dCUTTING,0.0)))<<"the cutting part is not the business of this test";
	EXPECT_FALSE(NonCuttingCollides(oTool,BoxAt(dTool+dHOLDER_LENGTH+0.1*dHOLDER_LENGTH,0.0)))<<"above the nose";
	//the cutting part keeps its own test
	EXPECT_TRUE(CuttingCollides(oTool,BoxAt(0.5*dCUTTING,0.0)));
	EXPECT_FALSE(CuttingCollides(oTool,BoxAt(dTool+0.5*dHOLDER_LENGTH,0.0)))<<"the holder is not the cutting part";
}

TEST_P(ToolAssemblyCollisionTest, WithoutAShankAndAHolderNothingCollides)
{
	CMillingToolBall oTool(dRADIUS,dRADIUS,dCUTTING,GetParam());
	EXPECT_FALSE(NonCuttingCollides(oTool,BoxAt(0.5*dCUTTING,0.0)));
	EXPECT_FALSE(NonCuttingCollides(oTool,BoxAt(dCUTTING+0.01,0.0)));
	EXPECT_FALSE(NonCuttingCollides(oTool,BoxAt(dCUTTING+1.0,0.0)));
	//a holder with a length but no radius keeps the tool away from the nose and has no body
	CMillingToolBall oNoBody(dRADIUS,dRADIUS,dCUTTING,GetParam(),0.0,CToolHolder(dHOLDER_LENGTH,0.0));
	EXPECT_DOUBLE_EQ(dCUTTING+dHOLDER_LENGTH,oNoBody.GetAssemblyLength());
	EXPECT_FALSE(oNoBody.HasNonCuttingPart());
	EXPECT_FALSE(NonCuttingCollides(oNoBody,BoxAt(dCUTTING+0.5*dHOLDER_LENGTH,0.0)));
}

TEST_P(ToolAssemblyCollisionTest, AFlatEndMillHasTheSameEnvelope)
{
	CMillingToolCylinder oTool(dRADIUS,0.0,dCUTTING,GetParam(),dSHANK,Holder());
	EXPECT_TRUE(NonCuttingCollides(oTool,BoxAt(dCUTTING+0.5*dSHANK,0.0)))<<"the shank";
	EXPECT_TRUE(NonCuttingCollides(oTool,BoxAt(dCUTTING+dSHANK+0.5*dHOLDER_LENGTH,0.0)))<<"the holder";
	EXPECT_FALSE(NonCuttingCollides(oTool,BoxAt(0.5*dCUTTING,0.0)));
	EXPECT_TRUE(CuttingCollides(oTool,BoxAt(0.5*dCUTTING,0.0)));
}

INSTANTIATE_TEST_CASE_P(ReferencePoints, ToolAssemblyCollisionTest, ::testing::Values(0.0,dRADIUS,0.04));
