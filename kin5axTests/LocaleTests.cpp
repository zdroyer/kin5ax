// The library must not depend on the numeric locale of the process: a host (FreeCAD, a Python
// interpreter, a German or Polish desktop) may run with the comma as the decimal separator, the files
// of kin5ax use the point. Every test here switches the process to such a locale for its duration.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "TestData.h"
#include "CutterLocationData.h"
#include "SimulationData.h"
#include "ChainJson.h"
#include "string_tools.h"
#include <clocale>

using namespace kin5ax;

namespace {

//a locale with the comma as decimal separator for the rest of the scope; Set() is false when none is installed
class CCommaLocale
{
public:
	CCommaLocale() : m_bSet(false)
	{
		const char *szOld = setlocale(LC_ALL,NULL);
		m_sOld = szOld ? szOld : "C";
		const char *szNames[] = {"de-DE","German_Germany","pl-PL","Polish_Poland","de_DE.UTF-8","pl_PL.UTF-8","de_DE","pl_PL"};
		for(const char *szName : szNames)
			if(setlocale(LC_ALL,szName)){
				m_bSet = true;
				break;
			}
		if(m_bSet){
			char szHalf[32];
			snprintf(szHalf,sizeof(szHalf),"%.1f",0.5);
			m_bSet = std::string(szHalf)=="0,5";	//the locale really uses the comma
		}
	}
	~CCommaLocale() { setlocale(LC_ALL,m_sOld.c_str()); }
	bool Set() const { return m_bSet; }
private:
	std::string m_sOld;
	bool m_bSet;
};

#define REQUIRE_COMMA_LOCALE(oLocale) if(!oLocale.Set()){ RecordProperty("skipped","no comma locale is installed"); return; }

std::string ReadWholeFile(const std::string &sFilePath)
{
	FILE *pFile = fopen(sFilePath.c_str(),"rb");
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

void WriteText(const std::string &sFilePath, const std::string &sText)
{
	FILE *pFile = fopen(sFilePath.c_str(),"wb");
	ASSERT_TRUE(pFile!=NULL)<<sFilePath;
	fwrite(sText.data(),1,sText.size(),pFile);
	fclose(pFile);
}

} // namespace

TEST(Locale, CLDataIsReadWithThePoint)
{
	CCommaLocale oLocale;
	REQUIRE_COMMA_LOCALE(oLocale);
	std::string sFilePath = TempFilePath("kin5axTests_locale.cld");
	WriteText(sFilePath,"GOTO/0.5,1.5,2.5,0,0,1\nGOTO/1e-3,-2.25,0.125,0,0.6,0.8\n");
	CCutterLocationData oData;
	try{
		oData.LoadFile(sFilePath,"locale");
	} catch (CGeneratorException &e){
		FAIL()<<e.Message();
	}
	remove(sFilePath.c_str());
	ASSERT_EQ((size_t)2,oData.GetLocationsCount());
	EXPECT_EQ((size_t)0,oData.GetErrorsCount());
	CR3Point p;
	CR3Vector v;
	oData.ResetLocationIterator();
	ASSERT_TRUE(oData.GetNextLocation(p,v));
	EXPECT_DOUBLE_EQ(0.5,p.GetX());
	EXPECT_DOUBLE_EQ(1.5,p.GetY());
	EXPECT_DOUBLE_EQ(2.5,p.GetZ());
	ASSERT_TRUE(oData.GetNextLocation(p,v));
	EXPECT_DOUBLE_EQ(0.001,p.GetX());
	EXPECT_DOUBLE_EQ(-2.25,p.GetY());
	EXPECT_DOUBLE_EQ(0.6,v.GetY());
}

TEST(Locale, NumbersAreWrittenWithThePoint)
{
	CCommaLocale oLocale;
	REQUIRE_COMMA_LOCALE(oLocale);
	EXPECT_EQ("0.5",double_to_string(0.5));
	EXPECT_EQ("1.2345679E+20",double_to_string(1.23456789E20));
	EXPECT_EQ("2.5",double_to_string_exact(2.5));
	EXPECT_EQ("0.500000",fixed_to_string(0.5,6));
	EXPECT_EQ("-12.25",fixed_to_string(-12.25,2));
}

TEST(Locale, GCodeIsWrittenWithThePoint)
{
	CCommaLocale oLocale;
	REQUIRE_COMMA_LOCALE(oLocale);
	CSimulationData oData(10);
	CChainConfiguration oConfig(5);
	oConfig[0].SetParameter(0.5,DISTANCE_UNIT);
	oConfig[1].SetParameter(1.5,DISTANCE_UNIT);
	oConfig[2].SetParameter(2.5,DISTANCE_UNIT);
	oConfig[3].SetParameter(0.25,RADIAN);
	oConfig[4].SetParameter(0.75,RADIAN);
	tChainConfigurationVector v;
	v.push_back(oConfig);
	oData.AddConfigurationSet(v);
	std::string sFilePath = TempFilePath("kin5axTests_locale.nc");
	FILE *pFile = fopen(sFilePath.c_str(),"wb");
	ASSERT_TRUE(pFile!=NULL);
	EXPECT_EQ(0,oData.ExportToGCode(pFile,0,1,2,'A',3,'C',4,true));
	fclose(pFile);
	std::string sText = ReadWholeFile(sFilePath);
	remove(sFilePath.c_str());
	EXPECT_EQ("N10 G1 X0.500000 Y1.500000 Z2.500000 A0.250000 C0.750000\n",sText);
}

TEST(Locale, ChainFilesReadAndWriteTheSameInAnyLocale)
{
	ASSERT_GE(GetChainFiles().size(),(size_t)1);
	std::string sFilePath = GetChainFiles()[0];
	std::string sJson;
	try{
		sJson = CChainJsonWriter::ToJson(CChainJsonReader::ReadFile(sFilePath));
	} catch (CGeneratorException &e){
		FAIL()<<e.Message();
	}
	CCommaLocale oLocale;
	REQUIRE_COMMA_LOCALE(oLocale);
	try{
		EXPECT_EQ(sJson,CChainJsonWriter::ToJson(CChainJsonReader::ReadFile(sFilePath)));
		EXPECT_EQ(sJson,CChainJsonWriter::ToJson(CChainJsonReader::Parse(sJson,"text")));
	} catch (CGeneratorException &e){
		FAIL()<<e.Message();
	}
}
