// CCLDataPostprocessingResult::m_vLocations: one record per solution set handed to the CSimulationData -
// the status, where the location came from, and which cutter location of the file it belongs to.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "CutterLocationData.h"
#include "CLDataPostprocessor.h"
#include "MillingToolBall.h"
#include "SphereCLDataGenerator.h"
#include "5CPostprocessor.h"
#include <algorithm>

using namespace kin5ax;

namespace {

//a straight path of four locations with one constant, oblique tool axis: no singular section anywhere,
//and the empty lines put the cutter locations on the file lines 1, 3, 6 and 7
const size_t nLOCATIONS = 4;
const size_t vFILE_LINES[nLOCATIONS] = {1,3,6,7};

std::string WriteStraightPath(const std::string &sFilePath)
{
	FILE *pFile = fopen(sFilePath.c_str(),"wt");
	if(!pFile)
		return "cannot write "+sFilePath;
	const char *szAxis = "0.300000,0.400000,0.866025";
	fprintf(pFile,"GOTO/0.000000,0.000000,0.030000,%s\n",szAxis);
	fprintf(pFile,"\n");
	fprintf(pFile,"GOTO/0.025000,0.000000,0.030000,%s\n",szAxis);
	fprintf(pFile,"\n");
	fprintf(pFile,"\n");
	fprintf(pFile,"GOTO/0.050000,0.000000,0.030000,%s\n",szAxis);
	fprintf(pFile,"GOTO/0.075000,0.000000,0.030000,%s\n",szAxis);
	fclose(pFile);
	return "";
}

std::string ChainFile(const std::string &sNamePart)
{
	std::vector<std::string> v = GetChainFiles();
	for(size_t i=0;i<v.size();i++)
		if(v[i].find(sNamePart)!=std::string::npos)
			return v[i];
	return "";
}

//the four locations of WriteStraightPath postprocessed on a tilt-rotary table
CCLDataPostprocessingResult RunStraightPath(CSimulationData &oSimulationData, int nInterpolation = 0)
{
	std::string sFilePath = TempFilePath("kin5axTests_straight.cld");
	std::string sProblem = WriteStraightPath(sFilePath);
	EXPECT_EQ("",sProblem);
	std::string sChain = ChainFile("AC_TiltRotaryTable");
	EXPECT_NE("",sChain);
	CChain oChain = LoadChain(sChain);
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);	//the default tool of the application
	CCutterLocationData oData;
	oData.LoadFile(sFilePath,"straight");
	EXPECT_EQ(nLOCATIONS,oData.GetLocationsCount());
	EXPECT_EQ((size_t)0,oData.GetErrorsCount())<<"the empty lines are not errors";
	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;	//the records are the subject here, not the boxes of the model
	if(nInterpolation==1)
		oSettings.m_bInterpolateBetweenLocations = false;	//the mode
	if(nInterpolation==2){
		oSettings.m_dMaxPositionDifference = 0.0;			//the old way of saying it
		oSettings.m_dMaxOrientationDifferenceRadians = 0.0;
	}
	CCLDataPostprocessor oPostprocessor(oSettings);
	CCLDataPostprocessingResult oResult = oPostprocessor.Process(oChain,oTool,oData,oSimulationData,NULL,0,NULL);
	remove(sFilePath.c_str());
	return oResult;
}

} // namespace

//four locations 0.025 apart, an interpolation limit of 1E-2: three steps per stretch, and the first
//location is a stretch of its own (the file is read from its first location twice)
TEST(SolvedLocation, OneRecordPerSetAndThreeStepsPerStretch)
{
	CSimulationData oSimulationData;
	CCLDataPostprocessingResult oResult = RunStraightPath(oSimulationData);
	ASSERT_EQ((size_t)0,oResult.CountOf(LOCATION_SINGULAR_SECTION))
		<<"an oblique constant tool axis is meant to keep this path out of a singular section";
	EXPECT_EQ(oSimulationData.GetSize(),oResult.m_vLocations.size())<<"one record per solution set";
	EXPECT_EQ((size_t)10,oResult.m_vLocations.size())<<"1 + 3 x 3";
	EXPECT_EQ(nLOCATIONS,oResult.CountOf(LOCATION_FROM_FILE));
	EXPECT_EQ((size_t)6,oResult.CountOf(LOCATION_INTERPOLATED));
	EXPECT_EQ(oResult.m_vLocations.size()
		,oResult.CountOf(LOCATION_FROM_FILE)+oResult.CountOf(LOCATION_INTERPOLATED))
		<<"every record has one of the two origins";
}

//a location of the file carries its own number, everything invented on the way to it carries the number
//of the location before it: the steps between the 3rd and the 4th carry 3
TEST(SolvedLocation, TheFileLocationAndItsLineAreRecorded)
{
	CSimulationData oSimulationData;
	CCLDataPostprocessingResult oResult = RunStraightPath(oSimulationData);
	ASSERT_EQ((size_t)10,oResult.m_vLocations.size());
	const std::vector<CSolvedLocation> &v = oResult.m_vLocations;

	size_t nFileSeen = 0;
	for(size_t i=0;i<v.size();i++){
		if(v[i].m_nOrigin==LOCATION_FROM_FILE){
			nFileSeen++;
			EXPECT_EQ(nFileSeen,v[i].m_nFileLocation)<<"record "<<i<<" is cutter location "<<nFileSeen;
			EXPECT_EQ(vFILE_LINES[nFileSeen-1],v[i].m_nFileLine)<<"the line it was read from";
		} else {
			ASSERT_GT(nFileSeen,(size_t)0)<<"nothing is interpolated before the first location";
			EXPECT_EQ(nFileSeen,v[i].m_nFileLocation)<<"record "<<i<<" is on the way to "<<(nFileSeen+1);
			EXPECT_EQ(vFILE_LINES[nFileSeen-1],v[i].m_nFileLine);
		}
		if(i)
			EXPECT_LE(v[i-1].m_nFileLocation,v[i].m_nFileLocation)<<"the numbers never go back";
	}
	EXPECT_EQ(nLOCATIONS,nFileSeen);

	//the whole sequence, written out: location 1, then two steps towards 2 and location 2, and so on
	const size_t vExpected[10] = {1, 1,1,2, 2,2,3, 3,3,4};
	for(size_t i=0;i<10;i++)
		EXPECT_EQ(vExpected[i],v[i].m_nFileLocation)<<"record "<<i;
}

TEST(SolvedLocation, TheStatusesAddUpToTheRecords)
{
	CSimulationData oSimulationData;
	CCLDataPostprocessingResult oResult = RunStraightPath(oSimulationData);
	size_t nSum = 0;
	for(int s=0;s<SOLVE_STATUS_COUNT;s++)
		nSum += oResult.CountOf((SOLVE_STATUS)s);
	EXPECT_EQ(oResult.m_vLocations.size(),nSum)<<"every record has exactly one status";
	//the set of a solved record is not empty, and an unsolved one has nothing in it
	for(size_t i=0;i<oResult.m_vLocations.size();i++){
		if(oResult.m_vLocations[i].m_nStatus==SOLVE_STATUS_SOLVED){
			EXPECT_TRUE(oSimulationData.HasSolution(i))<<"record "<<i<<" says solved";
		} else {
			EXPECT_FALSE(oSimulationData.HasSolution(i))<<"record "<<i<<" says "
				<<SolveStatusName(oResult.m_vLocations[i].m_nStatus);
		}
	}
	RecordProperty("solved",(int)oResult.CountOf(SOLVE_STATUS_SOLVED));
}

TEST(SolvedLocation, TheNamesOfTheOrigins)
{
	EXPECT_STREQ("from the file",LocationOriginName(LOCATION_FROM_FILE));
	EXPECT_STREQ("interpolated",LocationOriginName(LOCATION_INTERPOLATED));
	EXPECT_STREQ("singular section",LocationOriginName(LOCATION_SINGULAR_SECTION));
	EXPECT_STREQ("unknown",LocationOriginName((LOCATION_ORIGIN)42));
	//the default record is a solved location of the file, so a default-constructed one says so
	CSolvedLocation o;
	EXPECT_EQ(SOLVE_STATUS_SOLVED,o.m_nStatus);
	EXPECT_EQ(LOCATION_FROM_FILE,o.m_nOrigin);
	EXPECT_EQ((size_t)0,o.m_nFileLocation);
	EXPECT_EQ((size_t)0,o.m_nFileLine);
}

// ---------------------------------------------------------------- KnownBugs #70

//A meridian over the north pole crosses the singular orientation of most machines, so the
//singular-section solver runs. It returns a set only for the locations it can use - it starts at the
//first one it can solve and stops at the last - and the sets it does not return used to be simply
//missing: fewer records and fewer solution sets than locations solved, with nothing saying which ones
//had gone. Every location solved keeps its record now, the dropped ones with an empty set.
TEST(SolvedLocation, ASingularSectionLeavesNoLocationWithoutARecord)
{
	std::vector<std::string> vChains = GetChainFiles();
	ASSERT_GE(vChains.size(),(size_t)1);
	std::string sFilePath = TempFilePath("kin5axTests_meridian_records.cld");
	const size_t nPoints = 9;
	ASSERT_EQ(nPoints,WriteMeridianCLData(sFilePath,0.05,nPoints));
	size_t nSingularTotal = 0;
	for(size_t c=0;c<vChains.size();c++){
		CChain oChain = LoadChain(vChains[c]);
		CMillingToolBall oTool(0.016,0.016,0.064,0.016);
		CCutterLocationData oData;
		oData.LoadFile(sFilePath,"meridian");
		ASSERT_EQ(nPoints,oData.GetLocationsCount());
		CPostprocessorSettings oSettings;
		oSettings.m_bCheckCollisions = false;
		CSimulationData oSimulationData;
		CCLDataPostprocessor oPostprocessor(oSettings);
		CCLDataPostprocessingResult oResult = oPostprocessor.Process(oChain,oTool,oData,oSimulationData,NULL,0,NULL);
		const std::string sWhere = GetTestName(vChains[c]);
		EXPECT_EQ(oResult.m_nTotalProblemsCount,oResult.m_vLocations.size())
			<<sWhere<<": every location solved has a record";
		EXPECT_EQ(oSimulationData.GetSize(),oResult.m_vLocations.size())
			<<sWhere<<": and one solution set, empty or not";
		//a location of the file is either a record of its own or one of the locations of a section
		EXPECT_GE(oResult.CountOf(LOCATION_FROM_FILE)+oResult.CountOf(LOCATION_SINGULAR_SECTION),nPoints)
			<<sWhere<<": the locations of the file are all accounted for";
		nSingularTotal += oResult.CountOf(LOCATION_SINGULAR_SECTION);
	}
	remove(sFilePath.c_str());
	EXPECT_GT(nSingularTotal,(size_t)0)<<"no machine of the test set went through a singular section:"
		" this test would not be about #70 any more";
}

//Why the padding is needed: SolveSingularOrientation hands back one set per location it can use, and it
//starts at the first location it can solve and stops at the last. Handed a section whose ends are out of
//reach, it returns fewer sets than it got inputs, with nothing saying which inputs they belong to. The
//caller must not assume one set per input - CCLDataPostprocessor::Process pads what is missing.
TEST(SolvedLocation, TheSingularSolverMayReturnFewerSetsThanItGotLocations)
{
	std::string sChain = ChainFile("AC_TiltRotaryTable");
	ASSERT_NE("",sChain);
	CChain oChain = LoadChain(sChain);
	CMillingToolBall oTool(0.016,0.016,0.064,0.016);
	CPostprocessorSettings oSettings;
	oSettings.m_bCheckCollisions = false;
	C5CPostprocessor oPostprocessor(oChain,oSettings);

	//the singular orientation of a tilt-rotary table: the tool axis along the axis of the table
	const CR3Vector r3vSingular(0.0,0.0,1.0);
	std::vector<CR3AnchoredVector> vSection;
	vSection.push_back(CR3AnchoredVector(r3vSingular,CR3Point(1E6,0.0,0.0)));	//nowhere near the machine
	vSection.push_back(CR3AnchoredVector(r3vSingular,CR3Point(0.00,0.0,0.02)));
	vSection.push_back(CR3AnchoredVector(r3vSingular,CR3Point(0.01,0.0,0.02)));
	vSection.push_back(CR3AnchoredVector(r3vSingular,CR3Point(0.02,0.0,0.02)));
	vSection.push_back(CR3AnchoredVector(r3vSingular,CR3Point(1E6,0.0,0.0)));

	std::string sLog;
	std::vector<tChainConfigurationVector> v = oPostprocessor.SolveSingularOrientation(vSection,oChain,&oTool,sLog);
	ASSERT_GT(v.size(),(size_t)0)<<"the reachable middle of the section is solvable:\n"<<sLog;
	EXPECT_LT(v.size(),vSection.size())<<"the unreachable ends got no set at all";
}

//with the interpolation switched off every cutter location of the file is solved as it stands and
//nothing is invented between two of them
TEST(SolvedLocation, WithoutInterpolationThereIsOneRecordPerCutterLocation)
{
	CSimulationData oSimulationData;
	CCLDataPostprocessingResult oResult = RunStraightPath(oSimulationData,1);
	EXPECT_EQ(nLOCATIONS,oResult.m_vLocations.size());
	EXPECT_EQ(nLOCATIONS,oResult.CountOf(LOCATION_FROM_FILE));
	EXPECT_EQ((size_t)0,oResult.CountOf(LOCATION_INTERPOLATED));
	EXPECT_EQ(nLOCATIONS,oResult.m_nTotalProblemsCount);
	EXPECT_EQ(nLOCATIONS,oSimulationData.GetSize());
	for(size_t i=0;i<oResult.m_vLocations.size();i++){
		EXPECT_EQ(i+1,oResult.m_vLocations[i].m_nFileLocation);
		EXPECT_EQ(vFILE_LINES[i],oResult.m_vLocations[i].m_nFileLine);
	}
}

//the two limits at zero did the same before there was a flag for it, and still do
TEST(SolvedLocation, TheLimitsAtZeroDoWhatTheModeDoes)
{
	CSimulationData oWithFlag, oWithLimits;
	CCLDataPostprocessingResult oFlag = RunStraightPath(oWithFlag,1);
	CCLDataPostprocessingResult oLimits = RunStraightPath(oWithLimits,2);
	ASSERT_EQ(oFlag.m_vLocations.size(),oLimits.m_vLocations.size());
	EXPECT_EQ(oWithFlag.GetSize(),oWithLimits.GetSize());
	for(size_t i=0;i<oFlag.m_vLocations.size();i++){
		EXPECT_EQ(oFlag.m_vLocations[i].m_nOrigin,oLimits.m_vLocations[i].m_nOrigin)<<"record "<<i;
		EXPECT_EQ(oFlag.m_vLocations[i].m_nFileLocation,oLimits.m_vLocations[i].m_nFileLocation)<<"record "<<i;
		EXPECT_EQ(oFlag.m_vLocations[i].m_nStatus,oLimits.m_vLocations[i].m_nStatus)<<"record "<<i;
	}
}
