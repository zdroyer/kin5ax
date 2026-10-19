// Tests run once for every kinematic chain (.kc) file of kin5axTests/TestChains.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SolverRoundTrip.h"
#include "MillingToolBall.h"
#include "ChainJson.h"
#include "string_tools.h"
#include "5CPostprocessor.h"

using namespace kin5ax;

class ChainFileTest : public ::testing::TestWithParam<std::string>
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
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST(ChainFiles, AreFound)
{
	EXPECT_GE(GetChainFiles().size(),(size_t)1)<<"no .kc file in "<<GetTestChainsDir()<<" (the build copies kin5axTests/TestChains there)";
}

//the reader reports every problem as CGeneratorException, nothing else leaves it
static std::string LoadError(const std::string &sFilePath)
{
	try{
		LoadChain(sFilePath);
	} catch (CGeneratorException &e){
		return e.Message();
	}
	return std::string();
}

TEST(ChainFiles, MissingFileIsReported)
{
	std::string sError = LoadError(GetTestChainsDir()+"no_such_chain.kc");
	EXPECT_NE(std::string::npos,sError.find("no_such_chain.kc"))<<sError;
}

TEST(ChainFiles, BrokenJsonIsReportedWithTheLine)
{
	std::string sFilePath = GetTestChainsDir()+"kin5axTests_broken.kc.json";
	FILE *pFile = fopen(sFilePath.c_str(),"wt");
	ASSERT_TRUE(pFile!=NULL);
	fputs("{\n  \"format\": \"kin5ax-chain\",\n  \"version\": 1,\n  \"unit\": \"m\",\n  \"tool_branch\"\n}\n",pFile);
	fclose(pFile);
	std::string sError = LoadError(sFilePath);
	remove(sFilePath.c_str());
	EXPECT_NE(std::string::npos,sError.find("line 6"))<<sError;
}

TEST(ChainFiles, SomethingThatIsNotAChainIsReported)
{
	std::string sFilePath = GetTestChainsDir()+"kin5axTests_wrong_root.kc.json";
	FILE *pFile = fopen(sFilePath.c_str(),"wt");
	ASSERT_TRUE(pFile!=NULL);
	fputs("{ \"format\": \"something-else\", \"version\": 1 }\n",pFile);
	fclose(pFile);
	std::string sError = LoadError(sFilePath);
	remove(sFilePath.c_str());
	EXPECT_NE(std::string::npos,sError.find("kinematic chain"))<<sError;
}

TEST(XmlEscape, MarkupCharactersBecomeEntities)
{
	EXPECT_EQ("",xml_escape(""));
	EXPECT_EQ("Table_C1",xml_escape("Table_C1"));
	EXPECT_EQ("&amp;",xml_escape("&"));
	EXPECT_EQ("&lt;a&gt; &quot;b&quot; &apos;c&apos; &amp;amp;",xml_escape("<a> \"b\" 'c' &amp;"));
}

//KnownBugs #28: a name with characters that mean something to the format must survive a write
//and a read. The format is JSON now, so the quotes and the backslashes are what matter.
TEST(ChainFiles, SegmentNamesSurviveTheFile)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	CChain oChain;
	std::string sNames[2] = {"A \"quoted\" <name> & more", "it's <b>"};
	try{
		oChain = LoadChain(GetChainFiles()[0]);
		ASSERT_GE(oChain.GetNumberOfRegularLinks(),(size_t)2);
		oChain[0].SetName(sNames[0]);
		oChain[1].SetName(sNames[1]);
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message();
	}
	std::string sJson = CChainJsonWriter::ToJson(oChain);
	EXPECT_NE(std::string::npos,sJson.find("A \\\"quoted\\\" <name> & more"))<<sJson;

	std::string sFilePath = GetTestChainsDir()+"kin5axTests_escaped_names.kc.json";
	FILE *pFile = fopen(sFilePath.c_str(),"wb");
	ASSERT_TRUE(pFile!=NULL);
	fwrite(sJson.data(),1,sJson.size(),pFile);
	fclose(pFile);
	std::string sError;
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
	std::vector<std::string> vFiles = GetChainFiles();
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

//KnownBugs #32: a singular section with no solvable location took the front() of an empty vector
TEST_P(ChainFileTest, SingularSectionOutOfReachGivesNoSolution)
{
	CChain oChain = Load();
	ASSERT_TRUE(oChain.Is5CChain());
	if(!oChain.HasParameterConstraints()){
		RecordProperty("skipped","no joint limits: everything is within reach");
		return;
	}
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);
	C5CPostprocessor oPostprocessor(oChain,CPostprocessorSettings());
	//three locations of a vertical tool, a kilometre away: beyond the limits of the linear axes
	std::vector<CR3AnchoredVector> vInput;
	for(int i=0;i<3;i++)
		vInput.push_back(CR3AnchoredVector(CR3Vector(0.0,0.0,1.0),CR3Point(1000.0+i,0.0,0.0)));
	std::string sLog;
	try{
		std::vector<tChainConfigurationVector> vResult = oPostprocessor.SolveSingularOrientation(vInput,oChain,oTool,sLog);
		for(size_t i=0;i<vResult.size();i++)
			EXPECT_TRUE(vResult[i].empty())<<"set "<<i;
		EXPECT_NE(std::string::npos,sLog.find("can be solved"))<<sLog;
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
}

//KnownBugs #38: the milling tool was tested against the boxes of the machine only when the TOOL segment itself
//had a box. A chain of two straight branches: the tool frame and the holder frame are both the base frame, so a box
//around the origin holds the tip of the tool (a ball of radius 0.016, tip at the origin, axis along OZ).
static std::string ChainWithBoxlessTool(const std::string &sLinkBoxes, const std::string &sHolderBoxes)
{
	std::string s = R"KC({
  "format": "kin5ax-chain",
  "version": 1,
  "unit": "m",
  "name": "boxless tool",
  "tool_branch": {
    "links": [
      { LINK_BOXES "type": "linear", "name": "Z",
        "translation": [0, 0, 0],
        "rotation": { "axis": [0, 0, 1], "angle_deg": 0 } }
    ],
    "tool": { "name": "tool", "translation": [0, 0, 0], "rotation": { "axis": [0, 0, 1], "angle_deg": 0 } }
  },
  "workpiece_branch": {
    "links": [
      { "type": "rotary", "name": "C",
        "translation": [0, 0, 0],
        "rotation": { "axis": [0, 0, 1], "angle_deg": 0 } }
    ],
    "workpiece_holder": { HOLDER_BOXES "name": "holder", "translation": [0, 0, 0],
        "rotation": { "axis": [0, 0, 1], "angle_deg": 0 } }
  }
}
)KC";
	s.replace(s.find("LINK_BOXES"),10,sLinkBoxes);
	s.replace(s.find("HOLDER_BOXES"),12,sHolderBoxes);
	return s;
}

static const char *szBOX_AT_THE_TOOL = "\"boxes\": [ { \"min\": [-0.05, -0.05, -0.05], \"max\": [0.05, 0.05, 0.05] } ],";
static const char *szBOX_FAR_ABOVE = "\"boxes\": [ { \"min\": [1, 1, 1], \"max\": [1.1, 1.1, 1.1] } ],";
static const char *szBOX_FAR_BELOW = "\"boxes\": [ { \"min\": [-1.1, -1.1, -1.1], \"max\": [-1, -1, -1] } ],";

//the chain of the document and the collision report of the default tool at the home position
static bool Collides(const std::string &sJson, CChain &oChain, std::string &sLog)
{
	try{
		oChain = CChainJsonReader::Parse(sJson);
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<"exception: "<<e.Message();
		return false;
	}
	EXPECT_TRUE(oChain.m_vToolPart.back().GetPartVector().empty())<<"the TOOL segment must have no box";
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);
	return oChain.HasCollision(oTool,sLog);
}

TEST(Collisions, BoxlessToolClearOfTheMachine)
{
	CChain oChain;
	std::string sLog;
	EXPECT_FALSE(Collides(ChainWithBoxlessTool(szBOX_FAR_ABOVE,szBOX_FAR_BELOW),oChain,sLog))<<sLog;
}

TEST(Collisions, BoxlessToolInABoxOfTheWorkpieceHolder)
{
	CChain oChain;
	std::string sLog;
	EXPECT_TRUE(Collides(ChainWithBoxlessTool(szBOX_FAR_ABOVE,szBOX_AT_THE_TOOL),oChain,sLog));
	EXPECT_TRUE(oChain.m_vMaterialHolderPart.back().IsPartInCollision(0));
	EXPECT_FALSE(oChain.m_vToolPart[0].IsPartInCollision(0));
	EXPECT_NE(std::string::npos,sLog.find("box_1"))<<sLog;
}

TEST(Collisions, BoxlessToolInABoxOfItsOwnBranch)
{
	CChain oChain;
	std::string sLog;
	EXPECT_TRUE(Collides(ChainWithBoxlessTool(szBOX_AT_THE_TOOL,szBOX_FAR_BELOW),oChain,sLog));
	EXPECT_TRUE(oChain.m_vToolPart[0].IsPartInCollision(0));
	EXPECT_FALSE(oChain.m_vMaterialHolderPart.back().IsPartInCollision(0));
	EXPECT_NE(std::string::npos,sLog.find("box_1"))<<sLog;
}

INSTANTIATE_TEST_CASE_P(AllChains, ChainFileTest, ::testing::ValuesIn(GetChainFiles()), ChainFileTestName());

//---- the unit of a chain: three spellings, and nothing read into them

TEST(ChainUnit, TheThreeSpellingsAndWhatTheyAreInMillimetres)
{
	EXPECT_STREQ("mm",ChainUnitName(CHAIN_UNIT_MM));
	EXPECT_STREQ("m",ChainUnitName(CHAIN_UNIT_M));
	EXPECT_STREQ("in",ChainUnitName(CHAIN_UNIT_IN));
	EXPECT_DOUBLE_EQ(1.0,ChainUnitInMillimetres(CHAIN_UNIT_MM));
	EXPECT_DOUBLE_EQ(1000.0,ChainUnitInMillimetres(CHAIN_UNIT_M));
	EXPECT_DOUBLE_EQ(25.4,ChainUnitInMillimetres(CHAIN_UNIT_IN));
	for(int i=0;i<CHAIN_UNIT_COUNT;i++){
		CHAIN_UNIT unit = CHAIN_UNIT_COUNT;
		EXPECT_TRUE(ParseChainUnit(ChainUnitName((CHAIN_UNIT)i),unit))<<"spelling "<<i;
		EXPECT_EQ((CHAIN_UNIT)i,unit);
	}
}

TEST(ChainUnit, OnlyTheExactSpellingIsAUnit)
{
	//a file that says "MM" or "meter" was written against the wrong description, and a reader that
	//guessed what it meant would be right until the day it was not
	CHAIN_UNIT unit = CHAIN_UNIT_COUNT;
	EXPECT_FALSE(ParseChainUnit("MM",unit));
	EXPECT_FALSE(ParseChainUnit("meter",unit));
	EXPECT_FALSE(ParseChainUnit("inch",unit));
	EXPECT_FALSE(ParseChainUnit("",unit));
	EXPECT_FALSE(ParseChainUnit(" m",unit));
	EXPECT_EQ(CHAIN_UNIT_COUNT,unit)<<"nothing written into the answer when there is none";
}

TEST(ChainUnit, AChainMadeInMemoryIsMetresLikeEveryModelShipped)
{
	CChain oChain;
	EXPECT_EQ(CHAIN_UNIT_M,oChain.GetUnit());
	EXPECT_DOUBLE_EQ(1000.0,oChain.GetUnitInMillimetres());
	oChain.SetUnit(CHAIN_UNIT_IN);
	EXPECT_EQ(CHAIN_UNIT_IN,oChain.GetUnit());
	EXPECT_DOUBLE_EQ(25.4,oChain.GetUnitInMillimetres());
}

TEST(ChainUnit, EveryShippedModelSaysMetres)
{
	//the catalogue models were made in metres and the files now say so; one that did not would be
	//refused, and one that said something else would be a model with its lengths off by a thousand
	std::vector<std::string> v = GetChainFiles();
	ASSERT_FALSE(v.empty());
	for(size_t i=0;i<v.size();i++){
		CChain oChain = LoadChain(v[i]);
		EXPECT_EQ(CHAIN_UNIT_M,oChain.GetUnit())<<v[i];
	}
}
