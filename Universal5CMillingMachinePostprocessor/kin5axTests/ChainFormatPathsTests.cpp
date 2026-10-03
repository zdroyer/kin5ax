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
#include "ChainFileReader.h"

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

//the application opens a chain through CChainFileReader, which must pick the XML reader here
TEST_P(XmlChainFile, TheFileReaderPicksXml)
{
	EXPECT_EQ(CChainFileReader::FORMAT_XML,CChainFileReader::FormatOf(GetParam()));
	EXPECT_NO_CHAIN_EXCEPTION(ExpectSameChain(Image(ReadXml(GetParam())),Image(CChainFileReader::ReadFile(GetParam()))));
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

TEST_P(JsonChainFile, TheFileReaderPicksJson)
{
	EXPECT_EQ(CChainFileReader::FORMAT_JSON,CChainFileReader::FormatOf(GetParam()));
	EXPECT_NO_CHAIN_EXCEPTION(ExpectSameChain(Image(ReadJson(GetParam())),Image(CChainFileReader::ReadFile(GetParam()))));
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

//---- CChainFileReader on files whose extension says nothing: the content decides
namespace {

std::string ReadWholeFile(const std::string &sFilePath)
{
	FILE *pFile = fopen(sFilePath.c_str(),"rb");
	EXPECT_TRUE(pFile!=NULL)<<sFilePath;
	std::string s;
	if(pFile){
		char szBuffer[4096];
		size_t n;
		while((n = fread(szBuffer,1,sizeof(szBuffer),pFile))>0)
			s.append(szBuffer,n);
		fclose(pFile);
	}
	return s;
}

} // namespace

TEST(ChainFileReader, TheExtensionDecidesInAnyCase)
{
	EXPECT_EQ(CChainFileReader::FORMAT_XML,CChainFileReader::FormatOf("machine.kc"));
	EXPECT_EQ(CChainFileReader::FORMAT_XML,CChainFileReader::FormatOf("C:/chains/MACHINE.KC"));
	EXPECT_EQ(CChainFileReader::FORMAT_JSON,CChainFileReader::FormatOf("machine.kc.json"));
	EXPECT_EQ(CChainFileReader::FORMAT_JSON,CChainFileReader::FormatOf("machine.JSON"));
	EXPECT_EQ(CChainFileReader::FORMAT_XML,CChainFileReader::FormatOf("no_such_file.chain"))<<"unreadable: the XML reader reports it";
}

TEST(ChainFileReader, TheContentDecidesWhenTheExtensionDoesNot)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	ASSERT_GE(GetJsonChainFiles().size(),(size_t)1);
	std::string sXmlPath = TempFilePath("kin5axTests_sniff_xml.chain"), sJsonPath = TempFilePath("kin5axTests_sniff_json.chain");
	WriteText(sXmlPath,ReadWholeFile(GetChainFiles()[0]));
	WriteText(sJsonPath,"  \r\n\t"+ReadWholeFile(GetJsonChainFiles()[0]));	//blanks before the brace
	EXPECT_EQ(CChainFileReader::FORMAT_XML,CChainFileReader::FormatOf(sXmlPath));
	EXPECT_EQ(CChainFileReader::FORMAT_JSON,CChainFileReader::FormatOf(sJsonPath));
	EXPECT_NO_CHAIN_EXCEPTION(ExpectSameChain(Image(ReadXml(GetChainFiles()[0])),Image(CChainFileReader::ReadFile(sXmlPath))));
	EXPECT_NO_CHAIN_EXCEPTION(ExpectSameChain(Image(ReadJson(GetJsonChainFiles()[0])),Image(CChainFileReader::ReadFile(sJsonPath))));
	remove(sXmlPath.c_str());
	remove(sJsonPath.c_str());
}

TEST(ChainFileReader, AMissingFileIsReportedWithItsName)
{
	std::string sError;
	try{
		CChainFileReader::ReadFile("no_such_chain.kc.json");
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	EXPECT_NE(std::string::npos,sError.find("no_such_chain.kc.json"))<<sError;
}
