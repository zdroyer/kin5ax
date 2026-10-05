// CM4x4: the checked element access.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "M4x4.h"
#include "Rotation.h"
#include "Translation.h"
#include "Scale.h"
#include "R3Point.h"

using namespace kin5ax;

TEST(M4x4, GetAtAndSetAtReachEveryElement)
{
	CM4x4 m(1.0);
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++){
			EXPECT_DOUBLE_EQ((i==j) ? 1.0 : 0.0,m.GetAt(i,j))<<"identity at "<<i<<","<<j;
			m.SetAt(i,j,10.0*i+j);
		}
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++){
			EXPECT_DOUBLE_EQ(10.0*i+j,m.GetAt(i,j))<<i<<","<<j;
			EXPECT_DOUBLE_EQ(10.0*i+j,m.GetM(i,j))<<i<<","<<j;
		}
}

//KnownBugs #29: the column index was never checked against 0 (i<0 was tested twice)
TEST(M4x4, IndexesOutOfRangeAreRejected)
{
	CM4x4 m(1.0);
	EXPECT_THROW(m.GetAt(-1,0),CArgsMyException);
	EXPECT_THROW(m.GetAt(4,0),CArgsMyException);
	EXPECT_THROW(m.GetAt(0,-1),CArgsMyException)<<"negative column";
	EXPECT_THROW(m.GetAt(0,4),CArgsMyException);
	EXPECT_THROW(m.SetAt(-1,0,0.0),CArgsMyException);
	EXPECT_THROW(m.SetAt(4,0,0.0),CArgsMyException);
	EXPECT_THROW(m.SetAt(0,-1,0.0),CArgsMyException)<<"negative column";
	EXPECT_THROW(m.SetAt(0,4,0.0),CArgsMyException);
	EXPECT_THROW(m.GetM(3,-1),CArgsMyException);
	EXPECT_NO_THROW(m.GetAt(0,0));
	EXPECT_NO_THROW(m.GetAt(3,3));
	EXPECT_NO_THROW(m.SetAt(3,0,0.0));
	EXPECT_NO_THROW(m.SetAt(0,3,0.0));
}

//KnownBugs #52: the inverse of a general transformation used to be the identity, silently
TEST(Transformation, InvertedSaysItIsNotImplemented)
{
	CTransformation t(1.0);
	EXPECT_THROW(t.Inverted(),CGeneratorException)<<"the identity is not an answer for every transformation";
	CTransformation tMoved = CTranslation(1.0,2.0,3.0);
	EXPECT_THROW(tMoved.Inverted(),CGeneratorException);
	EXPECT_THROW(CM4x4(1.0).Inverted(),CGeneratorException)<<"and the matrix inverse is not implemented either";
}

//what to use instead: the classes that know their own inverse
TEST(Transformation, TheKnownInversesAreCorrect)
{
	CR3Point p(0.3,-0.7,1.1);
	CRotation r(CR3Vector(1.0,2.0,-0.5),0.7);
	CR3Point pBack = CTransformation(r.Inverted()) * (r * p);
	EXPECT_NEAR(0.0,d_E(p,pBack),1E-12)<<"a rotation inverts to its transpose";

	CTranslation t(1.0,2.0,3.0);
	pBack = CTransformation(t.Inverted()) * (t * p);
	EXPECT_NEAR(0.0,d_E(p,pBack),1E-12);

	CScale s(2.0,4.0,0.5);
	pBack = s.Inverted() * (s * p);
	EXPECT_NEAR(0.0,d_E(p,pBack),1E-12);
}
