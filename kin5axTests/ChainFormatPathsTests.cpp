// Every path through the reader and the writer of the chain file, on every test chain.
//   JSON (.kc.json): reader CChainJsonReader, writer CChainJsonWriter
// A chain that went through any sequence of them must still be the same chain: the same text and
// the same descriptive data as the chain read directly from the file.
//
// The XML form (.kc) was retired and lives only in tools/kc2json. CChainFileReader refuses one by
// name, which is the last thing in this file.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "ChainJson.h"
#include "ChainFileReader.h"

using namespace kin5ax;

namespace {

//what a chain is compared by: the written text, and the descriptive data that is not in it
struct CChainImage
{
	std::string sJson;
	std::string sName, sMachine, sAuthor, sModified;
	size_t nToolSegments = 0, nMaterialSegments = 0;
};

CChainImage Image(const CChain &oChain)
{
	CChainImage o;
	o.sJson = CChainJsonWriter::ToJson(oChain);
	o.sName = oChain.GetName();
	o.sMachine = oChain.GetMachine();
	o.sAuthor = oChain.GetAuthor();
	o.sModified = oChain.GetModified();
	o.nToolSegments = oChain.GetPart(CHAIN_PART_TOOL).size();
	o.nMaterialSegments = oChain.GetPart(CHAIN_PART_MATERIAL_HOLDER).size();
	return o;
}

void ExpectSameChain(const CChainImage &a, const CChainImage &b)
{
	EXPECT_EQ(a.sJson,b.sJson);
	EXPECT_EQ(a.sName,b.sName);
	EXPECT_EQ(a.sMachine,b.sMachine);
	EXPECT_EQ(a.sAuthor,b.sAuthor);
	EXPECT_EQ(a.sModified,b.sModified);
	EXPECT_EQ(a.nToolSegments,b.nToolSegments);
	EXPECT_EQ(a.nMaterialSegments,b.nMaterialSegments);
}

std::string ReadWholeFile(const std::string &sFilePath)
{
	std::string sText;
	FILE *pFile = fopen(sFilePath.c_str(),"rb");
	if(!pFile)
		return sText;
	char szBuffer[4096];
	size_t n = 0;
	while((n=fread(szBuffer,1,sizeof(szBuffer),pFile))>0)
		sText.append(szBuffer,n);
	fclose(pFile);
	return sText;
}

void WriteText(const std::string &sFilePath, const std::string &sText)
{
	FILE *pFile = fopen(sFilePath.c_str(),"wb");
	ASSERT_TRUE(pFile!=NULL);
	fwrite(sText.data(),1,sText.size(),pFile);
	fclose(pFile);
}

} // namespace

class ChainFile : public ::testing::TestWithParam<std::string> {};

struct FormatPathName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST_P(ChainFile, ReadsBackThroughTheWriter)
{
	CChain oChain;
	try{
		oChain = CChainJsonReader::ReadFile(GetParam());
	} catch (CGeneratorException &e){
		FAIL()<<e.Message();
	}
	const std::string sFilePath = TempFilePath("kin5axTests_paths.kc.json");
	WriteText(sFilePath,CChainJsonWriter::ToJson(oChain));
	try{
		ExpectSameChain(Image(oChain),Image(CChainJsonReader::ReadFile(sFilePath)));
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<e.Message();
	}
	remove(sFilePath.c_str());
}

TEST_P(ChainFile, TheFileReaderOpensIt)
{
	try{
		ExpectSameChain(Image(CChainJsonReader::ReadFile(GetParam()))
			,Image(CChainFileReader::ReadFile(GetParam())));
	} catch (CGeneratorException &e){
		FAIL()<<e.Message();
	}
	EXPECT_FALSE(CChainFileReader::IsRetiredXml(GetParam()));
}

INSTANTIATE_TEST_CASE_P(Files, ChainFile, ::testing::ValuesIn(GetChainFiles()), FormatPathName());

//---- what the file reader does with a file in the form that was retired

TEST(ChainFileReader, TheExtensionDecidesInAnyCase)
{
	EXPECT_TRUE(CChainFileReader::IsRetiredXml("machine.kc"));
	EXPECT_TRUE(CChainFileReader::IsRetiredXml("C:/chains/MACHINE.KC"));
	EXPECT_FALSE(CChainFileReader::IsRetiredXml("machine.kc.json"));
	EXPECT_FALSE(CChainFileReader::IsRetiredXml("C:/chains/MACHINE.KC.JSON"));
}

TEST(ChainFileReader, TheContentDecidesWhenTheExtensionDoesNot)
{
	const std::string sXmlPath = TempFilePath("kin5axTests_sniff.chain");
	const std::string sJsonPath = TempFilePath("kin5axTests_sniff.machine");
	WriteText(sXmlPath,"  \r\n\t<kinematic_chain/>");
	WriteText(sJsonPath,"  \r\n\t"+ReadWholeFile(GetChainFiles()[0]));	//blanks before the brace
	EXPECT_TRUE(CChainFileReader::IsRetiredXml(sXmlPath));
	EXPECT_FALSE(CChainFileReader::IsRetiredXml(sJsonPath));
	try{
		ExpectSameChain(Image(CChainJsonReader::ReadFile(GetChainFiles()[0]))
			,Image(CChainFileReader::ReadFile(sJsonPath)));
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<e.Message();
	}
	remove(sXmlPath.c_str());
	remove(sJsonPath.c_str());
}

//The library stopped reading the XML form; a file still in it is told so by name, and told what to
//do about it, rather than failing as a broken JSON document would.
TEST(ChainFileReader, ARetiredXmlFileIsRefusedByName)
{
	const std::string sPath = TempFilePath("kin5axTests_retired.kc");
	WriteText(sPath,"<?xml version=\"1.0\"?>\n<kinematic_chain unit=\"m\"/>\n");
	std::string sError;
	try{
		CChainFileReader::ReadFile(sPath);
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	remove(sPath.c_str());
	EXPECT_NE(std::string::npos,sError.find("kin5axTests_retired.kc"))<<sError;
	EXPECT_NE(std::string::npos,sError.find("kc2json"))<<"it has to say what to do: "<<sError;
}

TEST(ChainFileReader, AMissingFileIsReportedWithItsName)
{
	std::string sError;
	try{
		CChainFileReader::ReadFile(GetTestChainsDir()+"no_such_chain.kc.json");
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	EXPECT_NE(std::string::npos,sError.find("no_such_chain.kc.json"))<<sError;
}
