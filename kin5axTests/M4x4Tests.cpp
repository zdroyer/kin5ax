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

//KnownBugs #52: the inverse of a general transformation used to be the identity, silently, and then
//threw for want of a caller. The workpiece holder of a chain is that caller.
TEST(Transformation, TheRigidInverseUndoesTheTransformation)
{
	const CR3Point p(0.3,-0.7,1.1);
	//the identity inverts to itself, which is the one case the old answer got right
	EXPECT_NEAR(0.0,d_E(p,CTransformation(1.0).Inverted()*(CTransformation(1.0)*p)),1E-12);
	//a turn, a move, and a turn after a move: every rigid frame a machine is measured with
	const CTransformation vTests[] = {
		CTransformation(CTranslation(1.0,2.0,3.0))
		,CTransformation(CRotation(CR3Vector(1.0,2.0,-0.5),0.7))
		,CTransformation(CTranslation(0.0,0.0,0.08))*CTransformation(CRotation(CR3Vector(0.0,0.0,1.0),0.9))
		,CTransformation(CRotation(CR3Vector(0.0,1.0,0.0),-1.3))*CTransformation(CTranslation(-2.0,0.5,7.0))
	};
	for(size_t i=0;i<sizeof(vTests)/sizeof(vTests[0]);i++){
		SCOPED_TRACE(i);
		const CTransformation oBack = vTests[i].Inverted();
		EXPECT_NEAR(0.0,d_E(p,oBack*(vTests[i]*p)),1E-12)<<"there and back again";
		EXPECT_NEAR(0.0,d_E(p,vTests[i]*(oBack*p)),1E-12)<<"and back and there";
		//a vector carries no translation, so the inverse of the block alone has to undo it
		const CR3Vector v(0.2,0.9,-0.4);
		EXPECT_NEAR(0.0,(v-(oBack*(vTests[i]*v))).GetLength(),1E-12);
	}
}

TEST(Transformation, OnlyARigidTransformationHasThatInverse)
{
	//a scale is not rigid: its inverse is not its transpose, and answering as though it were would be
	//wrong in a way nothing downstream could see
	EXPECT_THROW(CTransformation(CScale(2.0,4.0,0.5)).Inverted(),CGeneratorException);
	EXPECT_THROW((CTransformation(CTranslation(1.0,0.0,0.0))*CTransformation(CScale(2.0,2.0,2.0)))
		.Inverted(),CGeneratorException);
	EXPECT_THROW(CM4x4(1.0).Inverted(),CGeneratorException)<<"the matrix inverse is not implemented";
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
