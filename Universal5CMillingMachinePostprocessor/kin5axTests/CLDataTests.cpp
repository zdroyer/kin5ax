// CL-data tests: a generated hemisphere file is read back and postprocessed for every chain.
#include "../stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SphereCLDataGenerator.h"
#include "CutterLocationData.h"
#include "CLDataPostprocessor.h"
#include "MillingToolBall.h"

static const double dSPHERE_RADIUS = 0.05;
static const double dLOCATIONS_DISTANCE = 0.01;

static string TempFilePath(const string &sName)
{
	char szFolder[MAX_PATH];
	GetTempPath(MAX_PATH,szFolder);
	return string(szFolder)+sName;
}

static size_t CountLines(const string &sFileName)
{
	FILE *pFile = fopen(sFileName.c_str(),"rt");
	if(!pFile)
		return 0;
	size_t n=0;
	char szLine[512];
	while(fgets(szLine,sizeof(szLine),pFile))
		n++;
	fclose(pFile);
	return n;
}

//the beginning and the end of a text file (nChars each)
static string ReadBothEnds(const string &sFileName, size_t nChars)
{
	string sAll;
	FILE *pFile = fopen(sFileName.c_str(),"rt");
	if(!pFile)
		return sAll;
	int c;
	while((c=fgetc(pFile))!=EOF)
		sAll+=(char)c;
	fclose(pFile);
	if(sAll.size()<=2*nChars)
		return sAll;
	return sAll.substr(0,nChars)+"\n[...]\n"+sAll.substr(sAll.size()-nChars);
}

TEST(SphereCLData, IsReadBackByCutterLocationData)
{
	string sFileName = TempFilePath("kin5axTests_sphere.cld");
	size_t nWritten = WriteSphereCLData(sFileName,dSPHERE_RADIUS,dLOCATIONS_DISTANCE);
	ASSERT_GT(nWritten,(size_t)10);
	try{
		CCutterLocationData oData;
		oData.LoadFile(sFileName,"kin5axTests_sphere.cld");
		EXPECT_EQ((size_t)0,oData.GetErrorsCount());
		EXPECT_EQ(nWritten,oData.GetLocationsCount());
		EXPECT_EQ(nWritten,oData.GetLinesCount());

		CR3Point p;
		CR3Vector v;
		size_t nRead=0;
		oData.ResetLocationIterator();
		while(oData.GetNextLocation(p,v)){
			nRead++;
			//the file has 6 decimals
			EXPECT_NEAR(dSPHERE_RADIUS,d_E(p,CR3Point(0.0)),1E-5)<<"location "<<nRead;
			EXPECT_NEAR(1.0,v.GetLength(),1E-5)<<"location "<<nRead;
			EXPECT_NEAR(0.0,((p-CR3Point(0.0))/dSPHERE_RADIUS - v).GetLength(),1E-4)<<"the tool axis is not normal to the sphere at location "<<nRead;
			EXPECT_GE(p.GetZ(),-1E-9);
		}
		EXPECT_EQ(nWritten,nRead);
		//the last location is the pole
		EXPECT_NEAR(dSPHERE_RADIUS,p.GetZ(),1E-6);
		EXPECT_NEAR(1.0,v.GetZ(),1E-6);
		EXPECT_GT(oData.GetTrajectoryLength(),0.0);
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	remove(sFileName.c_str());
}

class SphereCLDataOnChainTest : public ::testing::TestWithParam<string>
{
};

struct SphereCLDataOnChainTestName
{
	string operator()(const ::testing::TestParamInfo<string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

//CL-data file -> CCLDataPostprocessor -> shortest path -> G-code file
TEST_P(SphereCLDataOnChainTest, IsPostprocessedToGCode)
{
	string sCLDataFile = TempFilePath("kin5axTests_sphere_"+GetTestName(GetParam())+".cld");
	string sGCodeFile = TempFilePath("kin5axTests_sphere_"+GetTestName(GetParam())+".nc");
	size_t nWritten = WriteSphereCLData(sCLDataFile,dSPHERE_RADIUS,dLOCATIONS_DISTANCE);
	ASSERT_GT(nWritten,(size_t)10);
	try{
		CChain oChain = LoadChain(GetParam());
		ASSERT_TRUE(oChain.Is5CChain());
		CMillingToolBall oTool(0.016,0.016,0.064,0.016);	//the default tool of the application
		CCutterLocationData oData;
		oData.LoadFile(sCLDataFile,"sphere");
		ASSERT_EQ(nWritten,oData.GetLocationsCount());

		CPostprocessorSettings oSettings;
		CSimulationData oSimulationData;
		CCLDataPostprocessor oPostprocessor(oSettings);
		CCLDataPostprocessingResult oResult = oPostprocessor.Process(oChain,oTool,oData,oSimulationData,NULL,0,NULL);

		//the interpolation only adds problems; every problem gives one (possibly empty) solution set
		EXPECT_GE(oResult.m_nTotalProblemsCount,nWritten);
		EXPECT_GE(oSimulationData.GetSize(),nWritten);
		size_t nSolved = oSimulationData.GetSize()-oSimulationData.GetEmptySolutionsCount();
		if(nSolved==0){
			//run again with the detailed report: the solver log tells why
			string sReportFile = TempFilePath("kin5axTests_sphere_"+GetTestName(GetParam())+".txt");
			FILE *pReport = fopen(sReportFile.c_str(),"wt");
			CSimulationData oIgnored;
			oPostprocessor.Process(oChain,oTool,oData,oIgnored,pReport,0,NULL);
			fclose(pReport);
			ADD_FAILURE()<<"no cutter location of the hemisphere could be solved; the report begins and ends with:\n"<<ReadBothEnds(sReportFile,900);
			remove(sReportFile.c_str());
		}
		RecordProperty("problems",(int)oResult.m_nTotalProblemsCount);
		RecordProperty("solved",(int)nSolved);

		//every returned solution reproduces its cutter location
		EXPECT_LT(oResult.m_dMaxPositionError,1E-9)<<"input "<<oResult.m_cvMaxPositionErrorInput.m_p.toString()
			<<" solution "<<oResult.m_oMaxPositionErrorSolution.toString();
		EXPECT_LT(oResult.m_dMaxOrientationErrorVectorLength,1E-9)<<"input "<<oResult.m_cvMaxOrientationErrorInput.m_v.toString()
			<<" solution "<<oResult.m_oMaxOrientationErrorSolution.toString();

		oSimulationData.FindShortestPath(oSettings,NULL);

		//one G-code line for every solved problem
		size_t nFirst=0,nSecond=0,nThird=0,nAngle1=0,nAngle2=0;
		oChain.GetTranslationsIndexes(nFirst,nSecond,nThird);
		oChain.GetRotationsIndexes(nAngle1,nAngle2);
		FILE *pFile = fopen(sGCodeFile.c_str(),"wt");
		ASSERT_TRUE(pFile!=NULL);
		int nExport = oSimulationData.ExportToGCode(pFile,nFirst,nSecond,nThird,65,nAngle1,67,nAngle2,false);	//letters A and C
		fclose(pFile);
		EXPECT_EQ(0,nExport);
		EXPECT_EQ(nSolved,CountLines(sGCodeFile));
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	remove(sCLDataFile.c_str());
	remove(sGCodeFile.c_str());
}

INSTANTIATE_TEST_CASE_P(AllChains, SphereCLDataOnChainTest, ::testing::ValuesIn(GetChainFiles()), SphereCLDataOnChainTestName());
