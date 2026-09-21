#pragma once
#include "../Chain/Chain.h"
#include "orientationpostprocessor.h"
#include "positionpostprocessor.h"
#include "../millingtool.h"


typedef pair<CChainConfiguration,CR3CaughtVector> tChainConfigurationWithResult;


class C5CPostprocessor
{
protected:
	
public:
	C5CPostprocessor(const CChain &oChain);
	~C5CPostprocessor(void);
	vector<tChainConfigurationWithResult> SolveWithLocations(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,CMillingTool &oMillingTool,string &sLog);
	tChainConfigurationVector Solve(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,CMillingTool &oMillingTool,string &sLog);
	tChainConfigurationVector SolveWithKnownAlpha1(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,string &sLog,vector<double> vAlpha1,CMillingTool &oMillingTool);

	vector<tChainConfigurationVector> SolveSingularOrientation(const vector<CR3CaughtVector> &vInput, CChain &oChain,CMillingTool &oMillingTool,string &sLog);

protected:
	tChainConfigurationVector SolveTheoretically(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,string &sLog);
	tChainConfigurationVector ApplyParameterConstraints(const tChainConfigurationVector &vSolutions, CChain &oChain,string &sLog);
	tChainConfigurationVector ApplyCollisionDetection(const tChainConfigurationVector &vSolutions, CChain &oChain,CMillingTool &oMillingTool,string &sLog);


	void CreateSigns(const CChain &oChain);
	void CreateOrientationPostprocessor(const CChain &oChain);
	void CreatePositionPostprocessor(const CChain &oChain);

	COrientationPostprocessor m_oOrientationPostprocessor;
	CPositionPostprocessor m_oPositionPostprocessor;
	vector<double> m_vSigns;
	size_t m_nFirstRotationIndex,m_nSecondRotationIndex;
	size_t m_nFirstTranslationIndex,m_nSecondTranslationIndex, m_nThirdTranslationIndex;
	CTransformation m_m4x4InitialOrientationTransformation;
	CTransformation m_m4x4InitialPositionTransformation;
	


};
