#pragma once

#include "ChainConfiguration.h"
#include "ProgressCallback.h"


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
	void ReserveMemory(size_t n);
	void AddConfigurationSet(const tChainConfigurationVector &vConfigurations);
	//oSettings: the weights of the distance between two configurations; pProgress may be NULL
	void FindShortestPath(const CPostprocessorSettings &oSettings, CProgressCallback *pProgress);
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
