// CRotation: the constructors, the algebra and the rotation that turns one vector into another.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "Rotation.h"

using namespace kin5ax;

static const double dTOLERANCE = 1E-12;

static double Determinant3x3(const CTransformation &t)
{
	return t.GetM(0,0)*(t.GetM(1,1)*t.GetM(2,2)-t.GetM(2,1)*t.GetM(1,2))
		  -t.GetM(1,0)*(t.GetM(0,1)*t.GetM(2,2)-t.GetM(2,1)*t.GetM(0,2))
		  +t.GetM(2,0)*(t.GetM(0,1)*t.GetM(1,2)-t.GetM(1,1)*t.GetM(0,2));
}

//the biggest difference between the elements of two matrices
static double Difference(const CTransformation &a, const CTransformation &b)
{
	double d = 0.0;
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++)
			d = std::max(d,fabs(a.GetM(i,j)-b.GetM(i,j)));
	return d;
}

//a rotation matrix: R*RT = I and det R = +1
static void ExpectIsRotation(const CTransformation &r, const std::string &sWhat)
{
	EXPECT_LT(Difference(r*r.T(),CTransformation(1.0)),dTOLERANCE)<<sWhat<<": the matrix is not orthonormal";
	EXPECT_NEAR(1.0,Determinant3x3(r),dTOLERANCE)<<sWhat;
}

static void ExpectVectorsNear(const CR3Vector &expected, const CR3Vector &actual, const std::string &sWhat)
{
	EXPECT_LT((expected-actual).GetLength(),dTOLERANCE)<<sWhat<<": expected "<<expected.toString()<<", actual "<<actual.toString();
}

static std::string AngleText(double dDegrees)
{
	char sz[64];
	sprintf(sz,"angle %G deg",dDegrees);
	return std::string(sz);
}

static const CR3Vector r3vX(1.0,0.0,0.0), r3vY(0.0,1.0,0.0), r3vZ(0.0,0.0,1.0);

TEST(Rotation, DefaultIsIdentity)
{
	EXPECT_LT(Difference(CRotation(),CTransformation(1.0)),dTOLERANCE);
}

TEST(Rotation, AxisAngleIsRightHanded)
{
	CRotation r(r3vZ,dPiDiv2);
	ExpectVectorsNear(r3vY,r*r3vX,"Z 90 deg turns X into Y");
	ExpectVectorsNear(-r3vX,r*r3vY,"Z 90 deg turns Y into -X");
	ExpectVectorsNear(r3vZ,r*r3vZ,"the axis stays");
	CRotation rX(r3vX,dPiDiv2);
	ExpectVectorsNear(r3vZ,rX*r3vY,"X 90 deg turns Y into Z");
	CRotation rY(r3vY,dPiDiv2);
	ExpectVectorsNear(r3vX,rY*r3vZ,"Y 90 deg turns Z into X");
}

TEST(Rotation, AxisAngleNormalisesTheAxis)
{
	EXPECT_LT(Difference(CRotation(CR3Vector(0.0,0.0,5.0),0.7),CRotation(r3vZ,0.7)),dTOLERANCE);
	EXPECT_LT(Difference(CRotation(CR3Vector(3.0,-4.0,12.0),1.1),CRotation(CR3Vector(3.0,-4.0,12.0).GetUnitVector(),1.1)),dTOLERANCE);
}

TEST(Rotation, ZeroAxisGivesIdentity)
{
	EXPECT_LT(Difference(CRotation(CR3Vector(0.0),1.0),CTransformation(1.0)),dTOLERANCE);
}

TEST(Rotation, IsARotationMatrixForAnyAxisAndAngle)
{
	const CR3Vector vAxes[] = {r3vX,r3vY,r3vZ,CR3Vector(1.0,1.0,1.0),CR3Vector(-2.0,0.5,3.0)};
	for(int a=0;a<5;a++)
		for(double dAngle=-3.0;dAngle<=3.0;dAngle+=0.37){
			CRotation r(vAxes[a],dAngle);
			ExpectIsRotation(r,"axis "+vAxes[a].toString());
			ExpectVectorsNear(vAxes[a].GetUnitVector(),r*vAxes[a].GetUnitVector(),"the axis stays");
		}
}

TEST(Rotation, DegreeConstructorsEqualTheRadianOne)
{
	CR3Vector axis(1.0,2.0,-0.5);
	EXPECT_LT(Difference(CRotation(axis,37.5,0.0),CRotation(axis,degrees_to_radians(37.5))),dTOLERANCE);
	EXPECT_LT(Difference(CRotation(axis,(int)40),CRotation(axis,degrees_to_radians(40.0))),dTOLERANCE);
	EXPECT_LT(Difference(CRotation(axis,(int)400),CRotation(axis,degrees_to_radians(40.0))),dTOLERANCE)<<"the int constructor takes the angle modulo 360";
}

TEST(Rotation, OZRotationEqualsAxisAngleAboutZ)
{
	for(double dAngle=-3.0;dAngle<=3.0;dAngle+=0.5)
		EXPECT_LT(Difference(CRotation::OZRotation(dAngle),CRotation(r3vZ,dAngle)),dTOLERANCE)<<dAngle;
}

TEST(Rotation, CompositionAddsAnglesAboutTheSameAxis)
{
	CR3Vector axis(0.3,-1.0,2.0);
	EXPECT_LT(Difference(CRotation(axis,0.4)*CRotation(axis,0.9),CRotation(axis,1.3)),dTOLERANCE);
}

TEST(Rotation, InvertedIsTheTranspose)
{
	CRotation r(CR3Vector(1.0,2.0,3.0),0.8);
	EXPECT_LT(Difference(r.Inverted(),r.T()),dTOLERANCE);
	EXPECT_LT(Difference(r*CRotation(r.Inverted()),CTransformation(1.0)),dTOLERANCE);
	EXPECT_LT(Difference(r.Inverted(),CRotation(CR3Vector(1.0,2.0,3.0),-0.8)),dTOLERANCE);
}

TEST(Rotation, GetAngleAndGetAxisGiveTheConstructorArguments)
{
	CR3Vector axis = CR3Vector(2.0,-1.0,0.5).GetUnitVector();
	CRotation r(axis,degrees_to_radians(50.0));
	EXPECT_NEAR(50.0,r.GetAngle(),1E-9)<<"GetAngle returns degrees";
	ExpectVectorsNear(axis,r.GetAxis().GetUnitVector(),"GetAxis is parallel to the axis");
}

TEST(Rotation, EulerAnglesRoundTrip)
{
	double dAlpha, dBeta, dGamma;
	CRotation(30.0,40.0,50.0).GetEulerAngles(dAlpha,dBeta,dGamma);
	EXPECT_NEAR(30.0,dAlpha,1E-9);
	EXPECT_NEAR(40.0,dBeta,1E-9);
	EXPECT_NEAR(50.0,dGamma,1E-9);
	ExpectIsRotation(CRotation(30.0,40.0,50.0),"Euler constructor");
}

//CRotation(v0,v1): "v0 is rotated to be v1"
class RotationFromTwoVectorsTest : public ::testing::TestWithParam<double>
{
};

TEST_P(RotationFromTwoVectorsTest, TurnsTheFirstVectorIntoTheSecond)
{
	double dAngle = degrees_to_radians(GetParam());
	//a general pair of unit vectors with the given angle between them
	CRotation rGeneral(CR3Vector(1.0,-2.0,0.7),0.9);
	CR3Vector v0 = rGeneral*r3vZ;
	CR3Vector v1 = rGeneral*CR3Vector(sin(dAngle),0.0,cos(dAngle));
	ASSERT_NEAR(dAngle,v0.UndirectedAngle(v1),1E-7);	//acos is inexact for tiny angles

	CRotation r(v0,v1);
	ExpectIsRotation(r,AngleText(GetParam()));
	ExpectVectorsNear(v1,r*v0,"v0 must become v1");
}

TEST_P(RotationFromTwoVectorsTest, AcceptsVectorsOfAnyLength)
{
	double dAngle = degrees_to_radians(GetParam());
	CR3Vector v0(0.0,0.0,3.0);
	CR3Vector v1(0.25*sin(dAngle),0.0,0.25*cos(dAngle));
	CRotation r(v0,v1);
	ExpectIsRotation(r,AngleText(GetParam()));
	ExpectVectorsNear(v1.GetUnitVector(),r*v0.GetUnitVector(),"v0 must become v1");
}

//0 and 180 deg are the degenerate cases (the vector product is zero)
INSTANTIATE_TEST_CASE_P(Angles, RotationFromTwoVectorsTest, ::testing::Values(0.0,1E-7,10.0,30.0,45.0,60.0,90.0,120.0,150.0,179.0,180.0));

//---- GetAxis and GetAngle as a representation of the rotation (KnownBugs #53)
static void ExpectSameRotation(const CRotation &oExpected, const CRotation &oActual, const std::string &sWhat)
{
	double d = 0.0;
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			d = std::max(d,fabs(oExpected.GetM(i,j)-oActual.GetM(i,j)));
	//the axis of a half turn comes out of a square root, so half of the digits are lost, and
	//an angle of a few nanoradians cannot be read back from a matrix at all
	EXPECT_NEAR(0.0,d,1E-7)<<sWhat;
}

TEST(Rotation, AxisAndAngleRebuildTheRotation)
{
	const double dAngles[] = {0.0,1E-7,15.0,45.0,90.0,120.0,179.0,179.999,180.0};
	CR3Vector vAxes[] = {CR3Vector(1.0,0.0,0.0),CR3Vector(0.0,1.0,0.0),CR3Vector(0.0,0.0,1.0)
		,CR3Vector(1.0,1.0,1.0),CR3Vector(-2.0,1.0,0.5),CR3Vector(0.0,-3.0,4.0)};
	for(size_t a=0;a<sizeof(dAngles)/sizeof(dAngles[0]);a++)
		for(size_t v=0;v<sizeof(vAxes)/sizeof(vAxes[0]);v++){
			CRotation oRotation(vAxes[v],dAngles[a],0.0);
			CR3Vector r3vAxis = oRotation.GetAxis();
			double dAngle = oRotation.GetAngle();
			EXPECT_NEAR(1.0,r3vAxis.GetLength(),1E-9)<<"the axis is a unit vector";
			ExpectSameRotation(oRotation,CRotation(r3vAxis,dAngle,0.0)
				,"axis "+size_to_string(v)+", "+double_to_string(dAngles[a])+" degrees");
		}
}

TEST(Rotation, AngleOfAHalfTurnIsAHalfTurn)
{
	CRotation oRotation(CR3Vector(0.0,1.0,0.0),180.0,0.0);
	EXPECT_NEAR(180.0,oRotation.GetAngle(),1E-9);
	ExpectVectorsNear(CR3Vector(0.0,1.0,0.0),oRotation.GetAxis(),"the axis of a half turn");
}

TEST(Rotation, AngleOfARotationTimesItsInverseIsZero)
{
	//the trace of the product is 3 plus a rounding error: acos of more than 1 is not a number
	for(int i=0;i<64;i++){
		CR3Vector r3vAxis(1.0+0.01*i,-0.3*i,0.7);
		CRotation oRotation(r3vAxis,0.1*i+0.05,0.0);
		CRotation oProduct(oRotation*CRotation(oRotation.Inverted()));
		double dAngle = oProduct.GetAngle();
		EXPECT_FALSE(dAngle!=dAngle)<<"GetAngle returned a NaN at step "<<i;
		//acos is flat at 1: a rounding error of 1E-16 in the matrix is 1E-6 degrees in the angle
		EXPECT_NEAR(0.0,dAngle,1E-4);
	}
}

TEST(Rotation, AxisOfNoRotationIsUsable)
{
	CRotation oIdentity(CR3Vector(0.0,0.0,1.0),0.0,0.0);
	EXPECT_NEAR(0.0,oIdentity.GetAngle(),1E-12);
	EXPECT_NEAR(1.0,oIdentity.GetAxis().GetLength(),1E-12)<<"a unit vector, not the zero vector";
}
