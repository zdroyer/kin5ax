// The tool a chain file describes: a shape, its sizes, how far it sticks out of the holder and
// where the point a cutter location programs lies on it. CToolDescription is a value; MakeTool
// turns it into the solids of CMillingTool.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "ToolDescription.h"
#include "MillingToolToroidal.h"
#include <memory>

using namespace kin5ax;

namespace {

//a 16 mm toroidal cutter with a 3 mm corner, 30 of cutting part, 60 out of the holder
CToolDescription Toroidal()
{
	CToolDescription o;
	o.m_nType = MILLING_TOOL_TORUS;
	o.m_dRadius = 0.008;
	o.m_dCornerRadius = 0.003;
	o.m_dCuttingHeight = 0.030;
	o.m_dLength = 0.060;
	o.SetGiven(true);
	return o;
}

CToolDescription BallNose()
{
	CToolDescription o;
	o.m_nType = MILLING_TOOL_BALL;
	o.m_dRadius = 0.008;
	o.m_dCornerRadius = 0.008;			//a ball nose is rounded by its whole radius
	o.m_dCuttingHeight = 0.030;
	o.m_dLength = 0.060;
	o.SetGiven(true);
	return o;
}

} // namespace

//---- the names the file uses

TEST(ToolDescription, TheNamesOfTheShapesAndTheReferencePointsGoBothWays)
{
	MILLING_TOOL_TYPE nType = MILLING_TOOL_CONE;
	ASSERT_TRUE(MillingToolTypeFromName("toroidal",nType));
	EXPECT_EQ(MILLING_TOOL_TORUS,nType);
	EXPECT_EQ(std::string("toroidal"),MillingToolTypeName(MILLING_TOOL_TORUS));
	EXPECT_EQ(std::string("ball_end"),MillingToolTypeName(MILLING_TOOL_BALL));
	EXPECT_EQ(std::string("cylinder"),MillingToolTypeName(MILLING_TOOL_CYLINDER));
	EXPECT_FALSE(MillingToolTypeFromName("barrel",nType))<<"the library has no solid for it";

	TOOL_REFERENCE_POINT nReference = TOOL_REFERENCE_CUSTOM;
	ASSERT_TRUE(ToolReferencePointFromName("tip",nReference));
	EXPECT_EQ(TOOL_REFERENCE_TIP,nReference);
	EXPECT_EQ(std::string("ball_centre"),ToolReferencePointName(TOOL_REFERENCE_BALL_CENTRE));
	EXPECT_FALSE(ToolReferencePointFromName("corner_centre",nReference));
}

//---- what sticks out, and where the programmed point is

TEST(ToolDescription, TheShankIsWhatSticksOutAboveTheCuttingPart)
{
	CToolDescription o = Toroidal();
	EXPECT_DOUBLE_EQ(0.030,o.GetShankLength())<<"60 out of the holder, 30 of them cutting";
	o.m_dLength = o.m_dCuttingHeight;
	EXPECT_DOUBLE_EQ(0.0,o.GetShankLength())<<"a tool that is all cutting part has no shank";
}

TEST(ToolDescription, TheTipIsTheProgrammedPointUnlessTheFileSaysOtherwise)
{
	EXPECT_DOUBLE_EQ(0.0,Toroidal().GetReferencePointZ());
	EXPECT_DOUBLE_EQ(0.0,BallNose().GetReferencePointZ())<<"the tip, even for a ball nose";

	CToolDescription oCentre = BallNose();
	oCentre.m_nReference = TOOL_REFERENCE_BALL_CENTRE;
	EXPECT_DOUBLE_EQ(0.008,oCentre.GetReferencePointZ())<<"the centre of the ball";

	CToolDescription oCustom = Toroidal();
	oCustom.m_nReference = TOOL_REFERENCE_CUSTOM;
	oCustom.m_dReferenceHeight = 0.003;
	EXPECT_DOUBLE_EQ(0.003,oCustom.GetReferencePointZ())<<"the centre of the corner, as a height";
}

TEST(ToolDescription, OnlyABallNoseToolIsProgrammedAtTheCentreOfItsBall)
{
	CToolDescription o = Toroidal();
	o.m_nReference = TOOL_REFERENCE_BALL_CENTRE;
	EXPECT_NE("",o.DimensionProblem());
	EXPECT_EQ(NULL,o.MakeTool(CToolHolder()));
}

TEST(ToolDescription, DimensionsThatAreNotAToolAreRefused)
{
	EXPECT_EQ("no tool is described",CToolDescription().DimensionProblem());
	CToolDescription o = Toroidal();
	EXPECT_EQ("",o.DimensionProblem());
	o.m_dLength = 0.020;
	EXPECT_NE("",o.DimensionProblem())<<"sticking out less than the cutting part is long";
	o = Toroidal();
	o.m_dCornerRadius = 0.009;
	EXPECT_NE("",o.DimensionProblem())<<"a corner wider than the tool";
	o = Toroidal();
	o.m_dCornerRadius = 0.040;
	EXPECT_NE("",o.DimensionProblem())<<"a corner taller than the cutting part";
	o = Toroidal();
	o.m_dRadius = 0.0;
	EXPECT_NE("",o.DimensionProblem())<<"no radius at all";
}

//---- the solids it makes

TEST(ToolDescription, MakesTheShapeItNames)
{
	const CToolHolder oHolder("BT40 ER32",std::vector<CToolHolderSection>(1,CToolHolderSection(0.1,0.03)));
	std::unique_ptr<CMillingTool> pTool(Toroidal().MakeTool(oHolder));
	ASSERT_TRUE(pTool.get()!=NULL);
	EXPECT_EQ(MILLING_TOOL_TORUS,pTool->m_type);
	EXPECT_DOUBLE_EQ(0.060,pTool->GetToolLength())<<"the length that sticks out of the holder";
	EXPECT_DOUBLE_EQ(0.160,pTool->GetAssemblyLength())<<"and the holder above it";

	std::unique_ptr<CMillingTool> pBall(BallNose().MakeTool(oHolder));
	ASSERT_TRUE(pBall.get()!=NULL);
	EXPECT_EQ(MILLING_TOOL_BALL,pBall->m_type);
	EXPECT_DOUBLE_EQ(0.0,pBall->m_dReferencePointZ)<<"programmed at the tip";
}

//---- the shape of a toroidal cutter

TEST(ToroidalCutter, TheBottomEdgeIsRoundedAndTheRestIsACylinder)
{
	//8 mm radius, 3 mm corner: the flat part of the tip reaches to 5 mm off the axis
	const CMillingToolToroidal oTool(0.008,0.003,0.030,0.0);
	EXPECT_DOUBLE_EQ(0.003,oTool.GetCornerRadius());

	EXPECT_EQ(CUT_TYPE_CUT,oTool.IsCut(CR3Point(0.000,0.0,0.000)))<<"the tip itself";
	EXPECT_EQ(CUT_TYPE_CUT,oTool.IsCut(CR3Point(0.004,0.0,0.000)))<<"the flat part of the tip";
	EXPECT_EQ(CUT_TYPE_NOT_CUT,oTool.IsCut(CR3Point(0.007,0.0,0.000)))
		<<"outside the rounding at the height of the tip";
	EXPECT_EQ(CUT_TYPE_CUT,oTool.IsCut(CR3Point(0.0079,0.0,0.003)))<<"the full radius at the corner height";
	EXPECT_EQ(CUT_TYPE_CUT,oTool.IsCut(CR3Point(0.0079,0.0,0.020)))<<"the cylinder above it";
	EXPECT_EQ(CUT_TYPE_NOT_CUT,oTool.IsCut(CR3Point(0.0081,0.0,0.020)))<<"wider than the tool";
	EXPECT_EQ(CUT_TYPE_NOT_CUT,oTool.IsCut(CR3Point(0.000,0.0,0.031)))<<"above the cutting part";
	EXPECT_EQ(CUT_TYPE_NOT_CUT,oTool.IsCut(CR3Point(0.000,0.0,-0.001)))<<"below the tip";

	//the point on the arc: 45 degrees round the corner circle, which is inside by a whisker
	const double dOnArc = 0.003*0.70710678;
	EXPECT_EQ(CUT_TYPE_CUT,oTool.IsCut(CR3Point(0.005+dOnArc*0.99,0.0,0.003-dOnArc*0.99)));
	EXPECT_EQ(CUT_TYPE_NOT_CUT,oTool.IsCut(CR3Point(0.005+dOnArc*1.02,0.0,0.003-dOnArc*1.02)));
}

TEST(ToroidalCutter, AFullCornerIsABallAndNoCornerIsAFlatEnd)
{
	//the two ends of the family, as far as the cut test is concerned
	const CMillingToolToroidal oBall(0.008,0.008,0.030,0.0);
	EXPECT_EQ(CUT_TYPE_NOT_CUT,oBall.IsCut(CR3Point(0.0079,0.0,0.000)))<<"a ball touches the tip at a point";
	EXPECT_EQ(CUT_TYPE_CUT,oBall.IsCut(CR3Point(0.0079,0.0,0.008)))<<"its equator";

	const CMillingToolToroidal oFlat(0.008,0.0,0.030,0.0);
	EXPECT_EQ(CUT_TYPE_CUT,oFlat.IsCut(CR3Point(0.0079,0.0,0.000)))<<"a flat end cuts to its edge";
}
