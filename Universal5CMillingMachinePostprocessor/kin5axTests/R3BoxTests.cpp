// CR3Box: intersection with another box (every edge is tested) and with a cylinder about OZ.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "r3box.h"
#include "rotation.h"
#include "translation.h"

//gives the tests access to the one-directional edge test
class CR3BoxProbe : public CR3Box
{
public:
	CR3BoxProbe(double dMinX,double dMaxX,double dMinY,double dMaxY,double dMinZ,double dMaxZ)
		:CR3Box(dMinX,dMaxX,dMinY,dMaxY,dMinZ,dMaxZ)
	{}
	using CR3Box::AsymmetricIntersection;
};

// ---------------------------------------------------------------- box against box: all 12 edges

//the 12 edges of the box [-1,1]^3: two corners each
static const double vEDGES[12][6] = {
	{-1,-1,-1,  1,-1,-1}, {-1, 1,-1,  1, 1,-1}, {-1,-1, 1,  1,-1, 1}, {-1, 1, 1,  1, 1, 1},	//along X
	{-1,-1,-1, -1, 1,-1}, { 1,-1,-1,  1, 1,-1}, {-1,-1, 1, -1, 1, 1}, { 1,-1, 1,  1, 1, 1},	//along Y
	{-1,-1,-1, -1,-1, 1}, { 1,-1,-1,  1,-1, 1}, {-1, 1,-1, -1, 1, 1}, { 1, 1,-1,  1, 1, 1}};	//along Z

class BoxEdgeTest : public ::testing::TestWithParam<int>
{
};

//a small box around the middle of one edge of a big box: only that edge of the big box passes through it
TEST_P(BoxEdgeTest, EveryEdgeOfTheOtherBoxIsTested)
{
	const double *e = vEDGES[GetParam()];
	double dX = 0.5*(e[0]+e[3]), dY = 0.5*(e[1]+e[4]), dZ = 0.5*(e[2]+e[5]);
	CR3BoxProbe oSmall(dX-0.1,dX+0.1,dY-0.1,dY+0.1,dZ-0.1,dZ+0.1);
	CR3Box oBig(-1.0,1.0,-1.0,1.0,-1.0,1.0);
	EXPECT_TRUE(oSmall.AsymmetricIntersection(oBig,CTransformation(1.0)))<<"edge ("<<e[0]<<","<<e[1]<<","<<e[2]<<")-("<<e[3]<<","<<e[4]<<","<<e[5]<<")";
	//and the same through the public, symmetric test
	EXPECT_TRUE(oSmall.HasIntersectionWithBox(oBig,CTransformation(1.0),CTransformation(1.0)));
}

INSTANTIATE_TEST_CASE_P(Edges, BoxEdgeTest, ::testing::Range(0,12));

// ---------------------------------------------------------------- box against a line segment

TEST(BoxAgainstSegment, EndpointInsideOrOutside)
{
	CR3Box oBox(0.0,1.0,0.0,1.0,0.0,1.0);
	EXPECT_TRUE(oBox.HasIntersectionWithLineSegment(CR3Point(0.5,0.5,0.5),CR3Point(5.0,5.0,5.0)));
	EXPECT_FALSE(oBox.HasIntersectionWithLineSegment(CR3Point(2.0,0.5,0.5),CR3Point(5.0,0.5,0.5)));
	EXPECT_FALSE(oBox.HasIntersectionWithLineSegment(CR3Point(2.0,2.0,2.0),CR3Point(2.0,2.0,2.0)))<<"a point outside";
	EXPECT_TRUE(oBox.HasIntersectionWithLineSegment(CR3Point(1.0,1.0,1.0),CR3Point(2.0,2.0,2.0)))<<"touches the corner";
}

//the segment passes through the box; neither endpoint is inside; all directions of travel
TEST(BoxAgainstSegment, PassesThroughInEveryDirection)
{
	CR3Box oBox(0.0,1.0,0.0,1.0,0.0,1.0);
	const CR3Point a(-1.0,0.2,0.3), b(2.0,0.6,0.7);	//through the box along +X
	EXPECT_TRUE(oBox.HasIntersectionWithLineSegment(a,b));
	EXPECT_TRUE(oBox.HasIntersectionWithLineSegment(b,a))<<"the same segment, travelled in -X";
	//a diagonal that enters through the x=1 face and leaves through the y=0 face, travelled in -X and -Y
	const CR3Point c(2.0,1.5,0.5), d(0.2,-0.5,0.5);		//inside for t in [0.56,0.75]
	EXPECT_TRUE(oBox.HasIntersectionWithLineSegment(d,c));
	EXPECT_TRUE(oBox.HasIntersectionWithLineSegment(c,d))<<"KnownBugs #6";
	const CR3Point e(2.0,1.5,0.5), f(0.2,-0.6,0.5);		//inside for t in [0.56,0.71]
	EXPECT_TRUE(oBox.HasIntersectionWithLineSegment(e,f));
	EXPECT_TRUE(oBox.HasIntersectionWithLineSegment(f,e));
	//a diagonal that passes just outside the corner (1,0,z): x<=1 needs t>=0.67, y>=0 needs t<=0.55
	const CR3Point g(2.0,1.2,0.5), h(0.5,-1.0,0.5);
	EXPECT_FALSE(oBox.HasIntersectionWithLineSegment(g,h));
	EXPECT_FALSE(oBox.HasIntersectionWithLineSegment(h,g));
}

//random segments against the box, compared with a fine sampling of the segment
TEST(BoxAgainstSegment, AgreesWithPointSampling)
{
	CR3Box oBox(-0.5,0.5,-0.3,0.3,-0.2,0.2);
	srand(508);
	int nHits=0;
	for(int n=0;n<2000;n++){
		double r[6];
		for(int i=0;i<6;i++)
			r[i] = 2.0*(double)rand()/(double)RAND_MAX-1.0;
		CR3Point p1(r[0],r[1],r[2]), p2(r[3],r[4],r[5]);
		bool bInside=false, bNear=false;
		const int nSTEPS=400;
		for(int i=0;i<=nSTEPS;i++){
			double t = (double)i/nSTEPS;
			CR3Point p = p1 + (p2-p1)*t;
			if(oBox.Contains(p))
				bInside=true;
			CR3Box oGrown(oBox);
			oGrown.ExpandXYZ(0.01,0.01,0.01);
			if(oGrown.Contains(p))
				bNear=true;
		}
		bool bHit = oBox.HasIntersectionWithLineSegment(p1,p2);
		if(bInside)
			EXPECT_TRUE(bHit)<<"case "<<n<<": "<<p1.toString()<<" - "<<p2.toString();
		if(!bNear)
			EXPECT_FALSE(bHit)<<"case "<<n<<": "<<p1.toString()<<" - "<<p2.toString();
		if(bHit) nHits++;
	}
	EXPECT_GT(nHits,100);
	EXPECT_LT(nHits,1900);
}

TEST(BoxAgainstBox, SeparatedBoxesDoNotIntersect)
{
	CR3Box a(0.0,1.0,0.0,1.0,0.0,1.0), b(2.0,3.0,0.0,1.0,0.0,1.0);
	EXPECT_FALSE(a.HasIntersectionWithBox(b,CTransformation(1.0),CTransformation(1.0)));
	//moved onto a by the transformation
	CTranslation oMove(-1.5,0.0,0.0), oBack(1.5,0.0,0.0);
	EXPECT_TRUE(a.HasIntersectionWithBox(b,oMove,oBack));
}

// ---------------------------------------------------------------- boxes of a cylinder

TEST(CylinderBoxes, InnerBoxIsInscribedAndOuterBoxIsCircumscribed)
{
	const double dR = 2.0;
	CR3Box oInner = CR3Box::InnerBoxFromOZCylinder(dR,0.0,1.0);
	CR3Box oOuter = CR3Box::OuterBoxFromOZCylinder(dR,0.0,1.0);
	//the corners of the inner box lie on the circle, the sides of the outer box touch it
	EXPECT_NEAR(dR,sqrt(oInner.GetMaxPoint().GetX()*oInner.GetMaxPoint().GetX()+oInner.GetMaxPoint().GetY()*oInner.GetMaxPoint().GetY()),1E-12);
	EXPECT_NEAR(dR,oOuter.GetMaxPoint().GetX(),1E-12);
	EXPECT_NEAR(dR,oOuter.GetMaxPoint().GetY(),1E-12);
	EXPECT_LT(oInner.GetMaxPoint().GetX(),oOuter.GetMaxPoint().GetX());
}

// ---------------------------------------------------------------- box against a cylinder about OZ

static const double dRADIUS = 1.0;
static const double dHEIGHT = 4.0;

//the box is given in the frame of the cylinder moved/turned by oBoxPose (box frame -> cylinder frame)
static bool HitsCylinder(const CR3Box &oBox, const CTransformation &oBoxToCylinder, const CTransformation &oCylinderToBox)
{
	return oBox.HasIntersectionWithOZCylinder(dHEIGHT,dRADIUS,oCylinderToBox,oBoxToCylinder);
}

static bool HitsCylinder(const CR3Box &oBox)
{
	return HitsCylinder(oBox,CTransformation(1.0),CTransformation(1.0));
}

TEST(BoxAgainstCylinder, BoxFarAwayOrAboveOrBelow)
{
	EXPECT_FALSE(HitsCylinder(CR3Box(3.0,4.0,-1.0,1.0,1.0,2.0)));
	EXPECT_FALSE(HitsCylinder(CR3Box(-0.5,0.5,-0.5,0.5,dHEIGHT+0.01,dHEIGHT+1.0)))<<"above the top";
	EXPECT_FALSE(HitsCylinder(CR3Box(-0.5,0.5,-0.5,0.5,-1.0,-0.01)))<<"below the bottom";
	EXPECT_TRUE(HitsCylinder(CR3Box(-0.5,0.5,-0.5,0.5,dHEIGHT-0.01,dHEIGHT+1.0)))<<"dips into the top";
	EXPECT_TRUE(HitsCylinder(CR3Box(-0.5,0.5,-0.5,0.5,-1.0,0.01)))<<"reaches the bottom";
}

TEST(BoxAgainstCylinder, BoxInsideAndBoxAround)
{
	EXPECT_TRUE(HitsCylinder(CR3Box(-0.1,0.1,-0.1,0.1,1.0,1.2)))<<"a small box inside the cylinder";
	EXPECT_TRUE(HitsCylinder(CR3Box(-5.0,5.0,-5.0,5.0,-5.0,5.0)))<<"a big box around the whole cylinder";
	EXPECT_TRUE(HitsCylinder(CR3Box(0.2,0.4,-5.0,5.0,1.0,1.2)))<<"a bar through the cylinder, no vertex inside";
}

//the cylinder is round: between the inscribed and the circumscribed square only the circle decides
TEST(BoxAgainstCylinder, CornerRegionOfTheCircumscribedSquareIsFree)
{
	//the point (0.8,0.8) is inside the square [-1,1]^2 but 1.13 away from the axis
	EXPECT_FALSE(HitsCylinder(CR3Box(0.8,0.95,0.8,0.95,1.0,2.0)));
	EXPECT_TRUE(HitsCylinder(CR3Box(0.69,0.95,0.69,0.95,1.0,2.0)))<<"its corner (0.69,0.69) is 0.976 away from the axis";
}

//a big face against the side of the cylinder: no edge and no vertex of the box is near
TEST(BoxAgainstCylinder, FlatFaceAgainstTheSide)
{
	EXPECT_TRUE(HitsCylinder(CR3Box(0.8,9.0,-9.0,9.0,-9.0,9.0)))<<"0.2 deep";
	EXPECT_TRUE(HitsCylinder(CR3Box(0.999,9.0,-9.0,9.0,-9.0,9.0)));
	EXPECT_FALSE(HitsCylinder(CR3Box(1.001,9.0,-9.0,9.0,-9.0,9.0)));
	EXPECT_TRUE(HitsCylinder(CR3Box(-9.0,-0.999,-9.0,9.0,-9.0,9.0)));
	EXPECT_FALSE(HitsCylinder(CR3Box(-9.0,-1.001,-9.0,9.0,-9.0,9.0)));
	EXPECT_TRUE(HitsCylinder(CR3Box(-9.0,9.0,-9.0,-0.999,-9.0,9.0)));
	EXPECT_FALSE(HitsCylinder(CR3Box(-9.0,9.0,-9.0,-1.001,-9.0,9.0)));
}

//the same face turned by 45 deg about OZ: it meets the cylinder on the diagonal of the square
TEST(BoxAgainstCylinder, FlatFaceOnTheDiagonal)
{
	CRotation oTurn(CR3Vector(0.0,0.0,1.0),degrees_to_radians(45.0));
	CTransformation oBack = oTurn.Inverted();
	EXPECT_TRUE(HitsCylinder(CR3Box(0.999,9.0,-9.0,9.0,-9.0,9.0),oTurn,oBack));
	EXPECT_FALSE(HitsCylinder(CR3Box(1.001,9.0,-9.0,9.0,-9.0,9.0),oTurn,oBack));
	EXPECT_FALSE(HitsCylinder(CR3Box(1.3,9.0,-9.0,9.0,-9.0,9.0),oTurn,oBack))<<"1.3 away on the diagonal is still inside the circumscribed square";
}

//a tilted box that only cuts the rim of the top
TEST(BoxAgainstCylinder, TiltedBoxAtTheTopRim)
{
	CRotation oTilt(CR3Vector(0.0,1.0,0.0),degrees_to_radians(45.0));
	//the lowest edge of the tilted box runs along Y; it is moved to the rim point (1,0,4)
	CR3Box oBox(0.0,1.0,-3.0,3.0,0.0,1.0);
	for(int i=0;i<2;i++){
		double dGap = (i==0) ? -0.01 : 0.01;	//into the cylinder / away from it, along the outward diagonal
		CTransformation oPose = CTranslation(dRADIUS+dGap,0.0,dHEIGHT+dGap)*CTransformation(oTilt);
		CTransformation oBack = CTransformation(oTilt.Inverted())*CTranslation(-(dRADIUS+dGap),0.0,-(dHEIGHT+dGap));
		EXPECT_EQ(i==0,HitsCylinder(oBox,oPose,oBack))<<"gap "<<dGap;
	}
}

//random boxes and poses against a dense sampling of the box
TEST(BoxAgainstCylinder, AgreesWithPointSampling)
{
	srand(308);
	const int nGRID = 24;
	int nHits=0, nMisses=0;
	for(int n=0;n<400;n++){
		double r[12];
		for(int i=0;i<12;i++)
			r[i] = (double)rand()/(double)RAND_MAX;
		CR3Box oBox(-0.2-1.5*r[0],0.2+1.5*r[1],-0.2-1.5*r[2],0.2+1.5*r[3],-0.2-1.5*r[4],0.2+1.5*r[5]);
		CRotation oTurn(CR3Vector(r[6]-0.5,r[7]-0.5,r[8]-0.5+1E-3),6.0*r[9]);
		CTransformation oPose = CTranslation(5.0*(r[10]-0.5),5.0*(r[11]-0.5),2.0)*CTransformation(oTurn);
		CTransformation oBack = CTransformation(oTurn.Inverted())*CTranslation(-5.0*(r[10]-0.5),-5.0*(r[11]-0.5),-2.0);

		//the sampling: is a grid point of the box inside the cylinder / inside the cylinder grown by one grid step?
		CR3Point r3pMin = oBox.GetMinPoint(), r3pMax = oBox.GetMaxPoint();
		double dStep = oBox.GetMaxSize()/(double)nGRID*sqrt(3.0);
		bool bInside=false, bNear=false;
		for(int i=0;i<=nGRID && !bInside;i++)
			for(int j=0;j<=nGRID && !bInside;j++)
				for(int k=0;k<=nGRID && !bInside;k++){
					CR3Point p = oPose*CR3Point(r3pMin.GetX()+(r3pMax.GetX()-r3pMin.GetX())*i/nGRID
											  ,r3pMin.GetY()+(r3pMax.GetY()-r3pMin.GetY())*j/nGRID
											  ,r3pMin.GetZ()+(r3pMax.GetZ()-r3pMin.GetZ())*k/nGRID);
					double dRadial = sqrt(p.GetX()*p.GetX()+p.GetY()*p.GetY());
					if(dRadial<=dRADIUS && p.GetZ()>=0.0 && p.GetZ()<=dHEIGHT)
						bInside=true;
					if(dRadial<=dRADIUS+dStep && p.GetZ()>=-dStep && p.GetZ()<=dHEIGHT+dStep)
						bNear=true;
				}
		bool bHit = HitsCylinder(oBox,oPose,oBack);
		if(bInside)
			EXPECT_TRUE(bHit)<<"case "<<n<<": a point of the box is inside the cylinder";
		if(!bNear)
			EXPECT_FALSE(bHit)<<"case "<<n<<": no point of the box is even near the cylinder";
		if(bHit) nHits++; else nMisses++;
	}
	//the random cases must contain both kinds
	EXPECT_GT(nHits,50);
	EXPECT_GT(nMisses,50);
}
