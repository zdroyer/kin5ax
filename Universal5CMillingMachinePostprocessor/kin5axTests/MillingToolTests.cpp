// CMillingTool::IsCut: the cut volume must follow the position and the orientation of the tool.
// The tool is placed by its reference point, which lies on the tool axis m_dReferencePointZ above the tip.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "MillingToolBall.h"
#include "rotation.h"

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

class BallToolCutVolumeTest : public ::testing::TestWithParam<CToolPlacement>
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

	bool IsCut(const CR3Point &p) const
	{
		CMillingToolBall oTool(dRADIUS,dRADIUS,dCUTTING_HEIGHT,GetParam().m_dReferencePointZ);
		oTool.SetPositionAndOrientation(m_r3pReference,m_r3vAxis);
		return is_point_cut(oTool.IsCut(p))!=0;
	}
};

TEST_P(BallToolCutVolumeTest, StartsAtTheTip)
{
	EXPECT_TRUE(IsCut(m_r3pTip + m_r3vAxis*dSTEP))<<"just above the tip";
	EXPECT_FALSE(IsCut(m_r3pTip + m_r3vAxis*(-dSTEP)))<<"just below the tip";
}

TEST_P(BallToolCutVolumeTest, EndsAtTheCuttingHeight)
{
	EXPECT_TRUE(IsCut(m_r3pTip + m_r3vAxis*(dCUTTING_HEIGHT-dSTEP)));
	EXPECT_FALSE(IsCut(m_r3pTip + m_r3vAxis*(dCUTTING_HEIGHT+dSTEP)));
}

TEST_P(BallToolCutVolumeTest, BallIsRoundAboutItsCentre)
{
	CR3Point r3pCentre = m_r3pTip + m_r3vAxis*dRADIUS;
	EXPECT_TRUE(IsCut(r3pCentre));
	EXPECT_TRUE(IsCut(r3pCentre + m_r3vSide*(dRADIUS-dSTEP)));
	EXPECT_FALSE(IsCut(r3pCentre + m_r3vSide*(dRADIUS+dSTEP)));
	//45 deg below the equator of the ball
	CR3Vector r3vDown = (m_r3vSide - m_r3vAxis).GetUnitVector();
	EXPECT_TRUE(IsCut(r3pCentre + r3vDown*(dRADIUS-dSTEP)));
	EXPECT_FALSE(IsCut(r3pCentre + r3vDown*(dRADIUS+dSTEP)));
}

TEST_P(BallToolCutVolumeTest, ShankIsACylinderAboutTheAxis)
{
	//above the ball (higher than 2R over the tip)
	CR3Point r3pOnAxis = m_r3pTip + m_r3vAxis*(0.75*dCUTTING_HEIGHT);
	EXPECT_TRUE(IsCut(r3pOnAxis + m_r3vSide*(dRADIUS-dSTEP)));
	EXPECT_FALSE(IsCut(r3pOnAxis + m_r3vSide*(dRADIUS+dSTEP)));
	EXPECT_TRUE(IsCut(r3pOnAxis + m_r3vSide*(-(dRADIUS-dSTEP))));
	EXPECT_FALSE(IsCut(r3pOnAxis + m_r3vSide*(-(dRADIUS+dSTEP))));
}

static vector<CToolPlacement> Placements()
{
	const double vTilts[] = {0.0,10.0,30.0,45.0,90.0,135.0,180.0};
	const double vReferences[] = {0.0,dRADIUS,0.04};	//the tip, the ball centre (the default tool), a point on the shank
	vector<CToolPlacement> v;
	for(int i=0;i<7;i++)
		for(int j=0;j<3;j++){
			CToolPlacement o;
			o.m_dTiltDegrees = vTilts[i];
			o.m_dReferencePointZ = vReferences[j];
			v.push_back(o);
		}
	return v;
}

INSTANTIATE_TEST_CASE_P(Placements, BallToolCutVolumeTest, ::testing::ValuesIn(Placements()));
