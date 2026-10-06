#pragma once

#include "ChainConfiguration.h"

//How the first configuration of a trajectory is chosen. Everything after it follows by the shortest
//path, but the start itself is a convention of the machine and its control - the TWP/TCP functions of
//Fanuc, Heidenhain and Siemens each name one - so it is said here instead of being left to whichever
//candidate the solver happened to produce first.
namespace kin5ax {

enum TRAJECTORY_START_CRITERION {
	//The start is not fixed: whichever candidate makes the whole trajectory shortest begins it. What the
	//library did before this policy existed, and the right answer when no control imposes a start.
	TRAJECTORY_START_SHORTEST_PATH = 0,
	TRAJECTORY_START_MIN,			//the smallest value of the axis the policy looks at
	TRAJECTORY_START_MAX,			//the largest value of it
	TRAJECTORY_START_MIN_ABS,		//the value closest to zero: the usual convention of the controls
	TRAJECTORY_START_RANDOM,		//a candidate drawn with m_nRandomSeed, when nothing can predict the start
	TRAJECTORY_START_CRITERION_COUNT
};

//a short name for reports: "shortest path", "smallest", "largest", "closest to zero", "random"
const char *TrajectoryStartCriterionName(TRAJECTORY_START_CRITERION nCriterion);

class CTrajectoryStart
{
public:
	//the start is not fixed and there is no hint: the behaviour of the library before this policy
	//existed. CChain::GetTrajectoryStart gives the recommended one instead.
	CTrajectoryStart();

	//no parameter to apply the criterion to
	static const size_t nNO_PARAMETER;
	bool IsResolved() const { return m_nParameterIndex!=nNO_PARAMETER; }

	//True when the criterion fixes the start, so that one candidate of the first set and no other may
	//begin the trajectory. False for TRAJECTORY_START_SHORTEST_PATH, and for a criterion with no axis to
	//apply it to.
	bool FixesTheStart() const;

	//Which candidate of a solution set the trajectory starts from. The hint, when there is one, keeps
	//the candidates nearest to it, and the criterion decides between those that tie - which it has to,
	//because a symmetric hint such as all zeros ties by nature. Equal under the criterion as well: the
	//first of them. An empty set gives 0.
	size_t ChooseIndex(const tChainConfigurationVector &vCandidates, const CPostprocessorSettings &oSettings) const;

	//What beginning the trajectory at this candidate costs before anything is cut: the distance from the
	//configuration the trajectory continues from, or nothing at all when there is no hint. The shortest
	//path is then measured from where the machine actually stands.
	double StartCost(const CChainConfiguration &oCandidate, const CPostprocessorSettings &oSettings) const;

	//What this policy does, in one line for a report. The random start says so, because a random start
	//is nobody's convention and whoever reads the G-code has to know.
	std::string Describe() const;

	TRAJECTORY_START_CRITERION m_nCriterion;
	//The parameter of the configuration the criterion is applied to: the first rotary axis counted from
	//one end of the chain, which CChain::GetTrajectoryStart fills in. nNO_PARAMETER means there is
	//nothing to apply the criterion to, and then the first candidate is taken whatever m_nCriterion says.
	size_t m_nParameterIndex;
	//Optional: the configuration this trajectory continues from - the end of the previous file, the
	//position after a tool change, or all zeros for a machine parked at its origin. The parameter count
	//has to match the candidates (CChainConfiguration::Distance throws otherwise) and an ambiguous hint
	//is refused the way an ambiguous candidate is.
	const CChainConfiguration *m_pHint;
	unsigned int m_nRandomSeed;		//TRAJECTORY_START_RANDOM only, so that a run can be repeated
};

} // namespace kin5ax
