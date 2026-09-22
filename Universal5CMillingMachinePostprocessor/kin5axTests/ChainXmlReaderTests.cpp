// The two .kc readers: CChainXmlReaderPugixml (portable, explicit checks) against CChainXmlReader
// (MSXML) on every test chain, and the checks of the pugixml reader on deliberately broken files.
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
	"<kinematic_chain name=\"template\">\n"                                                                              // 2
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
		CChain oChain = CChainXmlReaderPugixml::ReadFile(sFilePath);
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
		CChainXmlReaderPugixml::ReadFile(TempFilePath("kin5axTests_no_such_chain.kc"));
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

//the structure check is shared: the MSXML reader rejects the same chains
TEST(MsxmlReader, AppliesTheStructureCheck)
{
	std::string s = Replaced(Template(),"<segment type=\"REVOLUTE_LINK\" name=\"B\"","<segment type=\"TOOL\" name=\"B\"");
	std::string sFilePath = WriteTemp("kin5axTests_msxml_structure.kc",s);
	std::string sError;
	try{
		CChainXmlReader::ReadFile(sFilePath);
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	remove(sFilePath.c_str());
	EXPECT_ERROR_CONTAINS(sError,"TOOL");
}

//---- every test chain: both readers give the same chain, and a saved chain reads back identically
class PugixmlChainFile : public ::testing::TestWithParam<std::string> {};

struct PugixmlChainFileName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST_P(PugixmlChainFile, EqualsTheMsxmlReader)
{
	std::string sPugi, sMsxml;
	try{
		sPugi = CChainXmlReaderPugixml::ReadFile(GetParam()).toXML();
		sMsxml = CChainXmlReader::ReadFile(GetParam()).toXML();
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	EXPECT_FALSE(sPugi.empty());
	EXPECT_EQ(sMsxml,sPugi);
}

TEST_P(PugixmlChainFile, SavedChainReadsBackIdentically)
{
	std::string sFilePath = TempFilePath("kin5axTests_pugixml_roundtrip.kc");
	std::string sFirst, sSecond;
	try{
		sFirst = CChainXmlReaderPugixml::ReadFile(GetParam()).toXML();
		WriteTemp("kin5axTests_pugixml_roundtrip.kc",sFirst);
		sSecond = CChainXmlReaderPugixml::ReadFile(sFilePath).toXML();
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	remove(sFilePath.c_str());
	EXPECT_EQ(sFirst,sSecond);
}

INSTANTIATE_TEST_CASE_P(Files, PugixmlChainFile, ::testing::ValuesIn(GetChainFiles()), PugixmlChainFileName());

//---- the name of the machine (the 'name' attribute of the root), kept by CChain since this commit
TEST(ChainName, IsReadByBothReaders)
{
	CChain oPugi;
	ASSERT_EQ(std::string(),PugiError(Template(),&oPugi));
	EXPECT_EQ("template",oPugi.GetName());

	std::string sFilePath = WriteTemp("kin5axTests_name.kc",Template());
	CChain oMsxml;
	try{
		oMsxml = CChainXmlReader::ReadFile(sFilePath);
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<e.Message();
	}
	remove(sFilePath.c_str());
	EXPECT_EQ("template",oMsxml.GetName());
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
		sReloaded = CChainXmlReaderPugixml::ReadFile(sFilePath).GetName();
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
