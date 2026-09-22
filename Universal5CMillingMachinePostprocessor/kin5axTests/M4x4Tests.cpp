// CM4x4: the checked element access.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "M4x4.h"

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
