// CSimulationData::FindShortestPath against a brute force search on small synthetic solution sets.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "SimulationData.h"

//one configuration with a single linear parameter
static CChainConfiguration Config(double dValue)
{
	CChainConfiguration oConfig(1);
	oConfig[0].SetParameter(dValue,DISTANCE_UNIT);
	return oConfig;
}

static tChainConfigurationVector Set(double d1)
{
	tChainConfigurationVector v;
	v.push_back(Config(d1));
	return v;
}

static tChainConfigurationVector Set(double d1, double d2)
{
	tChainConfigurationVector v = Set(d1);
	v.push_back(Config(d2));
	return v;
}

static tChainConfigurationVector EmptySet()
{
	return tChainConfigurationVector();
}

//length of the cheapest path through the non-empty sets, by trying every combination
static double BruteForceLength(const vector<tChainConfigurationVector> &vSets, const CPostprocessorSettings &oSettings)
{
	vector<tChainConfigurationVector> vNonEmpty;
	for(size_t i=0;i<vSets.size();i++)
		if(vSets[i].size())
			vNonEmpty.push_back(vSets[i]);
	if(vNonEmpty.size()<2)
		return 0.0;
	double dBest = -1.0;
	vector<size_t> vIndexes(vNonEmpty.size(),0);
	while(true){
		double d = 0.0;
		for(size_t i=1;i<vNonEmpty.size();i++)
			d += vNonEmpty[i-1][vIndexes[i-1]].Distance(vNonEmpty[i][vIndexes[i]],oSettings);
		if(dBest<0.0 || d<dBest)
			dBest = d;
		//next combination
		size_t k=0;
		while(k<vIndexes.size() && ++vIndexes[k]==vNonEmpty[k].size()){
			vIndexes[k]=0;
			k++;
		}
		if(k==vIndexes.size())
			break;
	}
	return dBest;
}

//length of the path chosen by FindShortestPath; every empty set must throw, every other must not
static double ChosenLength(const vector<tChainConfigurationVector> &vSets, const CPostprocessorSettings &oSettings)
{
	CSimulationData oData(10);
	for(size_t i=0;i<vSets.size();i++)
		oData.AddConfigurationSet(vSets[i]);
	oData.FindShortestPath(oSettings,NULL);

	double d = 0.0;
	bool bHasPrevious = false;
	CChainConfiguration oPrevious;
	for(size_t i=0;i<vSets.size();i++){
		if(vSets[i].empty()){
			EXPECT_THROW(oData[i],CGeneratorException)<<"set "<<i;
			continue;
		}
		CChainConfiguration oActual = oData[i];
		if(bHasPrevious)
			d += oPrevious.Distance(oActual,oSettings);
		oPrevious = oActual;
		bHasPrevious = true;
	}
	return d;
}

static void ExpectShortest(const vector<tChainConfigurationVector> &vSets)
{
	CPostprocessorSettings oSettings;
	EXPECT_NEAR(BruteForceLength(vSets,oSettings),ChosenLength(vSets,oSettings),1E-12);
}

TEST(FindShortestPath, PicksTheSecondSolutionOfTheFirstSet)
{
	vector<tChainConfigurationVector> v;
	v.push_back(Set(5.0,0.0));	//the path 0,0,0,0 starts with the second solution
	v.push_back(Set(0.1,4.0));
	v.push_back(Set(0.2,4.1));
	v.push_back(Set(0.0,9.0));
	ExpectShortest(v);
}

TEST(FindShortestPath, LastSetsAreEmpty)
{
	vector<tChainConfigurationVector> v;
	v.push_back(Set(0.0,5.0));
	v.push_back(Set(5.1,0.1));
	v.push_back(Set(0.2,5.2));
	v.push_back(EmptySet());
	v.push_back(EmptySet());
	ExpectShortest(v);
}

TEST(FindShortestPath, FirstSetsAreEmpty)
{
	vector<tChainConfigurationVector> v;
	v.push_back(EmptySet());
	v.push_back(EmptySet());
	v.push_back(Set(3.0,0.0));
	v.push_back(Set(0.1,3.1));
	v.push_back(Set(3.2,0.2));
	ExpectShortest(v);
}

TEST(FindShortestPath, EmptySetsInTheMiddle)
{
	vector<tChainConfigurationVector> v;
	v.push_back(Set(2.0,0.0));
	v.push_back(EmptySet());
	v.push_back(Set(0.1,2.1));
	v.push_back(EmptySet());
	v.push_back(EmptySet());
	v.push_back(Set(2.2,0.2));
	ExpectShortest(v);
}

TEST(FindShortestPath, FirstSetBiggerThanTheSecond)
{
	vector<tChainConfigurationVector> v;
	v.push_back(Set(1.0,0.0));
	v.push_back(Set(0.1));
	v.push_back(Set(0.2,1.2));
	v.push_back(Set(1.3,0.3));
	ExpectShortest(v);
}

TEST(FindShortestPath, FirstSetSmallerThanTheSecond)
{
	vector<tChainConfigurationVector> v;
	v.push_back(Set(0.0));
	v.push_back(Set(1.1,0.1));
	v.push_back(Set(0.2,1.2));
	v.push_back(Set(1.3,0.3));
	ExpectShortest(v);
}

TEST(FindShortestPath, OnlyOneSetHasSolutions)
{
	vector<tChainConfigurationVector> v;
	v.push_back(EmptySet());
	v.push_back(Set(1.0,0.0));
	v.push_back(EmptySet());
	v.push_back(EmptySet());
	ExpectShortest(v);
}

TEST(FindShortestPath, NoSetHasSolutions)
{
	vector<tChainConfigurationVector> v(4,EmptySet());
	ExpectShortest(v);
}
