// The solids a chain segment carries: a box, a piece of a cylinder, a piece of a sphere, and the
// pairwise test that says whether two of them meet. The promise of CSolid::Meets is one-sided - it
// never misses a collision that is there, and may report one that is not (KnownBugs #72) - so the
// cross-check at the end of this file samples points of the overlap and insists on the first half
// of that. The second half is only measured, since the design allows it.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "Solid.h"
#include "Translation.h"
#include "Rotation.h"
#include <random>
#include <vector>

using namespace kin5ax;

namespace {

//A solid and the frame it is given in, which is what a chain segment hands to DetectCollisions.
//Both directions are kept: CTransformation::Inverted refuses a general matrix, so an inverse is
//built out of the inverses of the pieces, in the other order - the way a chain segment does it.
struct CPlaced
{
	CSolid m_oSolid;
	CTransformation m_tr;			//from the frame of the solid to the common frame
	CTransformation m_trInv;
	CPlaced() :m_tr(1.0) ,m_trInv(1.0) {}
};

void Place(CPlaced &o, const CSolid &oSolid, double dX, double dY, double dZ
	,const CR3Vector &vAxis = CR3Vector(0.0,0.0,1.0), double dDegrees = 0.0)
{
	o.m_oSolid = oSolid;
	o.m_tr = CTranslation(dX,dY,dZ)*CRotation(vAxis,dDegrees,0.0);
	o.m_trInv = CRotation(vAxis,-dDegrees,0.0)*CTranslation(-dX,-dY,-dZ);
}

//does a point of the common frame lie in both solids?
bool InBoth(const CPlaced &a, const CPlaced &b, const CR3Point &p)
{
	return a.m_oSolid.Contains(a.m_trInv*p) && b.m_oSolid.Contains(b.m_trInv*p);
}

//Meets, as the chain calls it: tr from the frame of the other solid to the frame of this one
bool Meet(const CPlaced &a, const CPlaced &b)
{
	const CTransformation tr = a.m_trInv*b.m_tr;
	const CTransformation trInv = b.m_trInv*a.m_tr;
	const bool bOne = a.m_oSolid.Meets(b.m_oSolid,tr,trInv);
	//the answer must not depend on which of the two is asked
	EXPECT_EQ(bOne,b.m_oSolid.Meets(a.m_oSolid,trInv,tr))
		<<a.m_oSolid.GetTypeName()<<" against "<<b.m_oSolid.GetTypeName();
	return bOne;
}

CSolid Box(double dMinX, double dMaxX, double dMinY, double dMaxY, double dMinZ, double dMaxZ)
{
	return CSolid(CR3Box(dMinX,dMaxX,dMinY,dMaxY,dMinZ,dMaxZ));
}

} // namespace

// ---------------------------------------------------------------- the axes

TEST(Solid, AnAxisHasANameAndTwoAxesAcrossIt)
{
	EXPECT_STREQ("x",R3AxisName(R3_AXIS_X));
	EXPECT_STREQ("z",R3AxisName(R3_AXIS_Z));
	R3_AXIS nAxis = R3_AXIS_X;
	EXPECT_TRUE(R3AxisFromName("z",nAxis));
	EXPECT_EQ(R3_AXIS_Z,nAxis);
	EXPECT_TRUE(R3AxisFromName("Y",nAxis))<<"upper case is the same axis";
	EXPECT_EQ(R3_AXIS_Y,nAxis);
	EXPECT_FALSE(R3AxisFromName("w",nAxis));
	EXPECT_FALSE(R3AxisFromName("xy",nAxis));

	//cyclic, so that the three always make a right-handed frame
	R3_AXIS n1 = R3_AXIS_X, n2 = R3_AXIS_X;
	R3CrossAxes(R3_AXIS_Z,n1,n2);
	EXPECT_EQ(R3_AXIS_X,n1);
	EXPECT_EQ(R3_AXIS_Y,n2);
	R3CrossAxes(R3_AXIS_X,n1,n2);
	EXPECT_EQ(R3_AXIS_Y,n1);
	EXPECT_EQ(R3_AXIS_Z,n2);
	R3CrossAxes(R3_AXIS_Y,n1,n2);
	EXPECT_EQ(R3_AXIS_Z,n1);
	EXPECT_EQ(R3_AXIS_X,n2);

	//the point built from the three coordinates reads back the same way round
	const CR3Point p = R3PointFromAxes(R3_AXIS_Y,1.0,2.0,3.0);		//along Y, then Z, then X
	EXPECT_DOUBLE_EQ(3.0,p.GetX());
	EXPECT_DOUBLE_EQ(1.0,p.GetY());
	EXPECT_DOUBLE_EQ(2.0,p.GetZ());
}

// ---------------------------------------------------------------- the cylinder

TEST(Solid, ACylinderIsItsAxisItsEndsItsRadiusAndItsSector)
{
	//a Z cylinder, 100 mm long, radius 30, off to one side
	const CR3CylinderSegment oC(R3_AXIS_Z,CR3Point(0.1,0.0,0.0),0.2,0.3,0.03);
	EXPECT_DOUBLE_EQ(0.1,oC.GetLength());
	EXPECT_TRUE(oC.IsWholeTurn());
	EXPECT_FALSE(oC.IsEmpty());
	EXPECT_EQ("",oC.Problem());
	EXPECT_TRUE(oC.Contains(CR3Point(0.1,0.0,0.25)))<<"the middle of the axis";
	EXPECT_TRUE(oC.Contains(CR3Point(0.13,0.0,0.3)))<<"the rim of the top";
	EXPECT_FALSE(oC.Contains(CR3Point(0.1,0.0,0.31)))<<"above the top";
	EXPECT_FALSE(oC.Contains(CR3Point(0.1,0.0,0.19)))<<"below the base";
	EXPECT_FALSE(oC.Contains(CR3Point(0.14,0.0,0.25)))<<"outside the radius";

	//the ends of the axis, and the frame the collision test works in
	EXPECT_DOUBLE_EQ(0.2,oC.GetBasePoint().GetZ());
	EXPECT_DOUBLE_EQ(0.1,oC.GetBasePoint().GetX());
	EXPECT_DOUBLE_EQ(0.3,oC.GetTopPoint().GetZ());
	const CTransformation tr = oC.GetCanonicalTransformation();
	const CR3Point pBase = tr*CR3Point(0.0,0.0,0.0);
	EXPECT_NEAR(0.1,pBase.GetX(),1E-12);
	EXPECT_NEAR(0.2,pBase.GetZ(),1E-12);
	const CR3Point pRim = tr*CR3Point(0.03,0.0,0.1);		//the rim of the canonical top
	EXPECT_NEAR(0.13,pRim.GetX(),1E-12);
	EXPECT_NEAR(0.3,pRim.GetZ(),1E-12);
	EXPECT_TRUE(oC.Contains(pRim));
	//and back again
	const CR3Point pCanon = oC.GetInvertedCanonicalTransformation()*pRim;
	EXPECT_NEAR(0.03,pCanon.GetX(),1E-12);
	EXPECT_NEAR(0.0,pCanon.GetY(),1E-12);
	EXPECT_NEAR(0.1,pCanon.GetZ(),1E-12);

	//an empty one is nothing: no radius, or no length
	EXPECT_TRUE(CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.0),0.0,0.1,0.0).IsEmpty());
	EXPECT_TRUE(CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.0),0.1,0.1,0.03).IsEmpty());
	EXPECT_NE("",CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.0),0.0,0.1,0.0).Problem());
}

TEST(Solid, TheCanonicalFrameOfEveryAxisPutsTheLengthAlongTheAxis)
{
	//the canonical Z is the axis of the cylinder, whichever axis that is
	for(int i=0;i<R3_AXIS_COUNT;i++){
		const R3_AXIS nAxis = (R3_AXIS)i;
		const CR3CylinderSegment oC(nAxis,CR3Point(0.0),0.1,0.4,0.05);
		const CTransformation tr = oC.GetCanonicalTransformation();
		const CR3Point pTop = tr*CR3Point(0.0,0.0,0.3);		//the canonical top, on the axis
		EXPECT_NEAR(0.4,R3AxisComponent(pTop,nAxis),1E-12)<<"axis "<<R3AxisName(nAxis);
		EXPECT_TRUE(oC.Contains(pTop));
		//and the radius goes across it
		const CR3Point pSide = tr*CR3Point(0.05,0.0,0.3);
		EXPECT_NEAR(0.4,R3AxisComponent(pSide,nAxis),1E-12);
		EXPECT_TRUE(oC.Contains(pSide));
		const CR3Point pBack = oC.GetInvertedCanonicalTransformation()*pSide;
		EXPECT_NEAR(0.05,pBack.GetX(),1E-12);
		EXPECT_NEAR(0.3,pBack.GetZ(),1E-12);
	}
}

TEST(Solid, TheSectorOfACylinderIsAWedgeThatMayCrossZero)
{
	//a quarter of a Z cylinder, from +X towards +Y
	const CR3CylinderSegment oQuarter(R3_AXIS_Z,CR3Point(0.0),0.0,0.1,1.0,0.0,90.0);
	EXPECT_FALSE(oQuarter.IsWholeTurn());
	EXPECT_TRUE(oQuarter.Contains(CR3Point(0.5,0.5,0.05)))<<"45 degrees, inside";
	EXPECT_TRUE(oQuarter.Contains(CR3Point(0.0,0.0,0.05)))<<"the axis is in every wedge";
	EXPECT_FALSE(oQuarter.Contains(CR3Point(-0.5,0.5,0.05)))<<"135 degrees";
	EXPECT_FALSE(oQuarter.Contains(CR3Point(0.5,-0.5,0.05)))<<"-45 degrees";

	//a wedge written across zero keeps its two sides
	const CR3CylinderSegment oAcross(R3_AXIS_Z,CR3Point(0.0),0.0,0.1,1.0,-30.0,30.0);
	EXPECT_TRUE(oAcross.Contains(CR3Point(1.0,0.0,0.05)))<<"zero itself";
	EXPECT_TRUE(oAcross.Contains(CR3Point(0.9,0.4,0.05)))<<"a touch under 30 degrees";
	EXPECT_TRUE(oAcross.Contains(CR3Point(0.9,-0.4,0.05)))<<"and under -30";
	EXPECT_FALSE(oAcross.Contains(CR3Point(0.4,0.9,0.05)))<<"about 66 degrees";
	EXPECT_TRUE(IsAngleInSector(5.0,330.0,390.0));
	EXPECT_TRUE(IsAngleInSector(-5.0,330.0,390.0))<<"the same place, written the other way";
	EXPECT_FALSE(IsAngleInSector(180.0,330.0,390.0));
	EXPECT_TRUE(IsAngleInSector(180.0,0.0,360.0))<<"the whole turn takes everything";
}

TEST(Solid, TheBoxOfACylinderHoldsItAndNoMore)
{
	//the whole turn: the box is the square around the circle
	const CR3Box oWhole = CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.0),0.0,0.1,1.0).GetBoundingBox();
	EXPECT_DOUBLE_EQ(-1.0,oWhole.GetMinPoint().GetX());
	EXPECT_DOUBLE_EQ(1.0,oWhole.GetMaxPoint().GetY());
	EXPECT_DOUBLE_EQ(0.0,oWhole.GetMinPoint().GetZ());
	EXPECT_DOUBLE_EQ(0.1,oWhole.GetMaxPoint().GetZ());

	//a quarter from +X to +Y: the wedge reaches the axis, so the box is the positive quadrant
	const CR3Box oQuarter = CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.0),0.0,0.1,1.0,0.0,90.0)
		.GetBoundingBox();
	EXPECT_NEAR(0.0,oQuarter.GetMinPoint().GetX(),1E-12);
	EXPECT_NEAR(0.0,oQuarter.GetMinPoint().GetY(),1E-12);
	EXPECT_NEAR(1.0,oQuarter.GetMaxPoint().GetX(),1E-12);
	EXPECT_NEAR(1.0,oQuarter.GetMaxPoint().GetY(),1E-12);

	//a wedge across zero reaches +X fully and both ways in Y, but not to -X
	const CR3Box oAcross = CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.0),0.0,0.1,1.0,-30.0,30.0)
		.GetBoundingBox();
	EXPECT_NEAR(1.0,oAcross.GetMaxPoint().GetX(),1E-12);
	EXPECT_NEAR(0.0,oAcross.GetMinPoint().GetX(),1E-12)<<"the apex";
	EXPECT_NEAR(0.5,oAcross.GetMaxPoint().GetY(),1E-9);
	EXPECT_NEAR(-0.5,oAcross.GetMinPoint().GetY(),1E-9);

	//an X cylinder puts its length along X and its circle across Y and Z
	const CR3Box oX = CR3CylinderSegment(R3_AXIS_X,CR3Point(0.0,0.2,0.0),0.0,0.5,0.1)
		.GetBoundingBox();
	EXPECT_DOUBLE_EQ(0.0,oX.GetMinPoint().GetX());
	EXPECT_DOUBLE_EQ(0.5,oX.GetMaxPoint().GetX());
	EXPECT_DOUBLE_EQ(0.1,oX.GetMinPoint().GetY());
	EXPECT_DOUBLE_EQ(0.3,oX.GetMaxPoint().GetY());
	EXPECT_DOUBLE_EQ(-0.1,oX.GetMinPoint().GetZ());
}

// ---------------------------------------------------------------- the sphere

TEST(Solid, ASphereIsACentreARadiusAndTwoCuts)
{
	const CR3SphereSegment oWhole(CR3Point(0.0,0.0,0.5),0.1);
	EXPECT_TRUE(oWhole.IsWholeSphere());
	EXPECT_EQ("",oWhole.Problem());
	EXPECT_TRUE(oWhole.Contains(CR3Point(0.0,0.0,0.6)))<<"the north pole";
	EXPECT_TRUE(oWhole.Contains(CR3Point(0.0,0.0,0.4)))<<"and the south one";
	EXPECT_FALSE(oWhole.Contains(CR3Point(0.0,0.0,0.61)));

	//the upper half, cut across Z
	const CR3SphereSegment oDome(CR3Point(0.0,0.0,0.5),0.1,R3_AXIS_Z,0.0,0.1);
	EXPECT_FALSE(oDome.IsWholeSphere());
	EXPECT_TRUE(oDome.Contains(CR3Point(0.0,0.0,0.6)));
	EXPECT_TRUE(oDome.Contains(CR3Point(0.09,0.0,0.5)))<<"the rim of the cut";
	EXPECT_FALSE(oDome.Contains(CR3Point(0.0,0.0,0.45)))<<"below the cut";

	//a radius that grows keeps a whole sphere whole, and a cut one cut
	CR3SphereSegment oGrow(CR3Point(0.0),0.1);
	oGrow.SetRadius(0.2);
	EXPECT_TRUE(oGrow.IsWholeSphere());
	EXPECT_TRUE(oGrow.Contains(CR3Point(0.0,0.0,-0.19)));
	CR3SphereSegment oCut(CR3Point(0.0),0.1,R3_AXIS_Z,0.0,0.1);
	oCut.SetRadius(0.2);
	EXPECT_FALSE(oCut.IsWholeSphere());
	EXPECT_FALSE(oCut.Contains(CR3Point(0.0,0.0,-0.05)));

	EXPECT_TRUE(CR3SphereSegment(CR3Point(0.0),0.0).IsEmpty());
	EXPECT_NE("",CR3SphereSegment(CR3Point(0.0),0.0).Problem());
}

TEST(Solid, TheBoxOfASphereFollowsItsCut)
{
	const CR3Box oWhole = CR3SphereSegment(CR3Point(0.0,0.0,0.5),0.1).GetBoundingBox();
	EXPECT_DOUBLE_EQ(-0.1,oWhole.GetMinPoint().GetX());
	EXPECT_DOUBLE_EQ(0.4,oWhole.GetMinPoint().GetZ());
	EXPECT_DOUBLE_EQ(0.6,oWhole.GetMaxPoint().GetZ());

	//a dome: half as tall, and as wide as the equator because the cut is at it
	const CR3Box oDome = CR3SphereSegment(CR3Point(0.0,0.0,0.5),0.1,R3_AXIS_Z,0.0,0.1)
		.GetBoundingBox();
	EXPECT_DOUBLE_EQ(0.5,oDome.GetMinPoint().GetZ());
	EXPECT_DOUBLE_EQ(0.6,oDome.GetMaxPoint().GetZ());
	EXPECT_DOUBLE_EQ(-0.1,oDome.GetMinPoint().GetX());

	//a cap taken above the equator is narrower than the sphere: the circle at the nearer cut
	const CR3Box oCap = CR3SphereSegment(CR3Point(0.0),1.0,R3_AXIS_Z,0.6,1.0).GetBoundingBox();
	EXPECT_NEAR(0.8,oCap.GetMaxPoint().GetX(),1E-12);
	EXPECT_NEAR(-0.8,oCap.GetMinPoint().GetY(),1E-12);
	EXPECT_NEAR(0.6,oCap.GetMinPoint().GetZ(),1E-12);
}

// ---------------------------------------------------------------- the six pairs

TEST(Solid, BoxAgainstBoxIsTheTestItAlwaysWas)
{
	CPlaced a, b;
	Place(a,Box(-0.1,0.1,-0.1,0.1,0.0,0.2),0.0,0.0,0.0);
	Place(b,Box(-0.05,0.05,-0.05,0.05,0.0,0.1),0.0,0.0,0.15);
	EXPECT_TRUE(Meet(a,b))<<"the second box stands in the first";
	Place(b,b.m_oSolid,0.0,0.0,0.22);
	EXPECT_FALSE(Meet(a,b))<<"standing 20 mm clear above it";
	//the same place, leaned over: a corner of a 100 mm box dips 35 mm below its own origin
	Place(b,b.m_oSolid,0.0,0.0,0.22,CR3Vector(1.0,0.0,0.0),45.0);
	EXPECT_TRUE(Meet(a,b))<<"leaned over, a corner reaches down into it";
}

TEST(Solid, ACylinderMeetsABoxWhereItReallyIs)
{
	CPlaced a, b;
	Place(a,Box(-0.02,0.02,-0.02,0.02,0.0,0.1),0.0,0.0,0.0);		//a post at the origin
	//a Z cylinder beside it, 100 mm away: a radius of 50 mm leaves 30 mm, a radius of 90 mm does not
	Place(b,CSolid(CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.1,0.0,0.0),0.0,0.1,0.05)),0.0,0.0,0.0);
	EXPECT_FALSE(Meet(a,b))<<"a gap of 30 mm between them";
	Place(b,CSolid(CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.1,0.0,0.0),0.0,0.1,0.09)),0.0,0.0,0.0);
	EXPECT_TRUE(Meet(a,b))<<"a radius of 90 mm reaches the post";
	//and the two ends are exact: the same cylinder put above the post meets nothing
	Place(b,b.m_oSolid,0.0,0.0,0.2);
	EXPECT_FALSE(Meet(a,b));
}

TEST(Solid, ASphereMeetsABoxWhereItReallyIs)
{
	CPlaced a, b;
	Place(a,Box(-0.1,0.1,-0.1,0.1,-0.01,0.0),0.0,0.0,0.0);		//a plate under the origin
	Place(b,CSolid(CR3SphereSegment(CR3Point(0.0,0.0,0.05),0.04)),0.0,0.0,0.0);
	EXPECT_FALSE(Meet(a,b))<<"the ball hangs 10 mm above the plate";
	Place(b,CSolid(CR3SphereSegment(CR3Point(0.0,0.0,0.05),0.06)),0.0,0.0,0.0);
	EXPECT_TRUE(Meet(a,b))<<"a bigger ball touches it";
	//the cut is taken into account through the box of the solid: a dome facing up cannot reach down
	Place(b,CSolid(CR3SphereSegment(CR3Point(0.0,0.0,0.05),0.06,R3_AXIS_Z,0.02,0.06)),0.0,0.0,0.0);
	EXPECT_FALSE(Meet(a,b))<<"only the top of that ball is there";
}

TEST(Solid, TwoSpheresMeetWhenTheirCentresAreCloserThanTheirRadii)
{
	CPlaced a, b;
	Place(a,CSolid(CR3SphereSegment(CR3Point(0.0),0.05)),0.0,0.0,0.0);
	Place(b,CSolid(CR3SphereSegment(CR3Point(0.0),0.05)),0.0,0.0,0.09);
	EXPECT_TRUE(Meet(a,b));
	Place(b,b.m_oSolid,0.0,0.0,0.11);
	EXPECT_FALSE(Meet(a,b));
	//the centre of a sphere is in its own frame, so it moves with the frame
	Place(b,CSolid(CR3SphereSegment(CR3Point(0.0,0.0,-0.05),0.05)),0.0,0.0,0.11);
	EXPECT_TRUE(Meet(a,b))<<"the centre is now 60 mm away";
}

TEST(Solid, ACylinderAndASphereMeetAlongTheAxisOfTheCylinder)
{
	CPlaced a, b;
	//a bar along X, 500 mm long and 20 mm thick
	Place(a,CSolid(CR3CylinderSegment(R3_AXIS_X,CR3Point(0.0),0.0,0.5,0.02)),0.0,0.0,0.0);
	Place(b,CSolid(CR3SphereSegment(CR3Point(0.0),0.03)),0.25,0.04,0.0);
	EXPECT_TRUE(Meet(a,b))<<"the ball sits on the middle of the bar";
	Place(b,b.m_oSolid,0.25,0.06,0.0);
	EXPECT_FALSE(Meet(a,b))<<"10 mm clear of it";
	Place(b,b.m_oSolid,0.6,0.0,0.0);
	EXPECT_FALSE(Meet(a,b))<<"past the end of the bar";
	Place(b,b.m_oSolid,0.52,0.0,0.0);
	EXPECT_TRUE(Meet(a,b))<<"just off the end, and touching it";
}

TEST(Solid, TwoCylindersMeetWhenTheirAxesComeCloseEnough)
{
	CPlaced a, b;
	const CSolid oBar = CSolid(CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.0),0.0,0.3,0.02));
	Place(a,oBar,0.0,0.0,0.0);
	Place(b,oBar,0.03,0.0,0.0);
	EXPECT_TRUE(Meet(a,b))<<"two parallel bars 30 mm apart, each 20 mm thick";
	Place(b,oBar,0.05,0.0,0.0);
	EXPECT_FALSE(Meet(a,b))<<"50 mm apart and they clear each other";
	//crossing at a right angle, through the middle
	Place(b,oBar,0.0,0.0,0.15,CR3Vector(1.0,0.0,0.0),90.0);
	EXPECT_TRUE(Meet(a,b));
	//the same crossing moved along, so that the second bar passes clear of the first
	Place(b,oBar,0.0,0.0,0.45,CR3Vector(1.0,0.0,0.0),90.0);
	EXPECT_FALSE(Meet(a,b));
}

TEST(Solid, AnEmptySolidMeetsNothing)
{
	CPlaced a, b;
	Place(a,CSolid(),0.0,0.0,0.0);						//the empty box
	Place(b,Box(-1.0,1.0,-1.0,1.0,-1.0,1.0),0.0,0.0,0.0);
	EXPECT_FALSE(Meet(a,b));
	Place(a,CSolid(CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.0),0.0,0.0,0.1)),0.0,0.0,0.0);
	EXPECT_FALSE(Meet(a,b))<<"a cylinder of no length";
	Place(a,CSolid(CR3SphereSegment(CR3Point(0.0),0.0)),0.0,0.0,0.0);
	EXPECT_FALSE(Meet(a,b))<<"a sphere of no radius";
}

TEST(Solid, ASolidKnowsItsTypeAndItsName)
{
	EXPECT_STREQ("box",CSolid().GetTypeName());
	EXPECT_EQ(SOLID_BOX,CSolid().GetType());
	EXPECT_STREQ("cylinder",CSolid(CR3CylinderSegment()).GetTypeName());
	EXPECT_EQ(SOLID_SPHERE,CSolid(CR3SphereSegment()).GetType());
	SOLID_TYPE nType = SOLID_BOX;
	EXPECT_TRUE(SolidTypeFromName("sphere",nType));
	EXPECT_EQ(SOLID_SPHERE,nType);
	EXPECT_FALSE(SolidTypeFromName("torus",nType));
	//a box is a solid without being asked twice, and two of them compare
	const CSolid oOne = CR3Box(0.0,1.0,0.0,1.0,0.0,1.0);
	EXPECT_EQ(SOLID_BOX,oOne.GetType());
	EXPECT_TRUE(oOne==CSolid(CR3Box(0.0,1.0,0.0,1.0,0.0,1.0)));
	EXPECT_TRUE(oOne!=CSolid(CR3Box(0.0,2.0,0.0,1.0,0.0,1.0)));
	EXPECT_TRUE(oOne!=CSolid(CR3SphereSegment(CR3Point(0.0),1.0)));
}

// ---------------------------------------------------------------- the cross-check

//The one-sided promise, sampled: wherever a point of space is inside both solids, Meets has to say
//so. Random frames, every pair of types, and a grid over the box of the first solid. The other
//direction - Meets saying yes where no point is shared - is allowed and only counted.
TEST(Solid, MeetsNeverMissesAnOverlapThatSamplingFinds)
{
	std::vector<CSolid> vSolids;
	vSolids.push_back(Box(-0.06,0.06,-0.04,0.04,0.0,0.2));
	vSolids.push_back(CSolid(CR3CylinderSegment(R3_AXIS_Z,CR3Point(0.0),0.0,0.2,0.05)));
	vSolids.push_back(CSolid(CR3CylinderSegment(R3_AXIS_X,CR3Point(0.0,0.03,0.0),-0.1,0.1,0.04)));
	vSolids.push_back(CSolid(CR3CylinderSegment(R3_AXIS_Y,CR3Point(0.0),0.0,0.15,0.06,0.0,120.0)));
	vSolids.push_back(CSolid(CR3SphereSegment(CR3Point(0.0,0.0,0.1),0.07)));
	vSolids.push_back(CSolid(CR3SphereSegment(CR3Point(0.0),0.08,R3_AXIS_Z,0.0,0.08)));

	std::mt19937 oGenerator(7);
	std::uniform_real_distribution<double> oShift(-0.2,0.2);
	std::uniform_real_distribution<double> oAngle(-180.0,180.0);
	size_t nOverlaps = 0, nConservative = 0, nPairs = 0;
	for(size_t i=0;i<vSolids.size();i++)
		for(size_t j=0;j<vSolids.size();j++)
			for(int s=0;s<40;s++){
				CPlaced a, b;
				Place(a,vSolids[i],0.0,0.0,0.0);
				Place(b,vSolids[j],oShift(oGenerator),oShift(oGenerator),oShift(oGenerator)
					,CR3Vector(0.3,-0.5,0.8),oAngle(oGenerator));
				nPairs++;
				const bool bMeets = Meet(a,b);

				//a grid over the box of the first solid, in the common frame
				const CR3Box oBox = a.m_oSolid.GetBoundingBox();
				const CR3Point pMin = oBox.GetMinPoint();
				const CR3Point pMax = oBox.GetMaxPoint();
				const int nSteps = 12;
				bool bFound = false;
				for(int x=0;x<=nSteps && !bFound;x++)
					for(int y=0;y<=nSteps && !bFound;y++)
						for(int z=0;z<=nSteps && !bFound;z++){
							const CR3Point p(pMin.GetX()+(pMax.GetX()-pMin.GetX())*x/nSteps
								,pMin.GetY()+(pMax.GetY()-pMin.GetY())*y/nSteps
								,pMin.GetZ()+(pMax.GetZ()-pMin.GetZ())*z/nSteps);
							if(InBoth(a,b,p))
								bFound = true;
						}
				if(bFound){
					nOverlaps++;
					EXPECT_TRUE(bMeets)<<"sample "<<s<<": "<<a.m_oSolid.GetTypeName()<<" ("<<i
						<<") and "<<b.m_oSolid.GetTypeName()<<" ("<<j
						<<") share a point, and Meets said no";
				} else if(bMeets){
					nConservative++;
				}
			}
	EXPECT_GT(nOverlaps,(size_t)200)<<"too few overlaps among the samples to call this a test";
	RecordProperty("pairs",(int)nPairs);
	RecordProperty("overlaps_found_by_sampling",(int)nOverlaps);
	RecordProperty("meets_without_a_sampled_point",(int)nConservative);
}
