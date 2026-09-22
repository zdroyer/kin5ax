// CL-data along a meridian over the north pole: 9 cutter locations, the middle one on the pole
// with the tool axis (0,0,1) - the path crosses the singular orientation of most machines.
#include "../stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "SphereCLDataGenerator.h"
#include "CutterLocationData.h"
#include "CLDataPostprocessor.h"
#include "MillingToolBall.h"

static const double dRADIUS = 0.05;
static const size_t nPOINTS = 9;

static string MeridianTempFilePath(const string &sName)
{
	char szFolder[MAX_PATH];
	GetTempPath(MAX_PATH,szFolder);
	return string(szFolder)+sName;
}

TEST(MeridianCLData, HasThePoleInTheMiddle)
{
	string sFileName = MeridianTempFilePath("kin5axTests_meridian.cld");
	ASSERT_EQ(nPOINTS,WriteMeridianCLData(sFileName,dRADIUS,nPOINTS));
	EXPECT_EQ((size_t)0,WriteMeridianCLData(sFileName+"_",dRADIUS,10))<<"more than 9 points or an even number must be refused";
	try{
		CCutterLocationData oData;
		oData.LoadFile(sFileName,"meridian");
		EXPECT_EQ((size_t)0,oData.GetErrorsCount());
		ASSERT_EQ(nPOINTS,oData.GetLocationsCount());
		ASSERT_LE(oData.GetLocationsCount(),(size_t)10);

		CR3Point p;
		CR3Vector v;
		for(size_t i=0;i<nPOINTS;i++){
			ASSERT_TRUE(oData.GetLocation(p,v,i))<<"line "<<i;
			//on the sphere, in the XZ plane, northern hemisphere, tool axis normal to the surface (the file has 6 decimals)
			EXPECT_NEAR(dRADIUS,d_E(p,CR3Point(0.0)),1E-5)<<"line "<<i;
			EXPECT_NEAR(0.0,p.GetY(),1E-9)<<"line "<<i;
			EXPECT_GE(p.GetZ(),-1E-9)<<"line "<<i;
			EXPECT_NEAR(0.0,((p-CR3Point(0.0))/dRADIUS - v).GetLength(),1E-4)<<"line "<<i;
			if(i==nPOINTS/2){
				EXPECT_NEAR(0.0,p.GetX(),1E-9);
				EXPECT_NEAR(dRADIUS,p.GetZ(),1E-9);
				EXPECT_NEAR(1.0,v.GetZ(),1E-9);
			} else {
				//west of the pole before the middle point, east of it after
				EXPECT_EQ(i<nPOINTS/2,p.GetX()<0.0)<<"line "<<i;
			}
		}
	} catch (CGeneratorException &e){
		FAIL()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	remove(sFileName.c_str());
}

class MeridianCLDataOnChainTest : public ::testing::TestWithParam<string>
{
};

struct MeridianCLDataOnChainTestName
{
	string operator()(const ::testing::TestParamInfo<string> &oInfo) const
	{ return GetTestName(oInfo.param); }
};

//the biggest change of a joint between two neighbouring solved problems of the chosen path
static void GetBiggestSteps(const CSimulationData &oSimulationData, double &dMaxAngleStep, double &dMaxShiftStep)
{
	dMaxAngleStep = dMaxShiftStep = 0.0;
	bool bHasPrevious = false;
	CChainConfiguration oPrevious;
	for(size_t i=0;i<oSimulationData.GetSize();i++){
		try{
			const CChainConfiguration &oActual = oSimulationData[i];	//throws for a problem without solution
			if(bHasPrevious){
				for(size_t k=0;k<oActual.GetParametersCount();k++){
					double d = oActual[k].distanceTo(oPrevious[k]);
					if(oActual[k].IsAngle())
						dMaxAngleStep = max(dMaxAngleStep,d);
					else
						dMaxShiftStep = max(dMaxShiftStep,d);
				}
			}
			oPrevious = oActual;
			bHasPrevious = true;
		} catch (CGeneratorException &){
		}
	}
}

TEST_P(MeridianCLDataOnChainTest, IsPostprocessedOverThePole)
{
	string sCLDataFile = MeridianTempFilePath("kin5axTests_meridian_"+GetTestName(GetParam())+".cld");
	ASSERT_EQ(nPOINTS,WriteMeridianCLData(sCLDataFile,dRADIUS,nPOINTS));
	try{
		CChain oChain = LoadChain(GetParam());
		ASSERT_TRUE(oChain.Is5CChain());
		CMillingToolBall oTool(0.016,0.016,0.064,0.016);	//the default tool of the application
		CCutterLocationData oData;
		oData.LoadFile(sCLDataFile,"meridian");
		ASSERT_EQ(nPOINTS,oData.GetLocationsCount());

		CPostprocessorSettings oSettings;
		CSimulationData oSimulationData;
		CCLDataPostprocessor oPostprocessor(oSettings);
		CCLDataPostprocessingResult oResult = oPostprocessor.Process(oChain,oTool,oData,oSimulationData,NULL,0,NULL);

		//22.5 deg between the locations and 1 deg allowed by the settings: the path is interpolated
		EXPECT_GT(oResult.m_nTotalProblemsCount,(size_t)100);
		size_t nSolved = oSimulationData.GetSize()-oSimulationData.GetEmptySolutionsCount();
		EXPECT_GT(nSolved,(size_t)0);
		EXPECT_LT(oResult.m_dMaxPositionError,1E-9);
		EXPECT_LT(oResult.m_dMaxOrientationErrorVectorLength,1E-9);

		oSimulationData.FindShortestPath(oSettings,NULL);
		double dMaxAngleStep, dMaxShiftStep;
		GetBiggestSteps(oSimulationData,dMaxAngleStep,dMaxShiftStep);
		//The chosen path is smooth (1 deg interpolation), except that a machine which can tilt to one side
		//only has to turn its rotary axis by 180 deg once. The path length is a sum of absolute joint
		//changes, so one 180 deg step and two 90 deg steps around the pole cost the same: which of them is
		//chosen depends on the last bits of the arithmetic (x64: 90 deg, Win32: 180 deg for WM308S2).
		//More than one such turn in a single step would be a real jump.
		EXPECT_LE(radians_to_degrees(dMaxAngleStep),180.0+1E-6);
		EXPECT_LT(dMaxShiftStep,0.01);
		RecordProperty("problems",(int)oResult.m_nTotalProblemsCount);
		RecordProperty("solved",(int)nSolved);
		RecordProperty("max_angle_step_mdeg",(int)(1000.0*radians_to_degrees(dMaxAngleStep)));
		RecordProperty("max_shift_step_um",(int)(1E6*dMaxShiftStep));
	} catch (CGeneratorException &e){
		ADD_FAILURE()<<"exception: "<<e.Message()<<" ("<<e.GetThrowingPlace()<<")";
	}
	remove(sCLDataFile.c_str());
}

INSTANTIATE_TEST_CASE_P(AllChains, MeridianCLDataOnChainTest, ::testing::ValuesIn(GetChainFiles()), MeridianCLDataOnChainTestName());
