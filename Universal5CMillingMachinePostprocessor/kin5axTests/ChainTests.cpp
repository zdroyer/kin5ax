// Tests run once for every kinematic chain (.kc) file of kin5axTests/TestChains.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SolverRoundTrip.h"
#include "MillingToolBall.h"
#include "string_tools.h"

class ChainFileTest : public ::testing::TestWithParam<string>
{
protected:
	//the exceptions of kin5ax do not derive from std::exception: report their message
	CChain Load()
	{
		try{
			return LoadChain(GetParam());
		} catch (CGeneratorException &e){
			ADD_FAILURE()<<"exception while loading: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
		}
		return CChain();
	}
};

struct ChainFileTestName
{
	string operator()(const ::testing::TestParamInfo<string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST(ChainFiles, AreFound)
{
	EXPECT_GE(GetChainFiles().size(),(size_t)1)<<"no .kc file in "<<GetTestChainsDir()<<" (the build copies kin5axTests/TestChains there)";
}

//the reader reports every problem as CGeneratorException, nothing else leaves it
static string LoadError(const string &sFilePath)
{
	try{
		LoadChain(sFilePath);
	} catch (CGeneratorException &e){
		return e.Message();
	}
	return string();
}

TEST(ChainFiles, MissingFileIsReported)
{
	string sError = LoadError(GetTestChainsDir()+"no_such_chain.kc");
	EXPECT_NE(string::npos,sError.find("no_such_chain.kc"))<<sError;
}

TEST(ChainFiles, BrokenXmlIsReportedWithTheLine)
{
	string sFilePath = GetTestChainsDir()+"kin5axTests_broken.kc";
	FILE *pFile = fopen(sFilePath.c_str(),"wt");
	ASSERT_TRUE(pFile!=NULL);
	fputs("<?xml version=\"1.0\"?>\n<kinematic_chain>\n<tool_part>\n<segment name=\"x\" type=\"tool\">\n</kinematic_chain>\n",pFile);
	fclose(pFile);
	string sError = LoadError(sFilePath);
	remove(sFilePath.c_str());
	EXPECT_NE(string::npos,sError.find("line 5"))<<sError;
}

TEST(ChainFiles, WrongRootElementIsReported)
{
	string sFilePath = GetTestChainsDir()+"kin5axTests_wrong_root.kc";
	FILE *pFile = fopen(sFilePath.c_str(),"wt");
	ASSERT_TRUE(pFile!=NULL);
	fputs("<?xml version=\"1.0\"?>\n<not_a_chain/>\n",pFile);
	fclose(pFile);
	string sError = LoadError(sFilePath);
	remove(sFilePath.c_str());
	EXPECT_NE(string::npos,sError.find("root node"))<<sError;
}

TEST(XmlEscape, MarkupCharactersBecomeEntities)
{
	EXPECT_EQ("",xml_escape(""));
	EXPECT_EQ("Table_C1",xml_escape("Table_C1"));
	EXPECT_EQ("&amp;",xml_escape("&"));
	EXPECT_EQ("&lt;a&gt; &quot;b&quot; &apos;c&apos; &amp;amp;",xml_escape("<a> \"b\" 'c' &amp;"));
}

//KnownBugs #28: a segment name with markup characters must survive toXML -> file -> CChainXmlReader
TEST(ChainFiles, SegmentNamesAreEscapedInXml)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain;
	string sNames[2] = {"A \"quoted\" <name> & more", "it's <b>"};
	try{
		oChain = LoadChain(GetChainFiles()[0]);
		ASSERT_GE(oChain.GetNumberOfRegularLinks(),(size_t)2);
		oChain[0].SetName(sNames[0]);
		oChain[1].SetName(sNames[1]);
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message();
	}
	string sXML = oChain.toXML();
	EXPECT_NE(string::npos,sXML.find("name=\"A &quot;quoted&quot; &lt;name&gt; &amp; more\""))<<sXML;
	EXPECT_EQ(string::npos,sXML.find("<name>"))<<"an unescaped name";

	string sFilePath = GetTestChainsDir()+"kin5axTests_escaped_names.kc";
	FILE *pFile = fopen(sFilePath.c_str(),"wb");
	ASSERT_TRUE(pFile!=NULL);
	fwrite(sXML.data(),1,sXML.size(),pFile);
	fclose(pFile);
	string sError;
	try{
		CChain oRead = LoadChain(sFilePath);
		EXPECT_EQ(sNames[0],oRead[0].GetName());
		EXPECT_EQ(sNames[1],oRead[1].GetName());
		EXPECT_EQ(oChain.GetNumberOfRegularLinks(),oRead.GetNumberOfRegularLinks());
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	remove(sFilePath.c_str());
	EXPECT_EQ("",sError)<<"the written file does not read back";
}

//KnownBugs #30: after a rename the comparison restarted at 1, so a duplicate of the first name survived
TEST(ChainSegments, NamesAreMadeUniqueAndNonEmpty)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain = LoadChain(GetChainFiles()[0]);
	int n = (int)oChain.GetNumberOfRegularLinks();
	ASSERT_GE(n,3);
	oChain[0].SetName("A_1");
	oChain[1].SetName("A");
	oChain[2].SetName("A");
	for(int i=3;i<n;i++)
		oChain[i].SetName("");
	oChain.CheckAndCorrectSegmentsNames();
	for(int i=0;i<n;i++){
		EXPECT_FALSE(oChain[i].GetName().empty())<<"segment "<<i;
		for(int j=0;j<i;j++)
			EXPECT_NE(oChain[j].GetName(),oChain[i].GetName())<<"segments "<<j<<" and "<<i;
	}
}

//KnownBugs #31: the material holder part is stored from the base towards the holder, so the parameter
//of its segment i is the (size-2-i)th parameter of the chain, not the (size-1-i)th
TEST(ChainSegments, ActualParameterOfEveryPart)
{
	vector<string> vFiles = GetChainFiles();
	ASSERT_GE(vFiles.size(),(size_t)1);
	size_t nMaterialSideJoints = 0;
	for(size_t f=0;f<vFiles.size();f++){
		CChain oChain = LoadChain(vFiles[f]);
		size_t n = oChain.GetNumberOfRegularLinks();
		CChainConfiguration oConfig((int)n);
		for(size_t k=0;k<n;k++)
			oConfig[k].SetValue(0.1*(k+1));
		oChain.SetParameters(oConfig);
		for(size_t i=0;i+1<oChain.m_vMaterialHolderPart.size();i++,nMaterialSideJoints++)
			EXPECT_DOUBLE_EQ(oChain.m_vMaterialHolderPart[i].m_dActualParameterValue,oChain.GetActualParameter(i,false))<<GetTestName(vFiles[f])<<": material holder part segment "<<i;
		EXPECT_DOUBLE_EQ(0.0,oChain.GetActualParameter(oChain.m_vMaterialHolderPart.size()-1,false))<<"the material holder has no parameter";
		for(size_t i=0;i+1<oChain.m_vToolPart.size();i++)
			EXPECT_DOUBLE_EQ(oChain.m_vToolPart[i].m_dActualParameterValue,oChain.GetActualParameter(i,true))<<GetTestName(vFiles[f])<<": tool part segment "<<i;
		EXPECT_DOUBLE_EQ(0.0,oChain.GetActualParameter(oChain.m_vToolPart.size()-1,true))<<"the tool has no parameter";
	}
	EXPECT_GT(nMaterialSideJoints,(size_t)0)<<"no test chain with a joint on the material side";
}

TEST_P(ChainFileTest, LoadsAsValid5CChain)
{
	CChain oChain = Load();
	EXPECT_TRUE(oChain.Is5CChain());
	EXPECT_EQ((size_t)3,oChain.GetNumberOfPrismaticLinks());
	EXPECT_EQ((size_t)2,oChain.GetNumberOfRevoluteLinks());
}

TEST_P(ChainFileTest, SolverRoundTrip)
{
	CChain oChain = Load();
	ASSERT_TRUE(oChain.Is5CChain());
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);	//the default tool of the application
	try{
		CSolverRoundTripResult oResult = RunSolverRoundTrip(oChain,oTool,2000,1E-9);
		EXPECT_GT(oResult.m_nTargets,(size_t)50)<<"too few collision-free poses to judge the solver";
		EXPECT_EQ((size_t)0,oResult.m_nNoSolution)<<oResult.m_sFirstProblem;
		EXPECT_EQ((size_t)0,oResult.m_nWrongSolutions)<<oResult.m_sFirstProblem;
		EXPECT_EQ((size_t)0,oResult.m_nNotRecovered)<<oResult.m_sFirstProblem;
		RecordProperty("targets",(int)oResult.m_nTargets);
		RecordProperty("skipped_colliding_poses",(int)oResult.m_nCollisions);
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
}

INSTANTIATE_TEST_CASE_P(AllChains, ChainFileTest, ::testing::ValuesIn(GetChainFiles()), ChainFileTestName());
