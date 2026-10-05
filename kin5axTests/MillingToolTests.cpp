// CMillingTool: the cut volume (IsCut) must follow the position and the orientation of the tool, and the collision
// volume (CauseCollision) must be the same solid. The tool is placed by its reference point, which lies on the
// tool axis m_dReferencePointZ above the tip.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "MillingToolBall.h"
#include "MillingToolCylinder.h"
#include "ArgsMyException.h"
#include "Rotation.h"
#include "Translation.h"
#include <type_traits>

using namespace kin5ax;

static const double dRADIUS = 0.016;
static const double dCUTTING_HEIGHT = 0.064;
static const double dSTEP = 1E-6;	//just inside / just outside

//tilt angle of the tool axis [deg] and the height of the reference point above the tip
struct CToolPlacement
{
	double m_dTiltDegrees;
	double m_dReferencePointZ;
};

void PrintTo(const CToolPlacement &o, ::std::ostream *pOut)
{
	*pOut<<"tilt "<<o.m_dTiltDegrees<<" deg, reference point "<<o.m_dReferencePointZ<<" above the tip";
}

static std::vector<CToolPlacement> Placements()
{
	const double vTilts[] = {0.0,10.0,30.0,45.0,90.0,135.0,180.0};
	const double vReferences[] = {0.0,dRADIUS,0.04};	//the tip, the ball centre (the default tool), a point on the shank
	std::vector<CToolPlacement> v;
	for(int i=0;i<7;i++)
		for(int j=0;j<3;j++){
			CToolPlacement o;
			o.m_dTiltDegrees = vTilts[i];
			o.m_dReferencePointZ = vReferences[j];
			v.push_back(o);
		}
	return v;
}

//KnownBugs #26: the tools are deleted through the base class pointer
TEST(MillingTool, HasAVirtualDestructor)
{
	EXPECT_TRUE(std::has_virtual_destructor<CMillingTool>::value);
	CMillingTool *pTool = new CMillingToolBall(dRADIUS,dRADIUS,dCUTTING_HEIGHT,0.0);
	delete pTool;
}

// ---------------------------------------------------------------- the cut volume

class ToolCutVolumeTest : public ::testing::TestWithParam<CToolPlacement>
{
protected:
	CR3Point m_r3pReference;	//position given to the tool
	CR3Vector m_r3vAxis;		//from the tip towards the spindle
	CR3Vector m_r3vSide;		//perpendicular to the axis
	CR3Point m_r3pTip;

	virtual void SetUp()
	{
		//a general position and a tilt about a general horizontal axis
		m_r3pReference = CR3Point(0.12,-0.07,0.31);
		CRotation rTilt(CR3Vector(2.0,-1.0,0.0),degrees_to_radians(GetParam().m_dTiltDegrees));
		m_r3vAxis = rTilt*CR3Vector(0.0,0.0,1.0);
		m_r3vSide = rTilt*CR3Vector(0.6,0.8,0.0);
		m_r3pTip = m_r3pReference + m_r3vAxis*(-GetParam().m_dReferencePointZ);
	}

	bool IsCut(CMillingTool &oTool, const CR3Point &p) const
	{
		oTool.SetPositionAndOrientation(m_r3pReference,m_r3vAxis);
		return is_point_cut(oTool.IsCut(p))!=0;
	}
	//a point at the height dZ above the tip and dRadial beside the axis
	CR3Point At(double dZ, double dRadial) const
	{
		return m_r3pTip + m_r3vAxis*dZ + m_r3vSide*dRadial;
	}
};

class BallToolCutVolumeTest : public ToolCutVolumeTest
{
protected:
	CMillingToolBall m_oTool;
	BallToolCutVolumeTest() : m_oTool(dRADIUS,dRADIUS,dCUTTING_HEIGHT,0.0) {}
	virtual void SetUp()
	{
		ToolCutVolumeTest::SetUp();
		m_oTool = CMillingToolBall(dRADIUS,dRADIUS,dCUTTING_HEIGHT,GetParam().m_dReferencePointZ);
	}
	bool IsCut(const CR3Point &p) { return ToolCutVolumeTest::IsCut(m_oTool,p); }
};

TEST_P(BallToolCutVolumeTest, StartsAtTheTip)
{
	EXPECT_TRUE(IsCut(At(dSTEP,0.0)))<<"just above the tip";
	EXPECT_FALSE(IsCut(At(-dSTEP,0.0)))<<"just below the tip";
}

TEST_P(BallToolCutVolumeTest, EndsAtTheCuttingHeight)
{
	EXPECT_TRUE(IsCut(At(dCUTTING_HEIGHT-dSTEP,0.0)));
	EXPECT_FALSE(IsCut(At(dCUTTING_HEIGHT+dSTEP,0.0)));
}

TEST_P(BallToolCutVolumeTest, BallIsRoundAboutItsCentre)
{
	CR3Point r3pCentre = At(dRADIUS,0.0);
	EXPECT_TRUE(IsCut(r3pCentre));
	EXPECT_TRUE(IsCut(r3pCentre + m_r3vSide*(dRADIUS-dSTEP)));
	EXPECT_FALSE(IsCut(r3pCentre + m_r3vSide*(dRADIUS+dSTEP)));
	//45 deg below the equator of the ball
	CR3Vector r3vDown = (m_r3vSide - m_r3vAxis).GetUnitVector();
	EXPECT_TRUE(IsCut(r3pCentre + r3vDown*(dRADIUS-dSTEP)));
	EXPECT_FALSE(IsCut(r3pCentre + r3vDown*(dRADIUS+dSTEP)));
}

//the shank of a ball nose tool begins at the equator of the ball, not above the ball
TEST_P(BallToolCutVolumeTest, ShankBeginsAtTheEquator)
{
	EXPECT_TRUE(IsCut(At(1.5*dRADIUS,dRADIUS-dSTEP)))<<"beside the upper half of the ball";
	EXPECT_FALSE(IsCut(At(1.5*dRADIUS,dRADIUS+dSTEP)));
	EXPECT_TRUE(IsCut(At(2.0*dRADIUS,dRADIUS-dSTEP)))<<"at the top of the ball";
	EXPECT_FALSE(IsCut(At(2.0*dRADIUS,dRADIUS+dSTEP)));
	EXPECT_FALSE(IsCut(At(0.5*dRADIUS,dRADIUS-dSTEP)))<<"below the equator only the ball cuts";
}

TEST_P(BallToolCutVolumeTest, ShankIsACylinderAboutTheAxis)
{
	EXPECT_TRUE(IsCut(At(0.75*dCUTTING_HEIGHT,dRADIUS-dSTEP)));
	EXPECT_FALSE(IsCut(At(0.75*dCUTTING_HEIGHT,dRADIUS+dSTEP)));
	EXPECT_TRUE(IsCut(At(0.75*dCUTTING_HEIGHT,-(dRADIUS-dSTEP))));
	EXPECT_FALSE(IsCut(At(0.75*dCUTTING_HEIGHT,-(dRADIUS+dSTEP))));
}

INSTANTIATE_TEST_CASE_P(Placements, BallToolCutVolumeTest, ::testing::ValuesIn(Placements()));

class CylinderToolCutVolumeTest : public ToolCutVolumeTest
{
protected:
	CMillingToolCylinder m_oTool;
	CylinderToolCutVolumeTest() : m_oTool(dRADIUS,0.0,dCUTTING_HEIGHT,0.0) {}
	virtual void SetUp()
	{
		ToolCutVolumeTest::SetUp();
		m_oTool = CMillingToolCylinder(dRADIUS,0.0,dCUTTING_HEIGHT,GetParam().m_dReferencePointZ);
	}
	bool IsCut(const CR3Point &p) { return ToolCutVolumeTest::IsCut(m_oTool,p); }
};

TEST_P(CylinderToolCutVolumeTest, StartsAtTheTip)
{
	EXPECT_TRUE(IsCut(At(dSTEP,0.0)))<<"just above the tip";
	EXPECT_FALSE(IsCut(At(-dSTEP,0.0)))<<"just below the tip";
	EXPECT_TRUE(IsCut(At(dSTEP,dRADIUS-dSTEP)))<<"the flat end cuts up to its edge";
}

TEST_P(CylinderToolCutVolumeTest, EndsAtTheCuttingHeight)
{
	EXPECT_TRUE(IsCut(At(dCUTTING_HEIGHT-dSTEP,0.0)));
	EXPECT_FALSE(IsCut(At(dCUTTING_HEIGHT+dSTEP,0.0)));
}

TEST_P(CylinderToolCutVolumeTest, IsACylinderAboutTheAxis)
{
	EXPECT_TRUE(IsCut(At(0.5*dCUTTING_HEIGHT,dRADIUS-dSTEP)));
	EXPECT_FALSE(IsCut(At(0.5*dCUTTING_HEIGHT,dRADIUS+dSTEP)));
	EXPECT_TRUE(IsCut(At(0.5*dCUTTING_HEIGHT,-(dRADIUS-dSTEP))));
	EXPECT_FALSE(IsCut(At(0.5*dCUTTING_HEIGHT,-(dRADIUS+dSTEP))));
}

INSTANTIATE_TEST_CASE_P(Placements, CylinderToolCutVolumeTest, ::testing::ValuesIn(Placements()));

// ---------------------------------------------------------------- the collision volume

//CauseCollision gets the transformations between the tool frame (the reference point at the origin, the axis
//along OZ) and the frame of the box. The boxes below are given in the tip frame (the tip at the origin), so the
//box frame is the tool frame lifted by the reference point.
class ToolCollisionTest : public ::testing::TestWithParam<double>	//the reference point height
{
protected:
	bool Collides(const CMillingTool &oTool, const CR3Box &oBoxInTipFrame) const
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

TEST_P(ToolCollisionTest, BallToolCollidesWithItsWholeShape)
{
	CMillingToolBall oTool(dRADIUS,dRADIUS,dCUTTING_HEIGHT,GetParam());
	EXPECT_TRUE(Collides(oTool,BoxAt(dRADIUS,0.0)))<<"the centre of the ball";
	EXPECT_TRUE(Collides(oTool,BoxAt(0.5*dCUTTING_HEIGHT,0.0)))<<"the shank";
	EXPECT_TRUE(Collides(oTool,BoxAt(dCUTTING_HEIGHT-0.1*dRADIUS,0.0)))<<"the top of the tool";
	EXPECT_TRUE(Collides(oTool,BoxAt(1.5*dRADIUS,0.9*dRADIUS)))<<"the shank beside the upper half of the ball";
	EXPECT_FALSE(Collides(oTool,BoxAt(dCUTTING_HEIGHT+0.5*dRADIUS,0.0)))<<"above the tool";
	EXPECT_FALSE(Collides(oTool,BoxAt(-0.5*dRADIUS,0.0)))<<"below the tip";
	EXPECT_FALSE(Collides(oTool,BoxAt(0.5*dCUTTING_HEIGHT,1.5*dRADIUS)))<<"beside the shank";
	EXPECT_FALSE(Collides(oTool,BoxAt(0.1*dRADIUS,0.9*dRADIUS)))<<"beside the ball nose: the ball is narrow there";
}

TEST_P(ToolCollisionTest, CylinderToolCollidesWithItsWholeShape)
{
	CMillingToolCylinder oTool(dRADIUS,0.0,dCUTTING_HEIGHT,GetParam());
	EXPECT_TRUE(Collides(oTool,BoxAt(0.1*dRADIUS,0.9*dRADIUS)))<<"the edge of the flat end";
	EXPECT_TRUE(Collides(oTool,BoxAt(0.5*dCUTTING_HEIGHT,0.0)));
	EXPECT_TRUE(Collides(oTool,BoxAt(dCUTTING_HEIGHT-0.1*dRADIUS,0.0)))<<"the top of the tool";
	EXPECT_FALSE(Collides(oTool,BoxAt(dCUTTING_HEIGHT+0.5*dRADIUS,0.0)))<<"above the tool";
	EXPECT_FALSE(Collides(oTool,BoxAt(-0.5*dRADIUS,0.0)))<<"below the tip";
	EXPECT_FALSE(Collides(oTool,BoxAt(0.5*dCUTTING_HEIGHT,1.5*dRADIUS)))<<"beside the shank";
}

INSTANTIATE_TEST_CASE_P(ReferencePoints, ToolCollisionTest, ::testing::Values(0.0,dRADIUS,0.04));

// ---------------------------------------------------------------- the dimensions have to make a tool (#50)

//the dimensions are the point of these tests, not the object: the tool is constructed and dropped
static void MakeBall(double dRz, double dR, double dCuttingHeight, double dReferencePointZ, double dShank = 0.0)
{
	CMillingToolBall oTool(dRz,dR,dCuttingHeight,dReferencePointZ,dShank);
}
static void MakeCylinder(double dRz, double dCuttingHeight, double dReferencePointZ, double dShank = 0.0)
{
	CMillingToolCylinder oTool(dRz,0.0,dCuttingHeight,dReferencePointZ,dShank);
}

TEST(MillingToolDimensions, TheToolsOfTheApplicationPass)
{
	EXPECT_EQ("",CMillingTool::DimensionProblem(MILLING_TOOL_BALL,0.016,0.016,0.064,0.016,0.0))
		<<"the default tool of the document";
	EXPECT_EQ("",CMillingTool::DimensionProblem(MILLING_TOOL_CYLINDER,0.012,0.0,0.040,0.0,0.030))
		<<"the default fields of the tool dialog, with a shank";
	EXPECT_NO_THROW(MakeBall(dRADIUS,dRADIUS,dCUTTING_HEIGHT,dRADIUS));
	EXPECT_NO_THROW(MakeCylinder(dRADIUS,dCUTTING_HEIGHT,0.0));
}

//they used to be fabs'd, so a tool asked for with -16 mm came back with 16 and no word about it
TEST(MillingToolDimensions, ANegativeDimensionIsRefusedInsteadOfAbsolutised)
{
	EXPECT_THROW(MakeBall(-dRADIUS,dRADIUS,dCUTTING_HEIGHT,0.0),CArgsMyException)<<"Rz";
	EXPECT_THROW(MakeBall(dRADIUS,-dRADIUS,dCUTTING_HEIGHT,0.0),CArgsMyException)<<"R";
	EXPECT_THROW(MakeBall(dRADIUS,dRADIUS,-dCUTTING_HEIGHT,0.0),CArgsMyException)<<"the cutting height";
	EXPECT_THROW(MakeBall(dRADIUS,dRADIUS,dCUTTING_HEIGHT,dRADIUS,-0.01),CArgsMyException)<<"the shank";
	EXPECT_THROW(MakeCylinder(-dRADIUS,dCUTTING_HEIGHT,0.0),CArgsMyException);
}

TEST(MillingToolDimensions, ZeroIsNotATool)
{
	EXPECT_THROW(MakeBall(0.0,dRADIUS,dCUTTING_HEIGHT,0.0),CArgsMyException)<<"no radius";
	EXPECT_THROW(MakeBall(dRADIUS,0.0,dCUTTING_HEIGHT,0.0),CArgsMyException)<<"no ball";
	EXPECT_THROW(MakeBall(dRADIUS,dRADIUS,0.0,0.0),CArgsMyException)<<"no cutting part";
	EXPECT_THROW(MakeCylinder(0.0,dCUTTING_HEIGHT,0.0),CArgsMyException)<<"no radius";
	EXPECT_THROW(MakeCylinder(dRADIUS,0.0,0.0),CArgsMyException)<<"no cutting part";
}

TEST(MillingToolDimensions, TheBallFitsTheToolItIsTheNoseOf)
{
	//the check the constructor of the ball nose tool used to carry commented out: a narrower ball leaves
	//the shank standing out of the nose, and the cut volume (ball, then cylinder from the equator) is wrong
	EXPECT_THROW(MakeBall(dRADIUS,0.5*dRADIUS,dCUTTING_HEIGHT,0.0),CArgsMyException)<<"R < Rz";
	EXPECT_NO_THROW(MakeBall(0.5*dRADIUS,dRADIUS,dCUTTING_HEIGHT,dRADIUS))<<"a ball wider than the shank is a tool";
	//and the ball has to end inside the cutting part
	EXPECT_THROW(MakeBall(dRADIUS,dRADIUS,0.5*dRADIUS,0.0),CArgsMyException)<<"the ball is taller than the tool";
	EXPECT_NO_THROW(MakeBall(dRADIUS,dRADIUS,dRADIUS,dRADIUS))<<"a half ball: the cutting part is the ball itself";
}

TEST(MillingToolDimensions, TheProgrammedPointLiesOnTheTool)
{
	EXPECT_THROW(MakeBall(dRADIUS,dRADIUS,dCUTTING_HEIGHT,-dSTEP),CArgsMyException)<<"below the tip";
	EXPECT_THROW(MakeCylinder(dRADIUS,dCUTTING_HEIGHT,2.0*dCUTTING_HEIGHT),CArgsMyException)<<"above the tool";
	EXPECT_NO_THROW(MakeCylinder(dRADIUS,dCUTTING_HEIGHT,dCUTTING_HEIGHT))<<"the top of the cutting part";
	EXPECT_NO_THROW(MakeCylinder(dRADIUS,dCUTTING_HEIGHT,1.5*dCUTTING_HEIGHT,dCUTTING_HEIGHT))
		<<"a shank makes the tool longer, so a point above the cutting part is on the tool";
}

//KnownBugs #50: the box of the cutting part was unioned with the box of a solid that is not there, and an
//empty box used to stretch the union to the world origin
TEST(MillingToolDimensions, TheBoundingBoxOfTheCuttingPartStaysAroundTheTool)
{
	CMillingToolBall oBall(dRADIUS,dRADIUS,dCUTTING_HEIGHT,dRADIUS);
	oBall.SetPositionAndOrientation(CR3Point(1.0,2.0,3.0),CR3Vector(0.0,0.0,1.0));
	CR3Box box = oBall.GetCuttingPartBoundingBox();
	EXPECT_FALSE(box.Contains(CR3Point(0.0,0.0,0.0)))<<"the world origin is metres away from the tool";
	EXPECT_TRUE(box.Contains(CR3Point(1.0,2.0,3.0)))<<"the programmed point";
	EXPECT_TRUE(box.Contains(CR3Point(1.0,2.0,3.0-dRADIUS)))<<"the tip";
	EXPECT_TRUE(box.Contains(CR3Point(1.0,2.0,3.0+dCUTTING_HEIGHT-dRADIUS)))<<"the top of the cutting part";
}
