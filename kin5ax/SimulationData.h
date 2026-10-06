#pragma once

#include "ChainConfiguration.h"
#include "ProgressCallback.h"
#include "TrajectoryStart.h"


namespace kin5ax {

class CSimulationData
{

public:
	CSimulationData(size_t n=10000);
	~CSimulationData(void);
	int ExportToGCode(	FILE *pFile,
						size_t nXIndex, 
						size_t nYIndex,
						size_t nZIndex,
						char cAngle1Char,
						size_t nAngle1Index,
						char cAngle2Char,
						size_t nAngle2Index,
						bool bRadians) const;
	const CChainConfiguration &operator[](size_t n) const;
	size_t GetSize() const;
	//true when the location has at least one solution (operator[] throws for the others)
	bool HasSolution(size_t n) const;
	void ReserveMemory(size_t n);
	void AddConfigurationSet(const tChainConfigurationVector &vConfigurations);
	//oSettings: the weights of the distance between two configurations; pProgress may be NULL.
	//oStart: which candidate the trajectory starts from - the default takes the first candidate of the
	//first set that has one, which is what this did before the policy existed; CChain::GetTrajectoryStart
	//gives one resolved against a machine.
	//Returns false when pProgress->Continue() stopped the search: every index is 0 then, as before the search
	bool FindShortestPath(const CPostprocessorSettings &oSettings, CProgressCallback *pProgress
		,const CTrajectoryStart &oStart = CTrajectoryStart());
	//The configuration chosen for the last set that has one: where the machine is left standing, and what
	//the next file or the run after a tool change continues from (CTrajectoryStart::m_pHint). False when
	//no set has a solution. Before FindShortestPath it gives the first candidate of that set.
	bool GetLastChosenConfiguration(CChainConfiguration &oConfiguration) const;
	void DeleteContents();
	bool NextConfiguration(CChainConfiguration &oChainConfiguration);
	void ResetConfigurationsIterator();
protected:
	std::vector<tChainConfigurationVector> m_vSolutions; 
	std::vector<size_t> m_vIndexes;
	size_t m_nEmptySolutionsCount;
	size_t m_nActualSolution;
public:
	tChainConfigurationVector GetLastNonemptyConfigurationSet() const;
	size_t GetEmptySolutionsCount(void);
};

} // namespace kin5ax
