// CCutterLocationData::SetLocations / AddLocation: a tool path handed over instead of read from a file,
// for a host that generates one and has nowhere to put it. What is read back, and what is postprocessed
// out of it, has to be the same as if the same locations had gone through a file.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "CutterLocationData.h"
#include "CLDataPostprocessor.h"
#include "MillingToolBall.h"
#include "ArgsMyException.h"
#include <cmath>

using namespace kin5ax;

namespace {

const size_t nCOUNT = 4;
const double dSTEP = 0.025;

//four locations on a straight line with one oblique tool axis, the same ones WriteThem writes
std::vector<CR3AnchoredVector> TheLocations()
{
	std::vector<CR3AnchoredVector> v;
	const CR3Vector r3vAxis(0.3,0.4,0.866025);
	for(size_t i=0;i<nCOUNT;i++)
		v.push_back(CR3AnchoredVector(r3vAxis,CR3Point(0.1+(double)i*dSTEP,0.2,0.03)));
	return v;
}

std::string WriteThem(const std::string &sFilePath)
{
	const std::vector<CR3AnchoredVector> v = TheLocations();
	FILE *pFile = fopen(sFilePath.c_str(),"wt");
	if(!pFile)
		return "cannot write "+sFilePath;
	for(size_t i=0;i<v.size();i++)
		fprintf(pFile,"GOTO/%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n"
			,v[i].m_p.GetX(),v[i].m_p.GetY(),v[i].m_p.GetZ()
			,v[i].m_v.GetX(),v[i].m_v.GetY(),v[i].m_v.GetZ());
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

} // namespace

TEST(CLDataInMemory, TheLocationsComeBackAsTheyWereGiven)
{
	const std::vector<CR3AnchoredVector> v = TheLocations();
	CCutterLocationData oData;
	oData.SetLocations(v,"a path of four locations");
	EXPECT_EQ(nCOUNT,oData.GetLocationsCount());
	EXPECT_EQ(nCOUNT,oData.GetLinesCount())<<"one line per location and nothing else";
	EXPECT_EQ((size_t)0,oData.GetErrorsCount());
	EXPECT_EQ("a path of four locations",oData.GetFileName());
	EXPECT_EQ("a path of four locations",oData.GetFileTitle());

	//the iterator, and the line numbers: the place of the location in the vector, counted from 1
	oData.ResetLocationIterator();
	CR3Point p;
	CR3Vector r3v;
	size_t nLine = 0;
	for(size_t i=0;i<nCOUNT;i++){
		ASSERT_TRUE(oData.GetNextLocation(p,r3v,nLine))<<"location "<<i;
		EXPECT_NEAR(0.0,d_E(v[i].m_p,p),1E-12)<<"location "<<i;
		//as a direction: the axis is stored as a unit vector, the way a file is read
		EXPECT_NEAR(0.0,(v[i].m_v.GetUnitVector()-r3v).GetLength(),1E-12)<<"location "<<i;
		EXPECT_EQ(i+1,nLine);
	}
	EXPECT_FALSE(oData.GetNextLocation(p,r3v,nLine))<<"and no more";

	//GetLocation reaches them by index too
	ASSERT_TRUE(oData.GetLocation(p,r3v,2));
	EXPECT_NEAR(0.0,d_E(v[2].m_p,p),1E-12);
	//there is no text behind a line that never was one
	EXPECT_THROW(oData.GetLineText(0),CArgsMyException);
}

TEST(CLDataInMemory, TheStatisticsAreTheSameAsFromAFile)
{
	const std::vector<CR3AnchoredVector> v = TheLocations();
	CCutterLocationData oMemory;
	oMemory.SetLocations(v);

	const std::string sFilePath = TempFilePath("kin5axTests_inmemory.cld");
	ASSERT_EQ("",WriteThem(sFilePath));
	CCutterLocationData oFile;
	oFile.LoadFile(sFilePath,"from a file");

	EXPECT_EQ(oFile.GetLocationsCount(),oMemory.GetLocationsCount());
	EXPECT_NEAR(oFile.GetTrajectoryLength(),oMemory.GetTrajectoryLength(),1E-9);
	EXPECT_NEAR(oFile.GetTotalOrientationChangeRadians(),oMemory.GetTotalOrientationChangeRadians(),1E-9);
	for(int nTransformed=0;nTransformed<2;nTransformed++){
		const CR3Box oFromFile = oFile.GetBoundingBox(nTransformed!=0);
		const CR3Box oFromMemory = oMemory.GetBoundingBox(nTransformed!=0);
		//the file carries six decimals, so the two boxes are the same box to within that
		EXPECT_NEAR(0.0,d_E(oFromFile.GetMinPoint(),oFromMemory.GetMinPoint()),1E-6)<<"transformed: "<<nTransformed;
		EXPECT_NEAR(0.0,d_E(oFromFile.GetMaxPoint(),oFromMemory.GetMaxPoint()),1E-6)<<"transformed: "<<nTransformed;
	}
	//the path is a straight line of three steps, and the axis never turns
	EXPECT_NEAR(3.0*dSTEP,oMemory.GetTrajectoryLength(),1E-9);
	EXPECT_NEAR(0.0,oMemory.GetTotalOrientationChangeRadians(),1E-9);
	remove(sFilePath.c_str());
}

TEST(CLDataInMemory, OneLocationAtATime)
{
	const std::vector<CR3AnchoredVector> v = TheLocations();
	CCutterLocationData oData;
	for(size_t i=0;i<v.size();i++)
		oData.AddLocation(v[i].m_p,v[i].m_v);
	EXPECT_EQ(nCOUNT,oData.GetLocationsCount());
	EXPECT_NEAR(3.0*dSTEP,oData.GetTrajectoryLength(),1E-9);

	//a second path over the same object replaces the first one and forgets its statistics
	oData.SetLocations(std::vector<CR3AnchoredVector>(v.begin(),v.begin()+2));
	EXPECT_EQ((size_t)2,oData.GetLocationsCount());
	EXPECT_EQ((size_t)2,oData.GetLinesCount());
	EXPECT_NEAR(dSTEP,oData.GetTrajectoryLength(),1E-9);

	//and nothing at all is a path of no locations, not an error
	oData.SetLocations(std::vector<CR3AnchoredVector>());
	EXPECT_EQ((size_t)0,oData.GetLocationsCount());
	EXPECT_EQ((size_t)0,oData.GetLinesCount());
	EXPECT_FALSE(oData.HasMoreLocations());
	EXPECT_NEAR(0.0,oData.GetTrajectoryLength(),1E-12);
}

//the point of the whole thing: postprocessing a path handed over has to give what postprocessing the
//same path out of a file gives, record for record
TEST(CLDataInMemory, PostprocessingGivesTheSameAsFromAFile)
{
	const std::string sChain = ChainFile("AC_TiltRotaryTable");
	ASSERT_NE("",sChain);
	const std::string sFilePath = TempFilePath("kin5axTests_inmemory_post.cld");
	ASSERT_EQ("",WriteThem(sFilePath));

	CCLDataPostprocessingResult vResults[2];
	CSimulationData vData[2];
	for(int nRun=0;nRun<2;nRun++){
		CChain oChain = LoadChain(sChain);
		CMillingToolBall oTool(0.016,0.016,0.064,0.016,0.030,CToolHolder(0.050,0.030));
		CCutterLocationData oLocations;
		if(nRun==0)
			oLocations.LoadFile(sFilePath,"from a file");
		else
			oLocations.SetLocations(TheLocations(),"handed over");
		ASSERT_EQ(nCOUNT,oLocations.GetLocationsCount())<<"run "<<nRun;
		CPostprocessorSettings oSettings;
		CCLDataPostprocessor oPostprocessor(oSettings);
		vResults[nRun] = oPostprocessor.Process(oChain,oTool,oLocations,vData[nRun],NULL,0,NULL);
		oChain.SetParameters(CChainConfiguration((int)oChain.GetNumberOfRegularLinks()));
	}

	ASSERT_EQ(vResults[0].m_vLocations.size(),vResults[1].m_vLocations.size());
	EXPECT_EQ(vData[0].GetSize(),vData[1].GetSize());
	EXPECT_GT(vResults[0].m_vLocations.size(),nCOUNT)<<"the interpolation did its work in both runs";
	for(size_t i=0;i<vResults[0].m_vLocations.size();i++){
		const CSolvedLocation &a = vResults[0].m_vLocations[i];
		const CSolvedLocation &b = vResults[1].m_vLocations[i];
		EXPECT_EQ(a.m_nStatus,b.m_nStatus)<<"record "<<i;
		EXPECT_EQ(a.m_nOrigin,b.m_nOrigin)<<"record "<<i;
		EXPECT_EQ(a.m_nFileLocation,b.m_nFileLocation)<<"record "<<i;
		EXPECT_EQ(a.m_nFileLine,b.m_nFileLine)<<"record "<<i<<": the line numbers agree, because the"
			" locations handed over are numbered the way the lines of that file are";
		if(vData[0].HasSolution(i) && vData[1].HasSolution(i)){
			for(size_t k=0;k<vData[0][i].GetParametersCount();k++)
				EXPECT_NEAR(vData[0][i][k].GetValue(),vData[1][i][k].GetValue(),1E-12)
					<<"record "<<i<<", axis "<<k;
		} else {
			EXPECT_EQ(vData[0].HasSolution(i),vData[1].HasSolution(i))<<"record "<<i;
		}
	}
	remove(sFilePath.c_str());
}

//A location of a file carries a unit vector, because the reader makes one of whatever the line says.
//A location handed over has to be stored the same way, or the solver would see a different path.
TEST(CLDataInMemory, TheAxisIsStoredAsAUnitVector)
{
	CCutterLocationData oData;
	oData.AddLocation(CR3Point(0.1,0.2,0.03),CR3Vector(0.0,0.0,2.5));
	oData.AddLocation(CR3Point(0.2,0.2,0.03),CR3Vector(0.3,0.4,0.866025));	//rounded, so not quite unit
	oData.ResetLocationIterator();
	CR3Point p;
	CR3Vector r3v;
	ASSERT_TRUE(oData.GetNextLocation(p,r3v));
	EXPECT_NEAR(1.0,r3v.GetLength(),1E-15)<<"a long axis comes back as a direction";
	EXPECT_NEAR(1.0,r3v.GetZ(),1E-15);
	ASSERT_TRUE(oData.GetNextLocation(p,r3v));
	EXPECT_NEAR(1.0,r3v.GetLength(),1E-15)<<"and so does one that is a millionth off";

	//what the reader of a file calls an error is an argument error here
	EXPECT_THROW(oData.AddLocation(CR3Point(0.0),CR3Vector(0.0,0.0,0.0)),CArgsMyException);
	EXPECT_EQ((size_t)2,oData.GetLocationsCount())<<"and nothing was added";
}
