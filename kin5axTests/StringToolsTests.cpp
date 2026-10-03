// string_tools and the toString methods: no fixed buffers, any length.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "string_tools.h"
#include "R3Point.h"
#include "R3Vector.h"
#include "ChainParameter.h"

using namespace kin5ax;

TEST(StringPrintf, FormatsAnyLength)
{
	EXPECT_EQ("",string_printf(""));
	EXPECT_EQ("a=1, b=2.50",string_printf("a=%d, b=%.2f",1,2.5));
	std::string sLong = string_printf("%.0f",1E300);
	EXPECT_EQ((size_t)301,sLong.size());
	EXPECT_EQ('1',sLong[0]);
	//two results at once: no shared buffer
	std::string a = string_printf("%d",1), b = string_printf("%d",2);
	EXPECT_EQ("1",a);
	EXPECT_EQ("2",b);
}

TEST(StringPrintf, NumberConversions)
{
	EXPECT_EQ("12345678901",size_to_string(12345678901ULL));
	EXPECT_EQ("7\r\n",size_to_string(7,true));
	EXPECT_EQ("0.1",double_to_string(0.1));
	EXPECT_EQ("1.2345679E+20",double_to_string(1.23456789E20));
	EXPECT_EQ("-2.5\r\n",double_to_string(-2.5,true));
}

TEST(StringTools, FixedToString)
{
	EXPECT_EQ("0.500000",fixed_to_string(0.5,6));
	EXPECT_EQ("-12.25",fixed_to_string(-12.25,2));
	EXPECT_EQ("3",fixed_to_string(3.4,0));
	EXPECT_EQ("1000000.000000",fixed_to_string(1E6,6));
}

//the chain files: every double must come back unchanged (KnownBugs #65: %.8G lost the digits of 1/sqrt(3))
TEST(StringTools, DoubleToStringExactReadsBack)
{
	EXPECT_EQ("0.1",double_to_string_exact(0.1));
	EXPECT_EQ("-30",double_to_string_exact(-30.0));
	EXPECT_EQ("0.57735026919",double_to_string_exact(0.57735026919));
	EXPECT_EQ("-29.999999999999996",double_to_string_exact(-29.999999999999996));
	double dValues[] = {0.0, 1.0/3.0, -1.0/7.0, 1.0/sqrt(3.0), 1.23456789E20, 2.5E-12, radians_to_degrees(degrees_to_radians(-30.0))};
	for(double d : dValues){
		std::string s = double_to_string_exact(d);
		EXPECT_EQ(d,strtod(s.c_str(),NULL))<<s;
	}
}

//KnownBugs #18: a 35-byte static buffer overflowed with coordinates of 4 and more digits
TEST(ToString, PointWithBigCoordinates)
{
	EXPECT_EQ("(1.5000, -2.2500, 0.0000)",CR3Point(1.5,-2.25,0.0).toString());
	EXPECT_EQ("(123456789.1250, -123456789.1250, 123456789.1250)",CR3Point(123456789.125,-123456789.125,123456789.125).toString());
	std::string s = CR3Point(1E300,1E300,1E300).toString();
	EXPECT_GT(s.size(),(size_t)900);
	EXPECT_EQ("(1, -2, 3)",CR3Point(1.4,-2.4,3.4).toStringInt());
}

TEST(ToString, VectorAndParameter)
{
	EXPECT_EQ("[0.0000; 1.0000; -1.0000]",CR3Vector(0.0,1.0,-1.0).toString());
	EXPECT_GT(CR3Vector(1E300,0.0,0.0).toString().size(),(size_t)300);
	CChainParameter oValue(RADIAN,0.5), oAny(RADIAN,0.0,CHAIN_PARAMETER_ANY_NUMBER);
	EXPECT_EQ("0.5",oValue.toString());
	EXPECT_EQ("ANY",oAny.toString());
}
