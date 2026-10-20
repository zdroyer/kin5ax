// The JSON form of a kinematic chain (kin5ax/kc.schema.json): CChainJsonReader and CChainJsonWriter.
// The checks of the reader on deliberately broken documents, and the equivalence with the XML form
// for every test chain.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "ChainJson.h"
#include "double_tools.h"

using namespace kin5ax;

//a valid document; two joints and the tool in one branch, one joint and the holder in the other
static std::string Template()
{
	return R"KC({
  "format": "kin5ax-chain",
  "version": 2,
  "unit": "m",
  "name": "template",
  "tool_branch": {
    "links": [
      { "type": "linear", "name": "Z",
        "translation": [0, 0, 1],
        "rotation": { "axis": [0, 0, 1], "angle_deg": 0 },
        "limits": { "min": -0.5, "max": 0.5 },
        "boxes": [ { "min": [-0.1, -0.1, 0], "max": [0.1, 0.1, 0.2] } ] },
      { "type": "rotary", "name": "B",
        "translation": [0, 0, 0],
        "rotation": { "axis": [1, 0, 0], "angle_deg": 90 },
        "limits": { "min": -90, "max": 90 } }
    ],
    "tool_spindle_nose": {
      "name": "tool",
      "translation": [0, 0, 0],
      "rotation": { "axis": [1, 0, 0], "angle_deg": 0 }
    }
  },
  "workpiece_branch": {
    "links": [
      { "type": "rotary", "name": "C",
        "translation": [0, 0, 0],
        "rotation": { "axis": [0, 0, 1], "angle_deg": 0 } }
    ],
    "table": {
      "name": "holder",
      "translation": [0, 0, 0.1],
      "rotation": { "axis": [1, 0, 0], "angle_deg": 0 }
    }
  }
}
)KC";
}

static std::string Replaced(const std::string &sText, const std::string &sOld, const std::string &sNew)
{
	size_t n = sText.find(sOld);
	EXPECT_NE(std::string::npos,n)<<"the template does not contain: "<<sOld;
	return n==std::string::npos ? sText : sText.substr(0,n)+sNew+sText.substr(n+sOld.size());
}

//the message of the reader for this document, empty when it loads
static std::string JsonError(const std::string &sContent, CChain *pChain = NULL)
{
	try{
		CChain oChain = CChainJsonReader::Parse(sContent);
		if(pChain)
			*pChain = oChain;
	} catch (CGeneratorException &e){
		return e.Message();
	}
	return std::string();
}

#define EXPECT_ERROR_CONTAINS(sError, sText) EXPECT_NE(std::string::npos,(sError).find(sText))<<"message: "<<(sError)

TEST(ChainJsonReader, TemplateLoads)
{
	CChain oChain;
	std::string sError = JsonError(Template(),&oChain);
	ASSERT_EQ(std::string(),sError);
	EXPECT_EQ("template",oChain.GetName());
	ASSERT_EQ((size_t)3,oChain.m_vToolPart.size())<<"two joints and the tool";
	ASSERT_EQ((size_t)2,oChain.m_vMaterialHolderPart.size())<<"one joint and the workpiece holder";

	const CChainSegment &z = oChain.m_vToolPart[0];
	EXPECT_EQ(CHAIN_SEGMENT_PRISMATIC_LINK,z.GetType())<<"linear is the prismatic joint of the model";
	EXPECT_EQ("Z",z.GetName());
	EXPECT_DOUBLE_EQ(1.0,z.m_r3vConstTranslation.GetZ());
	EXPECT_TRUE(z.IsParameterConstrained());
	EXPECT_DOUBLE_EQ(-0.5,z.m_dParameterValueMinimum)<<"a linear limit is a length";
	EXPECT_DOUBLE_EQ(0.5,z.m_dParameterValueMaximum);
	ASSERT_EQ((size_t)1,z.GetPartVector().size());
	EXPECT_DOUBLE_EQ(-0.1,z.GetPartVector()[0].GetMinPoint().GetX());
	EXPECT_DOUBLE_EQ(0.2,z.GetPartVector()[0].GetMaxPoint().GetZ());

	const CChainSegment &b = oChain.m_vToolPart[1];
	EXPECT_EQ(CHAIN_SEGMENT_REVOLUTE_LINK,b.GetType())<<"rotary is the revolute joint of the model";
	EXPECT_NEAR(-dPiDiv2,b.m_dParameterValueMinimum,1E-12)<<"a rotary limit is degrees in the file";
	EXPECT_NEAR(dPiDiv2,b.m_dParameterValueMaximum,1E-12);
	EXPECT_DOUBLE_EQ(90.0,b.m_dConstRevoluteAngle)<<"angle_deg is stored as written";

	EXPECT_EQ(CHAIN_SEGMENT_TOOL_SPINDLE_NOSE,oChain.m_vToolPart[2].GetType());
	//CChainSegment::GetName ignores the stored name at the end of a branch and labels it itself
	EXPECT_EQ("tool",oChain.m_vToolPart[2].GetName())<<"the name of the file, not a label";
	EXPECT_FALSE(oChain.m_vToolPart[2].IsParameterConstrained())<<"the end of a branch is not a joint";
	EXPECT_EQ(CHAIN_SEGMENT_TABLE,oChain.m_vMaterialHolderPart[1].GetType());
	EXPECT_EQ("holder",oChain.m_vMaterialHolderPart[1].GetName())<<"the name of the file";
}

TEST(ChainJsonReader, ABranchMayHaveNoJoint)
{
	std::string s = Replaced(Template()
		,"\"links\": [\n      { \"type\": \"rotary\", \"name\": \"C\",\n        \"translation\": [0, 0, 0],\n"
		 "        \"rotation\": { \"axis\": [0, 0, 1], \"angle_deg\": 0 } }\n    ]"
		,"\"links\": []");
	CChain oChain;
	ASSERT_EQ(std::string(),JsonError(s,&oChain));
	ASSERT_EQ((size_t)1,oChain.m_vMaterialHolderPart.size());
	EXPECT_EQ(CHAIN_SEGMENT_TABLE,oChain.m_vMaterialHolderPart[0].GetType());
}

TEST(ChainJsonReader, MetadataIsRead)
{
	std::string s = Replaced(Template(),"\"name\": \"template\","
		,"\"name\": \"template\",\n  \"machine\": \"SHW UniSpeed 3500\",\n  \"author\": \"P. Zdroik\",\n  \"modified\": \"2026-09-22\",");
	CChain oChain;
	ASSERT_EQ(std::string(),JsonError(s,&oChain));
	EXPECT_EQ("SHW UniSpeed 3500",oChain.GetMachine());
	EXPECT_EQ("P. Zdroik",oChain.GetAuthor());
	EXPECT_EQ("2026-09-22",oChain.GetModified());
}

TEST(ChainJsonReader, ASchemaReferenceIsAccepted)
{
	EXPECT_EQ(std::string(),JsonError(Replaced(Template(),"{\n  \"format\"","{\n  \"$schema\": \"kc.schema.json\",\n  \"format\"")));
}

TEST(ChainJsonReader, SyntaxErrorIsReportedWithTheLine)
{
	std::string sError = JsonError(Replaced(Template(),"\"version\": 2,","\"version\": 2"));
	EXPECT_ERROR_CONTAINS(sError,"JSON error");
	EXPECT_ERROR_CONTAINS(sError,"line ");
}

TEST(ChainJsonReader, AForeignDocumentIsReported)
{
	std::string sError = JsonError("{ \"format\": \"something-else\", \"version\": 2, \"tool_branch\": {}, \"workpiece_branch\": {} }");
	EXPECT_ERROR_CONTAINS(sError,"format");
	EXPECT_ERROR_CONTAINS(sError,"kin5ax-chain");
}

//Version 1 named the terminals tool and workpiece_holder, and the origin of the first of them was
//the tool point in a file that named no holder. Nothing of that can be guessed at, so a file of
//that version is refused and told what converts it.
TEST(ChainJsonReader, TheVersionBeforeThisOneIsRefusedWithAWayOut)
{
	std::string sError = JsonError(Replaced(Template(),"\"version\": 2","\"version\": 1"));
	EXPECT_ERROR_CONTAINS(sError,"version 1");
	EXPECT_ERROR_CONTAINS(sError,"kc2json");
	EXPECT_ERROR_CONTAINS(sError,"tool_spindle_nose");
}

TEST(ChainJsonReader, AnotherVersionIsReported)
{
	std::string sError = JsonError(Replaced(Template(),"\"version\": 2","\"version\": 3"));
	EXPECT_ERROR_CONTAINS(sError,"version");
}

TEST(ChainJsonReader, MissingMemberIsReported)
{
	std::string sError = JsonError(Replaced(Template(),"\"name\": \"tool\",\n      ",""));
	EXPECT_ERROR_CONTAINS(sError,"tool_branch.tool_spindle_nose");
	EXPECT_ERROR_CONTAINS(sError,"'name'");
}

TEST(ChainJsonReader, UnknownMemberIsReported)
{
	std::string sError = JsonError(Replaced(Template(),"\"type\": \"rotary\", \"name\": \"B\"","\"type\": \"rotary\", \"colour\": \"red\", \"name\": \"B\""));
	EXPECT_ERROR_CONTAINS(sError,"tool_branch.links[1]");
	EXPECT_ERROR_CONTAINS(sError,"'colour'");
}

TEST(ChainJsonReader, TheEndOfABranchTakesNoLimits)
{
	std::string sError = JsonError(Replaced(Template()
		,"\"name\": \"tool\",","\"name\": \"tool\", \"limits\": { \"min\": 0, \"max\": 1 },"));
	EXPECT_ERROR_CONTAINS(sError,"tool_branch.tool_spindle_nose");
	EXPECT_ERROR_CONTAINS(sError,"'limits'");
}

TEST(ChainJsonReader, UnknownLinkTypeIsReported)
{
	std::string sError = JsonError(Replaced(Template(),"\"type\": \"rotary\", \"name\": \"B\"","\"type\": \"revolute\", \"name\": \"B\""));
	EXPECT_ERROR_CONTAINS(sError,"tool_branch.links[1].type");
	EXPECT_ERROR_CONTAINS(sError,"revolute");
}

TEST(ChainJsonReader, OneSidedLimitIsReported)
{
	std::string sError = JsonError(Replaced(Template(),"\"limits\": { \"min\": -90, \"max\": 90 }","\"limits\": { \"min\": -90 }"));
	EXPECT_ERROR_CONTAINS(sError,"limits");
	EXPECT_ERROR_CONTAINS(sError,"'max'");
}

TEST(ChainJsonReader, MinimumAboveMaximumIsReported)
{
	std::string sError = JsonError(Replaced(Template(),"\"limits\": { \"min\": -90, \"max\": 90 }","\"limits\": { \"min\": 90, \"max\": -90 }"));
	EXPECT_ERROR_CONTAINS(sError,"min is above max");
}

TEST(ChainJsonReader, AWrongTypeIsReported)
{
	std::string sError = JsonError(Replaced(Template(),"\"angle_deg\": 90","\"angle_deg\": \"90\""));
	EXPECT_ERROR_CONTAINS(sError,"angle_deg");
	EXPECT_ERROR_CONTAINS(sError,"number");
}

TEST(ChainJsonReader, AShortVectorIsReported)
{
	std::string sError = JsonError(Replaced(Template(),"\"translation\": [0, 0, 1]","\"translation\": [0, 1]"));
	EXPECT_ERROR_CONTAINS(sError,"tool_branch.links[0].translation");
	EXPECT_ERROR_CONTAINS(sError,"three numbers");
}

TEST(ChainJsonReader, ZeroRotationAxisWithAnAngleIsReported)
{
	std::string sError = JsonError(Replaced(Template(),"{ \"axis\": [1, 0, 0], \"angle_deg\": 90 }","{ \"axis\": [0, 0, 0], \"angle_deg\": 90 }"));
	EXPECT_ERROR_CONTAINS(sError,"zero vector");
}

TEST(ChainJsonReader, ZeroRotationAxisWithoutAnAngleIsAccepted)
{
	EXPECT_EQ(std::string(),JsonError(Replaced(Template(),"{ \"axis\": [0, 0, 1], \"angle_deg\": 0 }","{ \"axis\": [0, 0, 0], \"angle_deg\": 0 }")));
}

TEST(ChainJsonWriter, VectorsStayOnOneLine)
{
	CChain oChain;
	ASSERT_EQ(std::string(),JsonError(Template(),&oChain));
	std::string sJson = CChainJsonWriter::ToJson(oChain);
	EXPECT_ERROR_CONTAINS(sJson,"\"translation\": [0.0, 0.0, 1.0]");
	EXPECT_EQ('\n',sJson[sJson.size()-1]);
}

TEST(ChainJsonWriter, WritesAndReadsAFile)
{
	CChain oChain;
	ASSERT_EQ(std::string(),JsonError(Template(),&oChain));
	std::string sFilePath = TempFilePath("kin5axTests_chain.kc.json");
	CChain oReloaded;
	try{
		CChainJsonWriter::WriteFile(oChain,sFilePath);
		oReloaded = CChainJsonReader::ReadFile(sFilePath);
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	remove(sFilePath.c_str());
	EXPECT_EQ(CChainJsonWriter::ToJson(oChain),CChainJsonWriter::ToJson(oReloaded));
	EXPECT_EQ(sFilePath,oReloaded.GetFileName());
}

//---- every test chain, written and read again
class ChainJsonFile : public ::testing::TestWithParam<std::string> {};

struct ChainJsonFileName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST_P(ChainJsonFile, WritingIsStable)
{
	std::string sFirst, sSecond;
	try{
		CChain oChain = CChainJsonReader::ReadFile(GetParam());
		sFirst = CChainJsonWriter::ToJson(oChain);
		sSecond = CChainJsonWriter::ToJson(CChainJsonReader::Parse(sFirst,"json"));
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	EXPECT_FALSE(sFirst.empty());
	EXPECT_EQ(sFirst,sSecond)<<"a document read and written again must not change";
}

INSTANTIATE_TEST_CASE_P(Files, ChainJsonFile, ::testing::ValuesIn(GetChainFiles()), ChainJsonFileName());

//---- the tool holder of the tool terminal
static std::string WithToolHolder()
{
	return Replaced(Template(),"\"name\": \"tool\",","\"name\": \"tool\",\n      \"tool_holder\": { \"length\": 0.05, \"radius\": 0.03 },");
}

TEST(ChainJsonToolHolder, IsReadAndWritten)
{
	CChain oChain;
	ASSERT_EQ(std::string(),JsonError(WithToolHolder(),&oChain));
	EXPECT_TRUE(oChain.IsToolMountDeclared());
	EXPECT_DOUBLE_EQ(0.05,oChain.GetDefaultToolHolder().GetLength());
	EXPECT_DOUBLE_EQ(0.03,oChain.GetDefaultToolHolder().GetRadius());

	std::string sJson = CChainJsonWriter::ToJson(oChain);
	EXPECT_NE(std::string::npos,sJson.find("tool_holder"));
	CChain oReloaded;
	ASSERT_EQ(std::string(),JsonError(sJson,&oReloaded));
	EXPECT_TRUE(oReloaded.GetDefaultToolHolder()==oChain.GetDefaultToolHolder());

	CChain oPlain;
	ASSERT_EQ(std::string(),JsonError(Template(),&oPlain));
	EXPECT_FALSE(oPlain.IsToolMountDeclared());
	EXPECT_EQ(std::string::npos,CChainJsonWriter::ToJson(oPlain).find("tool_holder"))<<"not written when absent";
}

TEST(ChainJsonToolHolder, TheWorkpieceHolderCarriesNone)
{
	std::string s = Replaced(Template(),"\"name\": \"holder\",","\"name\": \"holder\",\n      \"tool_holder\": { \"length\": 0.05, \"radius\": 0.03 },");
	std::string sError = JsonError(s);
	EXPECT_ERROR_CONTAINS(sError,"tool_holder");
	EXPECT_ERROR_CONTAINS(sError,"workpiece_branch.table");
}

TEST(ChainJsonToolHolder, AMissingMemberIsNamedByItsPath)
{
	std::string s = Replaced(Template(),"\"name\": \"tool\",","\"name\": \"tool\",\n      \"tool_holder\": { \"length\": 0.05 },");
	std::string sError = JsonError(s);
	EXPECT_ERROR_CONTAINS(sError,"radius");
	EXPECT_ERROR_CONTAINS(sError,"tool_branch.tool_spindle_nose.tool_holder");
}

//KnownBugs #64: a terminal answered with a fixed label and the writer wrote that back, so the name
//of the file was lost on the first save. Both terminals carry a free-form name now.
TEST(ChainJsonTerminals, TheNameOfATerminalSurvivesTheFile)
{
	const std::string sSpindle = "HSK63A spindle of the right head";
	const std::string sTable = "the big table";
	std::string sText = Replaced(Template(),"\"name\": \"tool\"","\"name\": \""+sSpindle+"\"");
	sText = Replaced(sText,"\"name\": \"holder\"","\"name\": \""+sTable+"\"");

	CChain oChain;
	ASSERT_EQ(std::string(),JsonError(sText,&oChain));
	EXPECT_EQ(sSpindle,oChain.m_vToolPart.back().GetName());
	EXPECT_EQ(sTable,oChain.m_vMaterialHolderPart.back().GetName());

	CChain oReloaded;
	ASSERT_EQ(std::string(),JsonError(CChainJsonWriter::ToJson(oChain),&oReloaded));
	EXPECT_EQ(sSpindle,oReloaded.m_vToolPart.back().GetName())<<"written and read again";
	EXPECT_EQ(sTable,oReloaded.m_vMaterialHolderPart.back().GetName());
}

//a terminal of a chain built in code, which no file ever named, says what it is
TEST(ChainJsonTerminals, ATerminalWithNoNameSaysWhatItIs)
{
	CChainSegment oSpindle(CHAIN_SEGMENT_TOOL_SPINDLE_NOSE,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0));
	CChainSegment oTable(CHAIN_SEGMENT_TABLE,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0));
	EXPECT_EQ("Spindle nose",oSpindle.GetName());
	EXPECT_EQ("Table",oTable.GetName());
}

//---- the unit: required, exact, and kept

TEST(ChainJsonUnit, IsRead)
{
	CChain oChain;
	ASSERT_EQ(std::string(),JsonError(Template(),&oChain));
	EXPECT_EQ(CHAIN_UNIT_M,oChain.GetUnit());
	CChain oMillimetres;
	ASSERT_EQ(std::string(),JsonError(Replaced(Template(),"\"unit\": \"m\"","\"unit\": \"mm\""),&oMillimetres));
	EXPECT_EQ(CHAIN_UNIT_MM,oMillimetres.GetUnit());
}

TEST(ChainJsonUnit, AFileWithoutOneIsRefused)
{
	std::string sError = JsonError(Replaced(Template(),"  \"unit\": \"m\",\n",""));
	EXPECT_ERROR_CONTAINS(sError,"'unit'");
}

TEST(ChainJsonUnit, AnUnknownOneIsRefusedWithTheThreeThatExist)
{
	std::string sError = JsonError(Replaced(Template(),"\"unit\": \"m\"","\"unit\": \"meter\""));
	EXPECT_ERROR_CONTAINS(sError,"unit");
	EXPECT_ERROR_CONTAINS(sError,"'meter'");
	EXPECT_ERROR_CONTAINS(sError,"mm, m and in");
	//a number is not a spelling either
	EXPECT_NE(std::string(),JsonError(Replaced(Template(),"\"unit\": \"m\"","\"unit\": 1000")));
}

TEST(ChainJsonUnit, SurvivesTheJsonRoundTrip)
{
	CChain oChain;
	ASSERT_EQ(std::string(),JsonError(Replaced(Template(),"\"unit\": \"m\"","\"unit\": \"in\""),&oChain));
	std::string sJson = CChainJsonWriter::ToJson(oChain);
	EXPECT_NE(std::string::npos,sJson.find("\"unit\": \"in\""))<<sJson;
	CChain oReloaded;
	ASSERT_EQ(std::string(),JsonError(sJson,&oReloaded));
	EXPECT_EQ(CHAIN_UNIT_IN,oReloaded.GetUnit());
}
