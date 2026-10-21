// CToolAssembly: a holder with the tool installed in it, and the two lengths a machine needs.
// The nose of the spindle is one end; the tip of the tool and the point a cutter location programs
// are the other two. An empty assembly - NO_HOLDER with NO_TOOL - is nothing in the spindle, which a
// postprocessor takes like any other.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "ToolAssembly.h"
#include "5CPostprocessor.h"
#include <memory>

using namespace kin5ax;

namespace {

CToolHolder Holder()
{
	std::vector<CToolHolderSection> v;
	v.push_back(CToolHolderSection(0.050,0.040));
	v.push_back(CToolHolderSection(0.025,0.027));
	v.push_back(CToolHolderSection(0.040,0.025));
	return CToolHolder("HSK63A ER32",v);		//0.115 long
}

CToolDescription BallNose()
{
	CToolDescription o;
	o.m_nType = MILLING_TOOL_BALL;
	o.m_dRadius = 0.008;
	o.m_dCornerRadius = 0.008;
	o.m_dCuttingHeight = 0.030;
	o.m_dLength = 0.060;						//what sticks out of the holder
	o.SetGiven(true);
	return o;
}

} // namespace

TEST(ToolAssembly, TheLengthIsTheHolderAndWhatSticksOutOfIt)
{
	const CToolAssembly oAssembly(Holder(),BallNose());
	EXPECT_DOUBLE_EQ(0.115,oAssembly.GetHolderLength());
	EXPECT_DOUBLE_EQ(0.175,oAssembly.GetLength())<<"the nose to the tip";
	EXPECT_DOUBLE_EQ(0.175,oAssembly.GetToolOffset())<<"programmed at the tip, so the same";
	EXPECT_TRUE(oAssembly.HasBody());
	EXPECT_FALSE(oAssembly.IsEmpty());
	EXPECT_EQ("",oAssembly.Problem());
}

TEST(ToolAssembly, TheOffsetFollowsTheProgrammedPointUpTheTool)
{
	CToolDescription oTool = BallNose();
	oTool.m_nReference = TOOL_REFERENCE_BALL_CENTRE;
	const CToolAssembly oAssembly(Holder(),oTool);
	EXPECT_DOUBLE_EQ(0.175,oAssembly.GetLength())<<"the tip has not moved";
	EXPECT_DOUBLE_EQ(0.175-0.008,oAssembly.GetToolOffset())<<"the ball centre is a radius higher";
}

TEST(ToolAssembly, AnEmptyOneIsNothingInTheSpindle)
{
	const CToolAssembly oEmpty = CToolAssembly::Empty();
	EXPECT_TRUE(oEmpty.IsEmpty());
	EXPECT_FALSE(oEmpty.HasBody());
	EXPECT_DOUBLE_EQ(0.0,oEmpty.GetLength());
	EXPECT_DOUBLE_EQ(0.0,oEmpty.GetToolOffset());
	EXPECT_EQ("",oEmpty.Problem())<<"nothing in the spindle is a perfectly good answer";
	EXPECT_EQ(std::string(szNO_HOLDER),oEmpty.GetHolder().GetModel());
	EXPECT_FALSE(oEmpty.GetTool().IsGiven());
	EXPECT_EQ(NULL,oEmpty.MakeTool())<<"and there are no solids to make";
}

TEST(ToolAssembly, AHolderWithNoToolIsAMachineBetweenJobs)
{
	const CToolAssembly oAssembly(Holder(),NoTool());
	EXPECT_FALSE(oAssembly.IsEmpty());
	EXPECT_DOUBLE_EQ(0.115,oAssembly.GetLength())<<"the holder alone";
	EXPECT_EQ("",oAssembly.Problem());
	EXPECT_EQ(NULL,oAssembly.MakeTool())<<"no tool, so no tool solids";
}

TEST(ToolAssembly, MakesTheSolidsOfTheToolInItsHolder)
{
	const CToolAssembly oAssembly(Holder(),BallNose());
	std::unique_ptr<CMillingTool> pTool(oAssembly.MakeTool());
	ASSERT_TRUE(pTool.get()!=NULL);
	EXPECT_EQ(MILLING_TOOL_BALL,pTool->m_type);
	EXPECT_DOUBLE_EQ(0.060,pTool->GetToolLength());
	EXPECT_DOUBLE_EQ(0.175,pTool->GetAssemblyLength())<<"the same length the assembly says";
	EXPECT_DOUBLE_EQ(oAssembly.GetToolOffset(),pTool->GetToolOffset());
}

TEST(ToolAssembly, DimensionsThatAreNotAToolAreTheAssemblysProblem)
{
	CToolDescription oTool = BallNose();
	oTool.m_dLength = 0.010;					//less than the cutting part is long
	const CToolAssembly oAssembly(Holder(),oTool);
	EXPECT_NE("",oAssembly.Problem());
}

//---- the chain hands over what its file describes

TEST(ToolAssembly, AChainGivesTheAssemblyItsFileDescribes)
{
	CChain oChain = LoadChain(GetChainFiles()[0]);
	const CToolAssembly oFromFile = oChain.GetDefaultToolAssembly();
	//every shipped machine names a holder and a 12 mm end mill
	EXPECT_TRUE(oChain.HasDefaultToolHolder());
	EXPECT_TRUE(oChain.HasDefaultTool());
	EXPECT_FALSE(oFromFile.IsEmpty())<<"the holder of the file is in it";
	EXPECT_DOUBLE_EQ(oChain.GetDefaultToolHolder().GetLength()+oChain.GetDefaultTool().m_dLength
		,oFromFile.GetLength())<<"the holder, and what sticks out of it";

	//a chain that describes neither gives an empty assembly
	CChain oPlain;
	EXPECT_TRUE(oPlain.GetDefaultToolAssembly().IsEmpty());
}

//---- the postprocessor takes one or the other, and never assumes

TEST(ToolAssembly, TheOffsetAloneSolvesTheGeometry)
{
	CChain oChain = LoadChain(GetChainFiles()[0]);
	ASSERT_TRUE(oChain.Is5CChain());
	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;
	C5CPostprocessor oPostprocessor(oChain,oSettings);

	//a pose of the machine with a 144 mm assembly, and the location it puts the tool on
	oChain.SetToolOffset(0.144);
	oChain.SetParameters(CChainConfiguration((int)oChain.GetNumberOfRegularLinks()));
	const CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();

	CChain oFresh = LoadChain(GetChainFiles()[0]);
	ASSERT_FALSE(oFresh.IsToolOffsetGiven());
	std::string sLog;
	CSolveReport oReport;
	const tChainConfigurationVector v = oPostprocessor.Solve(cv.m_p,cv.m_v,oFresh,0.144,sLog,oReport);
	EXPECT_TRUE(oFresh.IsToolOffsetGiven())<<"the call said it, so the chain knows it";
	EXPECT_DOUBLE_EQ(0.144,oFresh.GetToolOffset());
	EXPECT_FALSE(v.empty())<<sLog;
}

TEST(ToolAssembly, TheOffsetAloneCannotCheckCollisions)
{
	CChain oChain = LoadChain(GetChainFiles()[0]);
	C5CPostprocessor oPostprocessor(oChain,CPostprocessorSettings());	//checks collisions
	std::string sLog;
	CSolveReport oReport;
	std::string sError;
	try{
		oPostprocessor.Solve(CR3Point(0.0,0.0,0.05),CR3Vector(0.0,0.0,1.0),oChain,0.144,sLog,oReport);
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	EXPECT_NE(std::string::npos,sError.find("tool assembly"))<<sError;
}

TEST(ToolAssembly, TheWholeAssemblyBringsItsOwnOffsetAndItsSolids)
{
	CChain oChain = LoadChain(GetChainFiles()[0]);
	ASSERT_TRUE(oChain.Is5CChain());
	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;		//the shipped boxes sit in the part area (KnownBugs #67)
	C5CPostprocessor oPostprocessor(oChain,oSettings);
	const CToolAssembly oAssembly(Holder(),BallNose());

	oChain.SetToolOffset(oAssembly.GetToolOffset());
	oChain.SetParameters(CChainConfiguration((int)oChain.GetNumberOfRegularLinks()));
	const CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();

	CChain oFresh = LoadChain(GetChainFiles()[0]);
	std::string sLog;
	CSolveReport oReport;
	const tChainConfigurationVector v = oPostprocessor.Solve(cv.m_p,cv.m_v,oFresh,oAssembly,sLog,oReport);
	EXPECT_DOUBLE_EQ(oAssembly.GetToolOffset(),oFresh.GetToolOffset());
	EXPECT_FALSE(v.empty())<<sLog;
}

TEST(ToolAssembly, AnEmptyAssemblyIsSolvedLikeAnyOtherAndChecksNothing)
{
	CChain oChain = LoadChain(GetChainFiles()[0]);
	ASSERT_TRUE(oChain.Is5CChain());
	C5CPostprocessor oPostprocessor(oChain,CPostprocessorSettings());	//checks collisions
	oChain.SetToolOffset(0.0);
	oChain.SetParameters(CChainConfiguration((int)oChain.GetNumberOfRegularLinks()));
	const CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();

	CChain oFresh = LoadChain(GetChainFiles()[0]);
	std::string sLog;
	CSolveReport oReport;
	const tChainConfigurationVector v = oPostprocessor.Solve(cv.m_p,cv.m_v,oFresh
		,CToolAssembly::Empty(),sLog,oReport);
	EXPECT_TRUE(oFresh.IsToolOffsetGiven())<<"zero, but said out loud";
	EXPECT_DOUBLE_EQ(0.0,oFresh.GetToolOffset());
	EXPECT_FALSE(v.empty())<<sLog;
	EXPECT_NE(std::string::npos,sLog.find("nothing is in the spindle"))<<sLog;
}

TEST(ToolAssembly, ANegativeOffsetWouldEndTheToolInsideTheSpindle)
{
	CChain oChain;
	EXPECT_THROW(oChain.SetToolOffset(-0.001),CGeneratorException);
	EXPECT_FALSE(oChain.IsToolOffsetGiven())<<"a refused offset tells it nothing";
	EXPECT_NO_THROW(oChain.SetToolOffset(0.0))<<"the nose itself is a place";
	EXPECT_TRUE(oChain.IsToolOffsetGiven());
}
