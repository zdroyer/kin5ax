// The bridge to the FreeCAD CAM machine file (*.fcm): CChainFcmReader and CChainFcmWriter.
// The geometry is checked where it can be checked without either converter: the joint of an
// imported chain, found by the forward kinematics of the model itself, must stand where the file
// put it, and driving a joint must move the tool the way the file says.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "ChainFcm.h"
#include "ChainXmlReader.h"
#include "double_tools.h"

using namespace kin5ax;

//a BC table machine: X, Y, Z in the head, B and C under the workpiece
static std::string Template()
{
	return R"FCM({
  "freecad_version": "1.1",
  "machine": {
    "name": "Test BC table",
    "manufacturer": "kin5axTests",
    "units": "metric",
    "kinematics": { "base_frame": { "origin": [0, 0, 0], "orientation_quaternion": [0, 0, 0, 1] } },
    "axes": {
      "X": { "type": "linear", "role": "head_linear", "parent": null, "sequence": 0,
             "joint": [[0, 0, 0], [1, 0, 0]], "limits": { "min": -350, "max": 350 } },
      "Y": { "type": "linear", "role": "head_linear", "parent": "X", "sequence": 1,
             "joint": [[0, 0, 0], [0, 1, 0]], "limits": { "min": -250, "max": 250 } },
      "Z": { "type": "linear", "role": "head_linear", "parent": "Y", "sequence": 2,
             "joint": [[0, 0, 0], [0, 0, 1]], "limits": { "min": -400, "max": 0 } },
      "B": { "type": "rotary", "role": "table_rotary", "parent": null, "sequence": 0,
             "joint": [[0, 0, -80], [0, 1, 0]], "limits": { "min": -110, "max": 110 } },
      "C": { "type": "rotary", "role": "table_rotary", "parent": "B", "sequence": 1,
             "joint": [[0, 0, -80], [0, 0, 1]], "limits": { "min": -360, "max": 360 } }
    },
    "toolheads": []
  },
  "version": 1
}
)FCM";
}

static std::string Replaced(const std::string &sText, const std::string &sOld, const std::string &sNew)
{
	size_t n = sText.find(sOld);
	EXPECT_NE(std::string::npos,n)<<"the template does not contain: "<<sOld;
	return n==std::string::npos ? sText : sText.substr(0,n)+sNew+sText.substr(n+sOld.size());
}

static std::string FcmError(const std::string &sContent, CChain *pChain = NULL, CFcmReport *pReport = NULL)
{
	CFcmOptions oOptions;
	CFcmReport oReport;
	try{
		CChain oChain = CChainFcmReader::Parse(sContent,std::string(),oOptions,oReport);
		if(pChain)
			*pChain = oChain;
		if(pReport)
			*pReport = oReport;
	} catch (CGeneratorException &e){
		return e.Message();
	}
	return std::string();
}

static CChain Imported(const std::string &sContent)
{
	CChain oChain;
	EXPECT_EQ(std::string(),FcmError(sContent,&oChain));
	return oChain;
}

//the frame of segment n of a part at the home position, by the forward kinematics of the model
static CTransformation FrameAt(const std::vector<CChainSegment> &vPart, size_t nCount)
{
	CTransformation tr(1.0);
	for(size_t i=0;i<nCount;i++)
		tr = tr*vPart[i].GetConstantTransformation();
	return tr;
}

static void ExpectVectorNear(const CR3Vector &expected, const CR3Vector &actual, const std::string &sWhat)
{
	EXPECT_NEAR(expected.GetX(),actual.GetX(),1E-9)<<sWhat;
	EXPECT_NEAR(expected.GetY(),actual.GetY(),1E-9)<<sWhat;
	EXPECT_NEAR(expected.GetZ(),actual.GetZ(),1E-9)<<sWhat;
}

#define EXPECT_ERROR_CONTAINS(sError, sText) EXPECT_NE(std::string::npos,(sError).find(sText))<<"message: "<<(sError)

TEST(ChainFcmReader, TemplateLoads)
{
	CChain oChain = Imported(Template());
	ASSERT_EQ((size_t)4,oChain.m_vToolPart.size())<<"X, Y, Z and the tool";
	ASSERT_EQ((size_t)3,oChain.m_vMaterialHolderPart.size())<<"B, C and the workpiece holder";
	EXPECT_EQ("Test BC table",oChain.GetName());
	EXPECT_EQ("X",oChain.m_vToolPart[0].GetName());
	EXPECT_EQ(CHAIN_SEGMENT_PRISMATIC_LINK,oChain.m_vToolPart[0].GetType());
	EXPECT_EQ(CHAIN_SEGMENT_REVOLUTE_LINK,oChain.m_vMaterialHolderPart[0].GetType());
	EXPECT_EQ(CHAIN_SEGMENT_TOOL,oChain.m_vToolPart[3].GetType());
	EXPECT_EQ(CHAIN_SEGMENT_MATERIAL_HOLDER,oChain.m_vMaterialHolderPart[2].GetType());
	EXPECT_TRUE(oChain.Is5CChain())<<"three linear and two rotary joints";
}

//the heart of the import: every joint has to end up where the file says it is
TEST(ChainFcmReader, TheJointsStandWhereTheFilePutsThem)
{
	CChain oChain = Imported(Template());
	//tool branch: X, Y, Z through the origin, along the three axes
	CR3Vector vDirections[3] = {CR3Vector(1.0,0.0,0.0),CR3Vector(0.0,1.0,0.0),CR3Vector(0.0,0.0,1.0)};
	for(size_t i=0;i<3;i++){
		CTransformation tr = FrameAt(oChain.m_vToolPart,i+1);
		ExpectVectorNear(vDirections[i],tr*CR3Vector(0.0,0.0,1.0)
			,"the local OZ of tool joint "+size_to_string(i)+" is the axis of the file");
		CR3Point p = tr*CR3Point(0.0);
		EXPECT_NEAR(0.0,p.toR3Vector().GetLength(),1E-9)<<"tool joint "<<i<<" goes through the origin";
	}
	//workpiece branch: B about OY and C about OZ, both through (0,0,-80)
	CTransformation trB = FrameAt(oChain.m_vMaterialHolderPart,1);
	ExpectVectorNear(CR3Vector(0.0,1.0,0.0),trB*CR3Vector(0.0,0.0,1.0),"B turns about OY");
	ExpectVectorNear(CR3Vector(0.0,0.0,-80.0),(trB*CR3Point(0.0)).toR3Vector(),"B goes through (0,0,-80)");
	CTransformation trC = FrameAt(oChain.m_vMaterialHolderPart,2);
	ExpectVectorNear(CR3Vector(0.0,0.0,1.0),trC*CR3Vector(0.0,0.0,1.0),"C turns about OZ");
	ExpectVectorNear(CR3Vector(0.0,0.0,-80.0),(trC*CR3Point(0.0)).toR3Vector(),"C goes through (0,0,-80)");
}

TEST(ChainFcmReader, BothFramesAreTheBaseFrameAtHome)
{
	CChain oChain = Imported(Template());
	CTransformation trTool = FrameAt(oChain.m_vToolPart,oChain.m_vToolPart.size());
	ExpectVectorNear(CR3Vector(0.0,0.0,1.0),trTool*CR3Vector(0.0,0.0,1.0),"the tool axis is OZ at home");
	EXPECT_NEAR(0.0,(trTool*CR3Point(0.0)).toR3Vector().GetLength(),1E-9)<<"the tool point is the origin";
	CTransformation trWork = FrameAt(oChain.m_vMaterialHolderPart,oChain.m_vMaterialHolderPart.size());
	ExpectVectorNear(CR3Vector(0.0,0.0,1.0),trWork*CR3Vector(0.0,0.0,1.0),"the workpiece frame is upright");
	EXPECT_NEAR(0.0,(trWork*CR3Point(0.0)).toR3Vector().GetLength(),1E-9)<<"and on the origin";
}

//driving a joint of the imported chain has to move the tool the way the file says
TEST(ChainFcmReader, DrivingAJointMovesTheToolAsTheFileSays)
{
	CChain oChain = Imported(Template());
	CChainConfiguration oConfiguration = oChain.GetParameters();
	ASSERT_EQ((size_t)5,oConfiguration.GetParametersCount());
	//the parameters run from the workpiece holder to the tool: C, B, X, Y, Z
	oConfiguration[2].SetValue(10.0);		//X
	oConfiguration[4].SetValue(-30.0);		//Z
	oChain.SetParameters(oConfiguration);
	CR3AnchoredVector cv = oChain.GetActualToolPositionAndOrientation();
	ExpectVectorNear(CR3Vector(10.0,0.0,-30.0),cv.m_p.toR3Vector(),"X = 10 and Z = -30 move the tool there");
	ExpectVectorNear(CR3Vector(0.0,0.0,1.0),cv.m_v,"the linear joints do not tilt the tool");
}

TEST(ChainFcmReader, RotaryLimitsBecomeRadians)
{
	CChain oChain = Imported(Template());
	const CChainSegment &b = oChain.m_vMaterialHolderPart[0];
	EXPECT_TRUE(b.IsParameterConstrained());
	EXPECT_NEAR(degrees_to_radians(-110.0),b.m_dParameterValueMinimum,1E-12);
	EXPECT_NEAR(degrees_to_radians(110.0),b.m_dParameterValueMaximum,1E-12);
	const CChainSegment &x = oChain.m_vToolPart[0];
	EXPECT_NEAR(-350.0,x.m_dParameterValueMinimum,1E-12)<<"a linear limit is a length";
}

TEST(ChainFcmReader, AxesAreOrderedByParentNotByTheFile)
{
	//listed Z, X, Y and numbered the other way round; "parent" still gives X, Y, Z
	std::string s = R"FCM({
  "machine": { "name": "order", "axes": {
    "Z": { "type": "linear", "role": "head_linear", "parent": "Y", "sequence": 0, "joint": [[0,0,0],[0,0,1]] },
    "X": { "type": "linear", "role": "head_linear", "sequence": 2, "joint": [[0,0,0],[1,0,0]] },
    "Y": { "type": "linear", "role": "head_linear", "parent": "X", "sequence": 1, "joint": [[0,0,0],[0,1,0]] },
    "C": { "type": "rotary", "role": "table_rotary", "joint": [[0,0,0],[0,0,1]] }
  } } }
)FCM";
	CChain oChain = Imported(s);
	ASSERT_EQ((size_t)4,oChain.m_vToolPart.size());
	EXPECT_EQ("X",oChain.m_vToolPart[0].GetName());
	EXPECT_EQ("Y",oChain.m_vToolPart[1].GetName());
	EXPECT_EQ("Z",oChain.m_vToolPart[2].GetName());
}

TEST(ChainFcmReader, BrokenParentLinksFallBackToSequence)
{
	CChain oChain;
	CFcmReport oReport;
	std::string s = Replaced(Template(),"\"parent\": \"X\"","\"parent\": \"nowhere\"");
	ASSERT_EQ(std::string(),FcmError(s,&oChain,&oReport));
	EXPECT_EQ("X",oChain.m_vToolPart[0].GetName());
	EXPECT_EQ("Z",oChain.m_vToolPart[2].GetName());
	EXPECT_ERROR_CONTAINS(oReport.ToString(),"'sequence'");
}

TEST(ChainFcmReader, AMissingRoleIsReported)
{
	CChain oChain;
	CFcmReport oReport;
	ASSERT_EQ(std::string(),FcmError(Replaced(Template(),"\"role\": \"head_linear\", ",""),&oChain,&oReport));
	EXPECT_ERROR_CONTAINS(oReport.ToString(),"no 'role'");
}

TEST(ChainFcmReader, FieldsWithNoPlaceInTheChainAreReported)
{
	CChain oChain;
	CFcmReport oReport;
	std::string s = Replaced(Template(),"\"joint\": [[0, 0, 0], [1, 0, 0]]","\"max_velocity\": 10000, \"joint\": [[0, 0, 0], [1, 0, 0]]");
	ASSERT_EQ(std::string(),FcmError(s,&oChain,&oReport));
	EXPECT_ERROR_CONTAINS(oReport.ToString(),"max_velocity");
}

TEST(ChainFcmReader, AMovedBaseFrameIsReported)
{
	CChain oChain;
	CFcmReport oReport;
	std::string s = Replaced(Template(),"\"origin\": [0, 0, 0]","\"origin\": [1, 2, 3]");
	ASSERT_EQ(std::string(),FcmError(s,&oChain,&oReport));
	EXPECT_ERROR_CONTAINS(oReport.ToString(),"base_frame");
}

TEST(ChainFcmReader, ErrorsAreReported)
{
	EXPECT_ERROR_CONTAINS(FcmError("{ \"machine\": { } }"),"no machine axes");
	EXPECT_ERROR_CONTAINS(FcmError(Replaced(Template(),"\"type\": \"rotary\", \"role\": \"table_rotary\", \"parent\": null","\"type\": \"swivel\", \"role\": \"table_rotary\", \"parent\": null")),"swivel");
	EXPECT_ERROR_CONTAINS(FcmError(Replaced(Template(),"\"joint\": [[0, 0, -80], [0, 1, 0]]","\"joint\": [[0, 0, 0], [0, 0, 0]]")),"direction");
	EXPECT_ERROR_CONTAINS(FcmError(Replaced(Template(),"\"joint\": [[0, 0, 0], [1, 0, 0]], ","")),"'joint'");
	EXPECT_ERROR_CONTAINS(FcmError(Replaced(Template(),"\"min\": -110, \"max\": 110","\"min\": 110, \"max\": -110")),"min is above max");
	EXPECT_ERROR_CONTAINS(FcmError("{ not json"),"JSON error");
}

//machine.py accepts the direction before the origin; so does the reader, and it says so
TEST(ChainFcmReader, TheReversedJointLayoutIsAcceptedAndReported)
{
	CChain oChain;
	CFcmReport oReport;
	std::string s = Replaced(Template(),"\"joint\": [[0, 0, -80], [0, 1, 0]]","\"joint\": [[0, 1, 0], [0, 0, 0]]");
	ASSERT_EQ(std::string(),FcmError(s,&oChain,&oReport));
	EXPECT_ERROR_CONTAINS(oReport.ToString(),"before the origin");
	CTransformation trB = FrameAt(oChain.m_vMaterialHolderPart,1);
	ExpectVectorNear(CR3Vector(0.0,1.0,0.0),trB*CR3Vector(0.0,0.0,1.0),"the direction was taken from the first entry");
}

TEST(ChainFcmReader, TheAxisDirectionOfATableJointCanBeNegated)
{
	CFcmOptions oOptions;
	oOptions.m_bNegateTableRotaryAxes = true;
	CFcmReport oReport;
	CChain oChain = CChainFcmReader::Parse(Template(),std::string(),oOptions,oReport);
	CTransformation trB = FrameAt(oChain.m_vMaterialHolderPart,1);
	ExpectVectorNear(CR3Vector(0.0,-1.0,0.0),trB*CR3Vector(0.0,0.0,1.0),"B turns the other way");
}

//---- writing

TEST(ChainFcmWriter, WhatComesBackIsTheSameMachine)
{
	CChain oChain = Imported(Template());
	CFcmOptions oOptions;
	CFcmReport oReport;
	std::string sFcm = CChainFcmWriter::ToFcm(oChain,oOptions,oReport);
	EXPECT_TRUE(oReport.IsExact())<<oReport.ToString();
	CChain oBack = Imported(sFcm);
	ASSERT_EQ(oChain.m_vToolPart.size(),oBack.m_vToolPart.size());
	ASSERT_EQ(oChain.m_vMaterialHolderPart.size(),oBack.m_vMaterialHolderPart.size());
	for(size_t i=0;i<oChain.m_vToolPart.size();i++){
		CTransformation a = FrameAt(oChain.m_vToolPart,i+1);
		CTransformation b = FrameAt(oBack.m_vToolPart,i+1);
		ExpectVectorNear(a*CR3Vector(0.0,0.0,1.0),b*CR3Vector(0.0,0.0,1.0),"tool joint "+size_to_string(i));
		ExpectVectorNear((a*CR3Point(0.0)).toR3Vector(),(b*CR3Point(0.0)).toR3Vector(),"tool origin "+size_to_string(i));
	}
	for(size_t i=0;i<oChain.m_vMaterialHolderPart.size();i++){
		CTransformation a = FrameAt(oChain.m_vMaterialHolderPart,i+1);
		CTransformation b = FrameAt(oBack.m_vMaterialHolderPart,i+1);
		ExpectVectorNear(a*CR3Vector(0.0,0.0,1.0),b*CR3Vector(0.0,0.0,1.0),"workpiece joint "+size_to_string(i));
		ExpectVectorNear((a*CR3Point(0.0)).toR3Vector(),(b*CR3Point(0.0)).toR3Vector(),"workpiece origin "+size_to_string(i));
	}
}

TEST(ChainFcmWriter, TheFileIsStable)
{
	CChain oChain = Imported(Template());
	CFcmOptions oOptions;
	CFcmReport oReport;
	std::string sFirst = CChainFcmWriter::ToFcm(oChain,oOptions,oReport);
	std::string sSecond = CChainFcmWriter::ToFcm(Imported(sFirst),oOptions,oReport);
	EXPECT_EQ(sFirst,sSecond);
}

TEST(ChainFcmWriter, TwoSegmentsOfTheSameNameAreRefused)
{
	CChain oChain = Imported(Template());
	oChain.m_vToolPart[1].SetName("X");
	CFcmOptions oOptions;
	CFcmReport oReport;
	std::string sError;
	try{
		CChainFcmWriter::ToFcm(oChain,oOptions,oReport);
	} catch (CGeneratorException &e){
		sError = e.Message();
	}
	EXPECT_ERROR_CONTAINS(sError,"named 'X'");
}

TEST(ChainFcmWriter, AnAxisThatIsNotAGCodeLetterIsReported)
{
	CChain oChain = Imported(Template());
	oChain.m_vToolPart[1].SetName("Yaw");
	CFcmOptions oOptions;
	CFcmReport oReport;
	CChainFcmWriter::ToFcm(oChain,oOptions,oReport);
	EXPECT_ERROR_CONTAINS(oReport.ToString(),"'Yaw'");
}

//---- what an .fcm file cannot hold, on the chains of the test set
class ChainFcmFile : public ::testing::TestWithParam<std::string> {};

struct ChainFcmFileName
{
	std::string operator()(const ::testing::TestParamInfo<std::string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

TEST_P(ChainFcmFile, ExportSaysWhatItCannotCarry)
{
	CChain oChain;
	try{
		oChain = CChainXmlReader::ReadFile(GetParam());
	} catch (CGeneratorException &e){
		FAIL()<<e.Message();
	}
	CFcmOptions oOptions;
	CFcmReport oReport;
	std::string sFcm;
	try{
		sFcm = CChainFcmWriter::ToFcm(oChain,oOptions,oReport);
	} catch (CGeneratorException &e){
		//the names of the segments of a hand written chain need not be axis letters, but they
		//do have to differ: that is the only reason an export may refuse the chain
		EXPECT_ERROR_CONTAINS(e.Message(),"named");
		return;
	}
	EXPECT_FALSE(sFcm.empty());
	//every one of them carries collision boxes, which an .fcm file has no room for
	EXPECT_ERROR_CONTAINS(oReport.ToString(),"collision boxes");
	//and the file has to be readable again
	CChain oBack = Imported(sFcm);
	EXPECT_EQ(oChain.GetNumberOfRevoluteLinks(),oBack.GetNumberOfRevoluteLinks());
	EXPECT_EQ(oChain.GetNumberOfPrismaticLinks(),oBack.GetNumberOfPrismaticLinks());
}

INSTANTIATE_TEST_CASE_P(Files, ChainFcmFile, ::testing::ValuesIn(GetChainFiles()), ChainFcmFileName());
