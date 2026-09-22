// CCutterLocationData::LoadFile: the parsing of the GOTO/x,y,z,i,j,k lines and the line bookkeeping.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "CutterLocationData.h"

//writes the text as it is (binary: the line ends are not touched) and loads it
static void LoadText(CCutterLocationData &oData, const std::string &sText, const char *szName="kin5axTests_parsing.cld")
{
	std::string sFileName = TempFilePath(szName);
	FILE *pFile = fopen(sFileName.c_str(),"wb");
	ASSERT_TRUE(pFile!=NULL);
	fwrite(sText.data(),1,sText.size(),pFile);
	fclose(pFile);
	try{
		oData.LoadFile(sFileName,szName);
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	remove(sFileName.c_str());
}

TEST(CLDataParsing, ReadsAllValuesOfALine)
{
	CCutterLocationData oData;
	LoadText(oData,"GOTO/1.5,-2.25,3e2,0,0,1\n");
	ASSERT_EQ((size_t)1,oData.GetLinesCount());
	EXPECT_EQ((size_t)1,oData.GetLocationsCount());
	EXPECT_EQ((size_t)0,oData.GetErrorsCount());
	CR3Point p;
	CR3Vector v;
	ASSERT_TRUE(oData.GetLocation(p,v,0));
	EXPECT_DOUBLE_EQ(1.5,p.GetX());
	EXPECT_DOUBLE_EQ(-2.25,p.GetY());
	EXPECT_DOUBLE_EQ(300.0,p.GetZ());
	EXPECT_DOUBLE_EQ(1.0,v.GetZ());
	EXPECT_STREQ("GOTO/1.5,-2.25,3e2,0,0,1",oData.GetLineText(0));
}

TEST(CLDataParsing, LineStatuses)
{
	CCutterLocationData oData;
	LoadText(oData,"MSYS/0,0,0,1,0,0,0,1,0\n\nGOTO/1,2,3,0,0,1\nGOTO/1,2,3,0,0\nGOTO/1,2,3,0,0,0\nsomething else\n");
	ASSERT_EQ((size_t)6,oData.GetLinesCount());
	EXPECT_EQ(CL_LINE_MSYS,oData.GetLine(0).m_status);
	EXPECT_EQ(CL_LINE_EMPTY,oData.GetLine(1).m_status);
	EXPECT_EQ(CL_LINE_OK,oData.GetLine(2).m_status);
	EXPECT_EQ(CL_LINE_ERROR,oData.GetLine(3).m_status)<<"five values";
	EXPECT_EQ(CL_LINE_ERROR,oData.GetLine(4).m_status)<<"zero orientation";
	EXPECT_EQ(CL_LINE_ERROR,oData.GetLine(5).m_status)<<"not a GOTO";
	EXPECT_EQ((size_t)1,oData.GetLocationsCount());
	EXPECT_EQ((size_t)3,oData.GetErrorsCount());
	EXPECT_EQ((size_t)3,oData.GetLine(3).m_nLineNumber);
}

//KnownBugs #9: one number out of range must not spoil the following lines
TEST(CLDataParsing, OverflowSpoilsOnlyItsOwnLine)
{
	CCutterLocationData oData;
	LoadText(oData,"GOTO/1e999,0,0,0,0,1\nGOTO/1,2,3,0,0,1\nGOTO/4,5,6,0,0,1\n");
	ASSERT_EQ((size_t)3,oData.GetLinesCount());
	EXPECT_EQ(CL_LINE_ERROR,oData.GetLine(0).m_status);
	EXPECT_EQ(CL_LINE_OK,oData.GetLine(1).m_status);
	EXPECT_EQ(CL_LINE_OK,oData.GetLine(2).m_status);
	EXPECT_EQ((size_t)2,oData.GetLocationsCount());
	EXPECT_EQ((size_t)1,oData.GetErrorsCount());
}

//KnownBugs #10: a number may begin with a point or a sign
TEST(CLDataParsing, NumbersMayBeginWithAPointOrASign)
{
	CCutterLocationData oData;
	LoadText(oData,"GOTO/.5,-.25,+2,0,0,1\n");
	ASSERT_EQ((size_t)1,oData.GetLocationsCount());
	CR3Point p;
	CR3Vector v;
	ASSERT_TRUE(oData.GetLocation(p,v,0));
	EXPECT_DOUBLE_EQ(0.5,p.GetX());
	EXPECT_DOUBLE_EQ(-0.25,p.GetY());
	EXPECT_DOUBLE_EQ(2.0,p.GetZ());
}

//KnownBugs #11: the last line counts even without a newline at the end of the file
TEST(CLDataParsing, LastLineWithoutNewline)
{
	CCutterLocationData oData;
	LoadText(oData,"GOTO/1,2,3,0,0,1\nGOTO/4,5,6,0,0,1");
	EXPECT_EQ((size_t)2,oData.GetLinesCount());
	EXPECT_EQ((size_t)2,oData.GetLocationsCount());
	EXPECT_STREQ("GOTO/4,5,6,0,0,1",oData.GetLineText(1));
}

//Windows line ends are accepted whatever the platform
TEST(CLDataParsing, WindowsLineEnds)
{
	CCutterLocationData oData;
	LoadText(oData,"GOTO/1,2,3,0,0,1\r\n\r\nGOTO/4,5,6,0,0,1\r\n");
	ASSERT_EQ((size_t)3,oData.GetLinesCount());
	EXPECT_EQ(CL_LINE_OK,oData.GetLine(0).m_status);
	EXPECT_EQ(CL_LINE_EMPTY,oData.GetLine(1).m_status);
	EXPECT_EQ(CL_LINE_OK,oData.GetLine(2).m_status);
	EXPECT_STREQ("GOTO/1,2,3,0,0,1",oData.GetLineText(0));
}

TEST(CLDataParsing, EmptyFileAndMissingFile)
{
	CCutterLocationData oData;
	LoadText(oData,"");
	EXPECT_EQ((size_t)0,oData.GetLinesCount());
	EXPECT_FALSE(oData.HasMoreLocations());
	EXPECT_THROW(oData.LoadFile(TempFilePath("kin5axTests_no_such_file.cld"),"none"),CGeneratorException);
}

//KnownBugs #27: the line index is checked, and a copy of the object owns its own text
TEST(CLDataParsing, LineIndexIsCheckedAndCopiesAreIndependent)
{
	CCutterLocationData oCopy;
	{
		CCutterLocationData oData;
		LoadText(oData,"GOTO/1,2,3,0,0,1\n");
		EXPECT_NO_THROW(oData.GetLine(0));
		EXPECT_THROW(oData.GetLine(1),CGeneratorException);
		EXPECT_THROW(oData.GetLineText(1),CGeneratorException);
		oCopy = oData;
	}
	EXPECT_EQ((size_t)1,oCopy.GetLocationsCount());
	EXPECT_STREQ("GOTO/1,2,3,0,0,1",oCopy.GetLineText(0));
}

TEST(CLDataParsing, IteratorSkipsTheBadLines)
{
	CCutterLocationData oData;
	LoadText(oData,"GOTO/1,2,3,0,0,1\nbad\nGOTO/4,5,6,0,0,1\n");
	oData.ResetLocationIterator();
	CR3Point p;
	CR3Vector v;
	ASSERT_TRUE(oData.GetNextLocation(p,v));
	EXPECT_DOUBLE_EQ(1.0,p.GetX());
	ASSERT_TRUE(oData.GetNextLocation(p,v));
	EXPECT_DOUBLE_EQ(4.0,p.GetX());
	EXPECT_FALSE(oData.GetNextLocation(p,v));
	EXPECT_NEAR(sqrt(27.0),oData.GetTrajectoryLength(),1E-12);
}
