// The exceptions of kin5ax: a std::exception for every host, the message, the throwing place.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "GeneratorException.h"
#include "ArgsMyException.h"
#include "DivByZeroMyException.h"
#include "MemoryMyException.h"

using namespace kin5ax;

static int g_nThrowLine = 0;

static void ThrowArgs()
{
	g_nThrowLine = __LINE__; MY_THROW(CArgsMyException,"bad argument");
}

static void ThrowDivByZero()
{
	MY_THROW(CDivByZeroMyException,"zero");
}

static void ThrowMemory()
{
	MY_THROW(CMemoryMyException,"memory");
}

TEST(Exceptions, AreStdExceptionsWithTheMessageAsWhat)
{
	try{
		ThrowArgs();
		FAIL()<<"nothing thrown";
	} catch (std::exception &e){
		EXPECT_STREQ("bad argument",e.what());
	}
}

TEST(Exceptions, DerivedTypesAreCaughtAsTheBase)
{
	EXPECT_THROW(ThrowArgs(),CGeneratorException);
	EXPECT_THROW(ThrowArgs(),CArgsMyException);
	EXPECT_THROW(ThrowDivByZero(),CGeneratorException);
	EXPECT_THROW(ThrowDivByZero(),std::exception);
	EXPECT_THROW(ThrowMemory(),CGeneratorException);
	EXPECT_THROW(ThrowMemory(),std::exception);
}

TEST(Exceptions, MyThrowRecordsThePlace)
{
	try{
		ThrowArgs();
		FAIL()<<"nothing thrown";
	} catch (CGeneratorException &e){
		EXPECT_EQ("bad argument",e.Message());
		EXPECT_NE(std::string::npos,e.GetThrowingPlace().find("ExceptionsTests.cpp"))<<e.GetThrowingPlace();
		EXPECT_NE(std::string::npos,e.GetThrowingPlace().find("Line: "+std::to_string(g_nThrowLine)))<<e.GetThrowingPlace();
		EXPECT_EQ(std::string::npos,e.GetThrowingPlace().find(std::string(1,'\0')))<<"no embedded NUL";
		EXPECT_NE(std::string::npos,e.GetThrowingMethod().find("ThrowArgs"))<<e.GetThrowingMethod();
	}
}

TEST(Exceptions, DescribeTheirKind)
{
	try{ ThrowArgs(); } catch (CGeneratorException &e){ EXPECT_EQ("Invalid function argument(s)",e.GetDescription()); }
	try{ ThrowDivByZero(); } catch (CGeneratorException &e){ EXPECT_EQ("Division by zero",e.GetDescription()); }
	try{ ThrowMemory(); } catch (CGeneratorException &e){ EXPECT_EQ("Memory allocation failed",e.GetDescription()); }
}

TEST(Exceptions, ACopyKeepsTheMessage)
{
	try{
		ThrowArgs();
	} catch (CGeneratorException &e){
		CGeneratorException oCopy(e);		//the copy must be complete
		EXPECT_EQ("bad argument",oCopy.Message());
		EXPECT_STREQ("bad argument",oCopy.what());
	}
}
