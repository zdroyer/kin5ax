// Every path through the readers and the writers of the chain file formats, on every test chain.
//   XML (.kc):       reader CChainXmlReader (pugixml), writer CChain::toXML
//   JSON (.kc.json): reader CChainJsonReader, writer CChainJsonWriter
// A chain that went through any sequence of them must still be the same chain: the same XML text,
// the same JSON text and the same descriptive data as the chain read directly from the file.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "ChainXmlReader.h"
#include "ChainJson.h"

using namespace kin5ax;

namespace {

//a reader of a chain file, by name
struct CReader
{
	const char *szName;
	CChain (*pfnReadFile)(const std::string &sFilePath);
};

CChain ReadXml(const std::string &sFilePath)	{ return CChainXmlReader::ReadFile(sFilePath); }
CChain ReadJson(const std::string &sFilePath)	{ return CChainJsonReader::ReadFile(sFilePath); }

//one XML reader today (the MSXML one was removed after these tests had shown both equal on every path)
const CReader vXML_READERS[] = { {"xml",&ReadXml} };
const CReader oJSON_READER = {"json",&ReadJson};

//what every writer makes of the chain, plus the descriptive data: equal images = the same chain
struct CChainImage
{
	std::string sXml, sJson, sName, sMachine, sAuthor, sModified;
};

CChainImage Image(const CChain &oChain)
{
	CChainImage o;
	o.sXml = oChain.toXML();
	o.sJson = CChainJsonWriter::ToJson(oChain);
	o.sName = oChain.GetName();
	o.sMachine = oChain.GetMachine();
	o.sAuthor = oChain.GetAuthor();
	o.sModified = oChain.GetModified();
	return o;
}

void ExpectSameChain(const CChainImage &oExpected, const CChainImage &oActual)
{
	EXPECT_FALSE(oActual.sXml.empty());
	EXPECT_EQ(oExpected.sXml,oActual.sXml);
	EXPECT_EQ(oExpected.sJson,oActual.sJson);
	EXPECT_EQ(oExpected.sName,oActual.sName);
	EXPECT_EQ(oExpected.sMachine,oActual.sMachine);
	EXPECT_EQ(oExpected.sAuthor,oActual.sAuthor);
	EXPECT_EQ(oExpected.sModified,oActual.sModified);
}

void WriteText(const std::string &sFilePath, const std::string &sText)
{
	FILE *pFile = fopen(sFilePath.c_str(),"wb");
	ASSERT_TRUE(pFile!=NULL)<<sFilePath;
	fwrite(sText.data(),1,sText.size(),pFile);
	fclose(pFile);
}

//the chain written as XML to a temporary file and read back by oReader
CChain ThroughXml(const CChain &oChain, const CReader &oReader)
{
	std::string sFilePath = TempFilePath("kin5axTests_paths.kc");
	WriteText(sFilePath,oChain.toXML());
	CChain oResult = oReader.pfnReadFile(sFilePath);
	remove(sFilePath.c_str());
	return oResult;
}

//the chain written as JSON to a temporary file and read back
CChain ThroughJson(const CChain &oChain)
{
	std::string sFilePath = TempFilePath("kin5axTests_paths.kc.json");
	CChainJsonWriter::WriteFile(oChain,sFilePath);
	CChain oResult = ReadJson(sFilePath);
	remove(sFilePath.c_str());
	return oResult;
}

//the exceptions of kin5ax do not derive from std::exception: name the path and the message
#define EXPECT_NO_CHAIN_EXCEPTION(statement) \
	try{ statement; } catch (CGeneratorException &e){ ADD_FAILURE()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")"; }

} // namespace

struct FormatPathName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

//---- every .kc file
class XmlChainFile : public ::testing::TestWithParam<std::string> {};

TEST_P(XmlChainFile, EveryXmlReaderGivesTheSameChain)
{
	CChainImage oFirst;
	EXPECT_NO_CHAIN_EXCEPTION(oFirst = Image(vXML_READERS[0].pfnReadFile(GetParam())));
	for(const CReader &oReader : vXML_READERS){
		SCOPED_TRACE(oReader.szName);
		EXPECT_NO_CHAIN_EXCEPTION(ExpectSameChain(oFirst,Image(oReader.pfnReadFile(GetParam()))));
	}
}

TEST_P(XmlChainFile, XmlWrittenByAnyReaderIsReadByEveryReader)
{
	for(const CReader &oSource : vXML_READERS){
		for(const CReader &oTarget : vXML_READERS){
			SCOPED_TRACE(std::string(oSource.szName)+" -> xml -> "+oTarget.szName);
			EXPECT_NO_CHAIN_EXCEPTION({
				CChain oChain = oSource.pfnReadFile(GetParam());
				ExpectSameChain(Image(oChain),Image(ThroughXml(oChain,oTarget)));
			});
		}
	}
}

TEST_P(XmlChainFile, JsonWrittenFromAnyReaderReadsBack)
{
	for(const CReader &oSource : vXML_READERS){
		SCOPED_TRACE(std::string(oSource.szName)+" -> json -> json");
		EXPECT_NO_CHAIN_EXCEPTION({
			CChain oChain = oSource.pfnReadFile(GetParam());
			ExpectSameChain(Image(oChain),Image(ThroughJson(oChain)));
			//and the text form, without a file
			ExpectSameChain(Image(oChain),Image(CChainJsonReader::Parse(CChainJsonWriter::ToJson(oChain),"text")));
		});
	}
}

TEST_P(XmlChainFile, JsonBackToXmlIsReadByEveryReader)
{
	for(const CReader &oSource : vXML_READERS){
		for(const CReader &oTarget : vXML_READERS){
			SCOPED_TRACE(std::string(oSource.szName)+" -> json -> xml -> "+oTarget.szName);
			EXPECT_NO_CHAIN_EXCEPTION({
				CChain oChain = oSource.pfnReadFile(GetParam());
				ExpectSameChain(Image(oChain),Image(ThroughXml(ThroughJson(oChain),oTarget)));
			});
		}
	}
}

INSTANTIATE_TEST_CASE_P(Files, XmlChainFile, ::testing::ValuesIn(GetChainFiles()), FormatPathName());

//---- every .kc.json file
class JsonChainFile : public ::testing::TestWithParam<std::string> {};

TEST_P(JsonChainFile, JsonReadsBackThroughJson)
{
	EXPECT_NO_CHAIN_EXCEPTION({
		CChain oChain = ReadJson(GetParam());
		ExpectSameChain(Image(oChain),Image(ThroughJson(oChain)));
	});
}

TEST_P(JsonChainFile, XmlWrittenFromJsonIsReadByEveryReader)
{
	for(const CReader &oTarget : vXML_READERS){
		SCOPED_TRACE(std::string("json -> xml -> ")+oTarget.szName);
		EXPECT_NO_CHAIN_EXCEPTION({
			CChain oChain = ReadJson(GetParam());
			ExpectSameChain(Image(oChain),Image(ThroughXml(oChain,oTarget)));
		});
	}
}

TEST_P(JsonChainFile, XmlThenJsonAgainIsTheSameChain)
{
	for(const CReader &oTarget : vXML_READERS){
		SCOPED_TRACE(std::string("json -> xml -> ")+oTarget.szName+" -> json -> json");
		EXPECT_NO_CHAIN_EXCEPTION({
			CChain oChain = ReadJson(GetParam());
			ExpectSameChain(Image(oChain),Image(ThroughJson(ThroughXml(oChain,oTarget))));
		});
	}
}

//the .kc twin of every .kc.json (X.kc next to X.kc.json) is the same machine for every XML reader
TEST_P(JsonChainFile, TheKcTwinIsTheSameMachineForEveryReader)
{
	std::string sKcPath = GetParam().substr(0,GetParam().size()-5);	//without ".json"
	FILE *pFile = fopen(sKcPath.c_str(),"rb");
	if(!pFile){
		RecordProperty("skipped","no .kc twin");
		return;
	}
	fclose(pFile);
	CChainImage oJson;
	EXPECT_NO_CHAIN_EXCEPTION(oJson = Image(ReadJson(GetParam())));
	for(const CReader &oReader : vXML_READERS){
		SCOPED_TRACE(oReader.szName);
		EXPECT_NO_CHAIN_EXCEPTION(ExpectSameChain(oJson,Image(oReader.pfnReadFile(sKcPath))));
	}
}

INSTANTIATE_TEST_CASE_P(Files, JsonChainFile, ::testing::ValuesIn(GetJsonChainFiles()), FormatPathName());
