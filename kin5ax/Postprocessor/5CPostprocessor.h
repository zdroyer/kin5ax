#pragma once
#include "Chain.h"
#include "OrientationPostprocessor.h"
#include "PositionPostprocessor.h"
#include "PostprocessorSettings.h"
#include "MillingTool.h"
#include "SolveReport.h"


namespace kin5ax {

typedef std::pair<CChainConfiguration,CR3AnchoredVector> tChainConfigurationWithResult;


class C5CPostprocessor
{
protected:
	
public:
	C5CPostprocessor(const CChain &oChain, const CPostprocessorSettings &oSettings);
	~C5CPostprocessor(void);
	std::vector<tChainConfigurationWithResult> SolveWithLocations(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,CMillingTool &oMillingTool,std::string &sLog);
	//all configurations that reach the cutter location (reference point r3pPosition, tool axis r3vOrientation),
	//are within the joint limits (when the settings check them) and do not collide (when the settings check
	//collisions; the tool may then be NULL, otherwise a NULL tool throws). sLog tells what was found and
	//rejected in words, oReport in numbers (and SOLVE_STATUS Status() why the result is empty)
	tChainConfigurationVector Solve(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,CMillingTool &oMillingTool,std::string &sLog);
	tChainConfigurationVector Solve(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,CMillingTool &oMillingTool,std::string &sLog, CSolveReport &oReport);
	tChainConfigurationVector Solve(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,CMillingTool *pMillingTool,std::string &sLog, CSolveReport &oReport);
	tChainConfigurationVector SolveWithKnownAlpha1(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,std::string &sLog,std::vector<double> vAlpha1,CMillingTool &oMillingTool);
	tChainConfigurationVector SolveWithKnownAlpha1(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,std::string &sLog,std::vector<double> vAlpha1,CMillingTool *pMillingTool);

	//the cutter locations of a section where the orientation is singular (alpha1 free): alpha1 is interpolated
	//between the solutions before and after the section. One solution set per solved location; none when no
	//location of the section can be solved. The tool as in Solve.
	std::vector<tChainConfigurationVector> SolveSingularOrientation(const std::vector<CR3AnchoredVector> &vInput, CChain &oChain,CMillingTool &oMillingTool,std::string &sLog);
	std::vector<tChainConfigurationVector> SolveSingularOrientation(const std::vector<CR3AnchoredVector> &vInput, CChain &oChain,CMillingTool *pMillingTool,std::string &sLog);

protected:
	//the configurations before the limits and the collisions; pReport (may be NULL) receives the counts
	tChainConfigurationVector SolveTheoretically(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,std::string &sLog, CSolveReport *pReport = NULL);
	tChainConfigurationVector ApplyParameterConstraints(const tChainConfigurationVector &vSolutions, CChain &oChain,std::string &sLog);
	tChainConfigurationVector ApplyCollisionDetection(const tChainConfigurationVector &vSolutions, CChain &oChain,CMillingTool &oMillingTool,std::string &sLog);


	void CreateSigns(const CChain &oChain);
	void CreateOrientationPostprocessor(const CChain &oChain);
	void CreatePositionPostprocessor(const CChain &oChain);

	//Copy of the settings, taken when the postprocessor is created: the postprocessor does not depend
	//on any global object and later changes of the settings do not affect an existing postprocessor
	//(create a new one to apply them).
	CPostprocessorSettings m_oSettings;

	COrientationPostprocessor m_oOrientationPostprocessor;
	CPositionPostprocessor m_oPositionPostprocessor;
	std::vector<double> m_vSigns;
	size_t m_nFirstRotationIndex,m_nSecondRotationIndex;
	size_t m_nFirstTranslationIndex,m_nSecondTranslationIndex, m_nThirdTranslationIndex;
	CTransformation m_m4x4InitialOrientationTransformation;
	CTransformation m_m4x4InitialPositionTransformation;
	


};

} // namespace kin5ax
