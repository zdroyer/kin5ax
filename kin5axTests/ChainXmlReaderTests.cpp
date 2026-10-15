// The .kc reader CChainXmlReader (pugixml): its checks on deliberately broken files, and every test
// chain read, written back and read again.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "ChainXmlReader.h"
#include "double_tools.h"

using namespace kin5ax;

//a small valid chain without DOCTYPE; the line numbers matter for the tests below
static std::string Template()
{
	return
	"<?xml version=\"1.0\"?>\n"                                                                                          // 1
	"<kinematic_chain unit=\"m\" name=\"template\">\n"                                                                              // 2
	"<tool_part>\n"                                                                                                      // 3
	"<segment type=\"PRISMATIC_LINK\" name=\"Z\" parameter_value_offset=\"0\" default_parameter_value=\"0\" "
	"parameter_value_minimum=\"-0.5\" parameter_value_maximum=\"0.5\">\n"                                               // 4
	"<initial_translation x=\"0\" y=\"0\" z=\"1\"/>\n"                                                                   // 5
	"<initial_rotation x=\"0\" y=\"0\" z=\"1\" alpha=\"0\"/>\n"                                                          // 6
	"<box x1=\"-0.1\" x2=\"0.1\" y1=\"-0.1\" y2=\"0.1\" z1=\"0\" z2=\"0.2\"/>\n"                                          // 7
	"</segment>\n"                                                                                                       // 8
	"<segment type=\"REVOLUTE_LINK\" name=\"B\" parameter_value_minimum=\"-90\" parameter_value_maximum=\"90\">\n"      // 9
	"<initial_translation x=\"0\" y=\"0\" z=\"0\"/>\n"                                                                   // 10
	"<initial_rotation x=\"1\" y=\"0\" z=\"0\" alpha=\"90\"/>\n"                                                         // 11
	"</segment>\n"                                                                                                       // 12
	"<segment type=\"TOOL\" name=\"tool\">\n"                                                                            // 13
	"<initial_translation x=\"0\" y=\"0\" z=\"0\"/>\n"                                                                   // 14
	"<initial_rotation x=\"1\" y=\"0\" z=\"0\" alpha=\"0\"/>\n"                                                          // 15
	"</segment>\n"                                                                                                       // 16
	"</tool_part>\n"                                                                                                     // 17
	"<material_holder_part>\n"                                                                                           // 18
	"<segment type=\"REVOLUTE_LINK\" name=\"C\">\n"                                                                      // 19
	"<initial_translation x=\"0\" y=\"0\" z=\"0\"/>\n"                                                                   // 20
	"<initial_rotation x=\"0\" y=\"0\" z=\"1\" alpha=\"0\"/>\n"                                                          // 21
	"</segment>\n"                                                                                                       // 22
	"<segment type=\"MATERIAL_HOLDER\" name=\"holder\">\n"                                                               // 23
	"<initial_translation x=\"0\" y=\"0\" z=\"0.1\"/>\n"                                                                 // 24
	"<initial_rotation x=\"1\" y=\"0\" z=\"0\" alpha=\"0\"/>\n"                                                          // 25
	"</segment>\n"                                                                                                       // 26
	"</material_holder_part>\n"                                                                                          // 27
	"</kinematic_chain>\n";                                                                                              // 28
}

static std::string Replaced(const std::string &sText, const std::string &sOld, const std::string &sNew)
{
	size_t n = sText.find(sOld);
	EXPECT_NE(std::string::npos,n)<<"the template does not contain: "<<sOld;
	return n==std::string::npos ? sText : sText.substr(0,n)+sNew+sText.substr(n+sOld.size());
}

static std::string WriteTemp(const std::string &sName, const std::string &sContent)
{
	std::string sFilePath = TempFilePath(sName);
	FILE *pFile = fopen(sFilePath.c_str(),"wb");
	EXPECT_TRUE(pFile!=NULL)<<sFilePath;
	if(pFile){
		fwrite(sContent.data(),1,sContent.size(),pFile);
		fclose(pFile);
	}
	return sFilePath;
}

//the message of the pugixml reader for this content, empty when it loads
static std::string PugiError(const std::string &sContent, CChain *pChain = NULL)
{
	std::string sFilePath = WriteTemp("kin5axTests_pugixml.kc",sContent);
	std::string sError;
	try{
		CChain oChain = CChainXmlReader::ReadFile(sFilePath);
		if(pChain)
			*pChain = oChain;
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	remove(sFilePath.c_str());
	return sError;
}

#define EXPECT_ERROR_CONTAINS(sError, sText) EXPECT_NE(std::string::npos,(sError).find(sText))<<"message: "<<(sError)

TEST(PugixmlReader, TemplateLoads)
{
	CChain oChain;
	std::string sError = PugiError(Template(),&oChain);
	ASSERT_EQ(std::string(),sError);
	ASSERT_EQ((size_t)3,oChain.m_vToolPart.size());
	ASSERT_EQ((size_t)2,oChain.m_vMaterialHolderPart.size());
	const CChainSegment &z = oChain.m_vToolPart[0];
	EXPECT_EQ(CHAIN_SEGMENT_PRISMATIC_LINK,z.GetType());
	EXPECT_EQ("Z",z.GetName());
	EXPECT_TRUE(z.IsParameterConstrained());
	EXPECT_DOUBLE_EQ(-0.5,z.m_dParameterValueMinimum);
	EXPECT_DOUBLE_EQ(0.5,z.m_dParameterValueMaximum);
	EXPECT_EQ((size_t)1,z.GetPartVector().size());
	EXPECT_DOUBLE_EQ(1.0,z.m_r3vConstTranslation.GetZ());
	const CChainSegment &b = oChain.m_vToolPart[1];
	EXPECT_EQ(CHAIN_SEGMENT_REVOLUTE_LINK,b.GetType());
	EXPECT_NEAR(-dPiDiv2,b.m_dParameterValueMinimum,1E-12)<<"revolute limits are degrees in the file";
	EXPECT_NEAR(dPiDiv2,b.m_dParameterValueMaximum,1E-12);
	EXPECT_DOUBLE_EQ(90.0,b.m_dConstRevoluteAngle)<<"alpha is stored as written";
	EXPECT_FALSE(oChain.m_vMaterialHolderPart[0].IsParameterConstrained());
	EXPECT_EQ(CHAIN_SEGMENT_MATERIAL_HOLDER,oChain.m_vMaterialHolderPart[1].GetType());
}

TEST(PugixmlReader, CommentsAreIgnored)
{
	std::string s = Replaced(Template(),"<tool_part>\n","<tool_part>\n<!-- the tool part -->\n");
	s = Replaced(s,"</segment>\n<segment type=\"TOOL\"","</segment>\n<!-- comment --> <segment type=\"TOOL\"");
	EXPECT_EQ(std::string(),PugiError(s));
}

TEST(PugixmlReader, MissingFileIsReported)
{
	std::string sError;
	try{
		CChainXmlReader::ReadFile(TempFilePath("kin5axTests_no_such_chain.kc"));
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	EXPECT_ERROR_CONTAINS(sError,"no_such_chain.kc");
}

TEST(PugixmlReader, BrokenXmlIsReportedWithTheLine)
{
	std::string sError = PugiError(Replaced(Template(),"</tool_part>\n",""));
	EXPECT_ERROR_CONTAINS(sError,"XML error");
	EXPECT_ERROR_CONTAINS(sError,"line ");
}

TEST(PugixmlReader, WrongRootNodeIsReported)
{
	std::string sError = PugiError("<?xml version=\"1.0\"?>\n<not_a_chain/>\n");
	EXPECT_ERROR_CONTAINS(sError,"root node");
	EXPECT_ERROR_CONTAINS(sError,"line 2");
}

TEST(PugixmlReader, MissingPartIsReported)
{
	std::string s = Template();
	size_t nBegin = s.find("<material_holder_part>"), nEnd = s.find("</material_holder_part>\n")+strlen("</material_holder_part>\n");
	std::string sError = PugiError(s.substr(0,nBegin)+s.substr(nEnd));
	EXPECT_ERROR_CONTAINS(sError,"<material_holder_part>");
}

TEST(PugixmlReader, EmptyPartIsReported)
{
	std::string s = Template();
	size_t nBegin = s.find("<segment type=\"REVOLUTE_LINK\" name=\"C\""), nEnd = s.find("</material_holder_part>");
	std::string sError = PugiError(s.substr(0,nBegin)+s.substr(nEnd));
	EXPECT_ERROR_CONTAINS(sError,"<material_holder_part> has no <segment>");
	EXPECT_ERROR_CONTAINS(sError,"line 18");
}

TEST(PugixmlReader, ToolMustBeTheLastSegmentOfTheToolPart)
{
	std::string s = Replaced(Template(),"<segment type=\"TOOL\" name=\"tool\">","<segment type=\"REVOLUTE_LINK\" name=\"A\">");
	s = Replaced(s,"<segment type=\"PRISMATIC_LINK\" name=\"Z\"","<segment type=\"TOOL\" name=\"Z\"");
	std::string sError = PugiError(s);
	EXPECT_ERROR_CONTAINS(sError,"TOOL");
}

TEST(PugixmlReader, OnlyOneToolSegment)
{
	std::string sError = PugiError(Replaced(Template(),"<segment type=\"REVOLUTE_LINK\" name=\"B\"","<segment type=\"TOOL\" name=\"B\""));
	EXPECT_ERROR_CONTAINS(sError,"TOOL");
}

TEST(PugixmlReader, MaterialHolderMustBeTheLastSegmentOfItsPart)
{
	std::string sError = PugiError(Replaced(Template(),"<segment type=\"REVOLUTE_LINK\" name=\"C\">","<segment type=\"MATERIAL_HOLDER\" name=\"C\">"));
	EXPECT_ERROR_CONTAINS(sError,"MATERIAL_HOLDER");
}

TEST(PugixmlReader, TerminalsInTheWrongPartAreReported)
{
	std::string sError = PugiError(Replaced(Template(),"<segment type=\"REVOLUTE_LINK\" name=\"B\"","<segment type=\"MATERIAL_HOLDER\" name=\"B\""));
	EXPECT_ERROR_CONTAINS(sError,"MATERIAL_HOLDER");
}

TEST(PugixmlReader, UnknownTypeIsReported)
{
	std::string sError = PugiError(Replaced(Template(),"type=\"REVOLUTE_LINK\" name=\"B\"","type=\"ROTARY\" name=\"B\""));
	EXPECT_ERROR_CONTAINS(sError,"ROTARY");
	EXPECT_ERROR_CONTAINS(sError,"line 9");
}

TEST(PugixmlReader, MissingAttributeIsReported)
{
	std::string sError = PugiError(Replaced(Template()," name=\"B\"",""));
	EXPECT_ERROR_CONTAINS(sError,"'name'");
	EXPECT_ERROR_CONTAINS(sError,"line 9");
}

TEST(PugixmlReader, NotANumberIsReported)
{
	std::string sError = PugiError(Replaced(Template(),"<initial_translation x=\"0\" y=\"0\" z=\"1\"/>","<initial_translation x=\"0\" y=\"1,5\" z=\"1\"/>"));
	EXPECT_ERROR_CONTAINS(sError,"'1,5'");
	EXPECT_ERROR_CONTAINS(sError,"line 5");
}

TEST(PugixmlReader, NumbersMayHaveBlanksAroundThem)
{
	EXPECT_EQ(std::string(),PugiError(Replaced(Template(),"z=\"1\"/>","z=\" 1.0e0 \"/>")));
}

TEST(PugixmlReader, OneSidedLimitIsReported)
{
	std::string sError = PugiError(Replaced(Template()," parameter_value_maximum=\"0.5\"",""));
	EXPECT_ERROR_CONTAINS(sError,"given together");
	EXPECT_ERROR_CONTAINS(sError,"line 4");
}

TEST(PugixmlReader, MinimumAboveMaximumIsReported)
{
	std::string sError = PugiError(Replaced(Template(),"parameter_value_minimum=\"-90\" parameter_value_maximum=\"90\"","parameter_value_minimum=\"90\" parameter_value_maximum=\"-90\""));
	EXPECT_ERROR_CONTAINS(sError,"above");
}

TEST(PugixmlReader, UnknownElementIsReported)
{
	std::string sError = PugiError(Replaced(Template(),"</segment>\n<segment type=\"TOOL\"","<colour/>\n</segment>\n<segment type=\"TOOL\""));
	EXPECT_ERROR_CONTAINS(sError,"<colour>");
	EXPECT_ERROR_CONTAINS(sError,"line 12");
}

TEST(PugixmlReader, UnknownAttributeIsReported)
{
	std::string sError = PugiError(Replaced(Template(),"<box x1=","<box colour=\"red\" x1="));
	EXPECT_ERROR_CONTAINS(sError,"'colour'");
	EXPECT_ERROR_CONTAINS(sError,"line 7");
}

TEST(PugixmlReader, ZeroRotationAxisWithAnAngleIsReported)
{
	std::string sError = PugiError(Replaced(Template(),"<initial_rotation x=\"1\" y=\"0\" z=\"0\" alpha=\"90\"/>","<initial_rotation x=\"0\" y=\"0\" z=\"0\" alpha=\"90\"/>"));
	EXPECT_ERROR_CONTAINS(sError,"zero vector");
	EXPECT_ERROR_CONTAINS(sError,"line 11");
}

TEST(PugixmlReader, ZeroRotationAxisWithoutAnAngleIsAccepted)
{
	EXPECT_EQ(std::string(),PugiError(Replaced(Template(),"<initial_rotation x=\"1\" y=\"0\" z=\"0\" alpha=\"0\"/>","<initial_rotation x=\"0\" y=\"0\" z=\"0\" alpha=\"0\"/>")));
}

TEST(PugixmlReader, DuplicateTransformationIsReported)
{
	std::string sError = PugiError(Replaced(Template(),"<initial_translation x=\"0\" y=\"0\" z=\"1\"/>\n","<initial_translation x=\"0\" y=\"0\" z=\"1\"/>\n<initial_translation x=\"0\" y=\"0\" z=\"2\"/>\n"));
	EXPECT_ERROR_CONTAINS(sError,"more than one <initial_translation>");
}

TEST(PugixmlReader, MissingTransformationIsReported)
{
	std::string sError = PugiError(Replaced(Template(),"<initial_rotation x=\"0\" y=\"0\" z=\"1\" alpha=\"0\"/>\n<box",""
		"<box"));
	EXPECT_ERROR_CONTAINS(sError,"has no <initial_rotation>");
	EXPECT_ERROR_CONTAINS(sError,"line 4");
}

//---- every test chain: a saved chain reads back identically
class PugixmlChainFile : public ::testing::TestWithParam<std::string> {};

struct PugixmlChainFileName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST_P(PugixmlChainFile, SavedChainReadsBackIdentically)
{
	std::string sFilePath = TempFilePath("kin5axTests_pugixml_roundtrip.kc");
	std::string sFirst, sSecond;
	try{
		sFirst = CChainXmlReader::ReadFile(GetParam()).toXML();
		WriteTemp("kin5axTests_pugixml_roundtrip.kc",sFirst);
		sSecond = CChainXmlReader::ReadFile(sFilePath).toXML();
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	remove(sFilePath.c_str());
	EXPECT_EQ(sFirst,sSecond);
}

INSTANTIATE_TEST_CASE_P(Files, PugixmlChainFile, ::testing::ValuesIn(GetChainFiles()), PugixmlChainFileName());

//---- the name of the machine (the 'name' attribute of the root), kept by CChain since this commit
TEST(ChainName, IsRead)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(Template(),&oChain));
	EXPECT_EQ("template",oChain.GetName());
}

TEST(ChainName, SurvivesTheXmlRoundTripAndIsEscaped)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(Replaced(Template(),"name=\"template\""
		,"name=\"A &quot;quoted&quot; &lt;name&gt; &amp; more\""),&oChain));
	EXPECT_EQ("A \"quoted\" <name> & more",oChain.GetName());

	std::string sFilePath = WriteTemp("kin5axTests_name_roundtrip.kc",oChain.toXML());
	std::string sReloaded;
	try{
		sReloaded = CChainXmlReader::ReadFile(sFilePath).GetName();
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<e.Message();
	}
	remove(sFilePath.c_str());
	EXPECT_EQ(oChain.GetName(),sReloaded);
}

TEST(ChainName, AbsentNameIsEmptyAndNothingIsWritten)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(Replaced(Template()," name=\"template\"",""),&oChain));
	EXPECT_EQ(std::string(),oChain.GetName());
	EXPECT_EQ(std::string::npos,oChain.toXML().find("<kinematic_chain name="))<<"no name attribute for an unnamed chain";
}

//---- machine, author and last modification: free text beside the name
static std::string WithMetadata()
{
	return Replaced(Template(),"name=\"template\""
		,"name=\"template\" machine=\"SHW UniSpeed 3500\" author=\"P. Zdroik\" modified=\"2026-09-22\"");
}

TEST(ChainMetadata, IsRead)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(WithMetadata(),&oChain));
	EXPECT_EQ("SHW UniSpeed 3500",oChain.GetMachine());
	EXPECT_EQ("P. Zdroik",oChain.GetAuthor());
	EXPECT_EQ("2026-09-22",oChain.GetModified());
}

TEST(ChainMetadata, SurvivesTheXmlRoundTrip)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(WithMetadata(),&oChain));
	std::string sFilePath = WriteTemp("kin5axTests_metadata_roundtrip.kc",oChain.toXML());
	CChain oReloaded;
	try{
		oReloaded = CChainXmlReader::ReadFile(sFilePath);
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<e.Message();
	}
	remove(sFilePath.c_str());
	EXPECT_EQ(oChain.GetName(),oReloaded.GetName());
	EXPECT_EQ(oChain.GetMachine(),oReloaded.GetMachine());
	EXPECT_EQ(oChain.GetAuthor(),oReloaded.GetAuthor());
	EXPECT_EQ(oChain.GetModified(),oReloaded.GetModified());
}

TEST(ChainMetadata, AbsentFieldsAreEmptyAndNothingIsWritten)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(Template(),&oChain));
	EXPECT_EQ(std::string(),oChain.GetMachine());
	EXPECT_EQ(std::string(),oChain.GetAuthor());
	EXPECT_EQ(std::string(),oChain.GetModified());
	std::string sXml = oChain.toXML();
	EXPECT_EQ(std::string::npos,sXml.find(" machine=\""));
	EXPECT_EQ(std::string::npos,sXml.find(" author=\""));
	EXPECT_EQ(std::string::npos,sXml.find(" modified=\""));
}

//---- the tool holder of the TOOL segment: the chain file declares the tool mount of the machine
static std::string WithToolHolder()
{
	return Replaced(Template(),"</segment>\n</tool_part>"
		,"<tool_holder length=\"0.05\" radius=\"0.03\"/>\n</segment>\n</tool_part>");
}

TEST(ToolHolderXml, IsReadFromTheToolSegment)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(WithToolHolder(),&oChain));
	EXPECT_TRUE(oChain.IsToolMountDeclared());
	EXPECT_DOUBLE_EQ(0.05,oChain.GetDefaultToolHolder().GetLength());
	EXPECT_DOUBLE_EQ(0.03,oChain.GetDefaultToolHolder().GetRadius());

	CChain oPlain;
	ASSERT_EQ(std::string(),PugiError(Template(),&oPlain));
	EXPECT_FALSE(oPlain.IsToolMountDeclared())<<"a file without a holder declares no mount";
	EXPECT_EQ(std::string::npos,oPlain.toXML().find("tool_holder length"))<<"and writes none";
}

TEST(ToolHolderXml, SurvivesTheRoundTrip)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(WithToolHolder(),&oChain));
	EXPECT_NE(std::string::npos,oChain.toXML().find("<tool_holder length=\"0.05\" radius=\"0.03\"/>"));
	std::string sFilePath = WriteTemp("kin5axTests_tool_holder.kc",oChain.toXML());
	CChain oReloaded;
	try{
		oReloaded = CChainXmlReader::ReadFile(sFilePath);
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<e.Message();
	}
	remove(sFilePath.c_str());
	EXPECT_TRUE(oReloaded.IsToolMountDeclared());
	EXPECT_TRUE(oReloaded.GetDefaultToolHolder()==oChain.GetDefaultToolHolder());
	EXPECT_EQ(oChain.toXML(),oReloaded.toXML());
}

TEST(ToolHolderXml, OnlyTheToolSegmentCarriesOne)
{
	std::string s = Replaced(Template(),"</segment>\n<segment type=\"REVOLUTE_LINK\" name=\"B\""
		,"<tool_holder length=\"0.05\" radius=\"0.03\"/>\n</segment>\n<segment type=\"REVOLUTE_LINK\" name=\"B\"");
	EXPECT_ERROR_CONTAINS(PugiError(s),"TOOL");
}

TEST(ToolHolderXml, AMissingAttributeIsReported)
{
	std::string s = Replaced(WithToolHolder(),"<tool_holder length=\"0.05\" radius=\"0.03\"/>","<tool_holder length=\"0.05\"/>");
	EXPECT_ERROR_CONTAINS(PugiError(s),"radius");
}

//---- the unit: required, exact, and kept

TEST(ChainUnit, IsReadFromTheRootElement)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(Template(),&oChain));
	EXPECT_EQ(CHAIN_UNIT_M,oChain.GetUnit());
	CChain oInches;
	ASSERT_EQ(std::string(),PugiError(Replaced(Template(),"unit=\"m\"","unit=\"in\""),&oInches));
	EXPECT_EQ(CHAIN_UNIT_IN,oInches.GetUnit());
}

TEST(ChainUnit, AFileWithoutOneIsRefused)
{
	//the numbers of a chain are not lengths until the file says what they are in; the models
	//written before the unit existed were all metres, and they all say so now rather than the
	//reader assuming it for the next file that forgets
	std::string sError = PugiError(Replaced(Template()," unit=\"m\"",""));
	EXPECT_ERROR_CONTAINS(sError,"unit");
	EXPECT_ERROR_CONTAINS(sError,"kinematic_chain");
}

TEST(ChainUnit, AnUnknownOneIsRefusedWithTheThreeThatExist)
{
	std::string sError = PugiError(Replaced(Template(),"unit=\"m\"","unit=\"cm\""));
	EXPECT_ERROR_CONTAINS(sError,"'cm'");
	EXPECT_ERROR_CONTAINS(sError,"mm, m and in");
	//case and synonyms are not read into, see ChainUnit.OnlyTheExactSpellingIsAUnit
	EXPECT_NE(std::string(),PugiError(Replaced(Template(),"unit=\"m\"","unit=\"M\"")));
}

TEST(ChainUnit, SurvivesTheXmlRoundTripAndTheDtdSaysItIsRequired)
{
	CChain oChain;
	ASSERT_EQ(std::string(),PugiError(Replaced(Template(),"unit=\"m\"","unit=\"in\""),&oChain));
	std::string sXml = oChain.toXML();
	EXPECT_NE(std::string::npos,sXml.find("unit (mm|m|in) #REQUIRED"))<<"the DTD of the file written";
	EXPECT_NE(std::string::npos,sXml.find("<kinematic_chain unit=\"in\""))<<"and the attribute, first";
	std::string sFilePath = WriteTemp("kin5axTests_unit_roundtrip.kc",sXml);
	CChain oReloaded;
	try{
		oReloaded = CChainXmlReader::ReadFile(sFilePath);
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<e.Message();
	}
	remove(sFilePath.c_str());
	EXPECT_EQ(CHAIN_UNIT_IN,oReloaded.GetUnit());
}
