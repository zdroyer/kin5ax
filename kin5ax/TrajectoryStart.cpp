#include "stdafx.h"
#include "TrajectoryStart.h"
#include "ArgsMyException.h"
#include <random>

namespace kin5ax {

const size_t CTrajectoryStart::nNO_PARAMETER = (size_t)-1;

const char *TrajectoryStartCriterionName(TRAJECTORY_START_CRITERION nCriterion)
{
	static const char *szNames[TRAJECTORY_START_CRITERION_COUNT] = {
		"shortest path", "smallest", "largest", "closest to zero", "random"
	};
	return (nCriterion>=0 && nCriterion<TRAJECTORY_START_CRITERION_COUNT) ? szNames[nCriterion] : "unknown";
}

CTrajectoryStart::CTrajectoryStart()
:m_nCriterion(TRAJECTORY_START_SHORTEST_PATH)
,m_nParameterIndex(nNO_PARAMETER)
,m_pHint(NULL)
,m_nRandomSeed(0)
{
}

bool CTrajectoryStart::FixesTheStart() const
{
	if(m_nCriterion==TRAJECTORY_START_RANDOM)
		return true;					//a drawn candidate needs no axis
	if(m_nCriterion==TRAJECTORY_START_SHORTEST_PATH)
		return false;
	return IsResolved();				//no axis to compare: nothing to fix the start by
}

double CTrajectoryStart::StartCost(const CChainConfiguration &oCandidate
	,const CPostprocessorSettings &oSettings) const
{
	if(!m_pHint)
		return 0.0;
	//throws when the hint does not belong to this chain: a different count of parameters
	return m_pHint->Distance(oCandidate,oSettings);
}

size_t CTrajectoryStart::ChooseIndex(const tChainConfigurationVector &vCandidates
	,const CPostprocessorSettings &oSettings) const
{
	if(vCandidates.empty())
		return 0;

	//the hint first: the candidates nearest to it, which is usually more than one
	std::vector<size_t> vKept;
	vKept.reserve(vCandidates.size());
	if(m_pHint){
		if(m_pHint->IsAmbiguous())
			MY_THROW(CArgsMyException,"An ambiguous configuration cannot be the start a trajectory continues from");
		double dNearest = 0.0;
		for(size_t k=0;k<vCandidates.size();k++){
			//throws when the hint does not belong to this chain: a different count of parameters
			const double d = m_pHint->Distance(vCandidates[k],oSettings);
			if(vKept.empty() || d<dNearest-dEPSILON){
				dNearest = d;
				vKept.clear();
				vKept.push_back(k);
			} else if(are_equal(d,dNearest)){
				vKept.push_back(k);
			}
		}
	} else {
		for(size_t k=0;k<vCandidates.size();k++)
			vKept.push_back(k);
	}
	if(vKept.size()==1)
		return vKept.front();

	if(m_nCriterion==TRAJECTORY_START_RANDOM){
		std::mt19937 oGenerator(m_nRandomSeed);
		//a candidate out of the few that are left: the modulo is good enough for that and keeps the
		//choice the same on every platform for a given seed
		return vKept[(size_t)(oGenerator()%(unsigned long)vKept.size())];
	}
	//nothing to compare, or nothing asked for: the first of what the hint left
	if(!FixesTheStart())
		return vKept.front();
	if(m_nParameterIndex>=vCandidates.front().GetParametersCount())
		MY_THROW(CArgsMyException,"The axis the start of the trajectory is chosen by is not a parameter of these configurations");

	size_t nBest = vKept.front();
	double dBest = vCandidates[nBest][m_nParameterIndex].GetValue();
	if(m_nCriterion==TRAJECTORY_START_MIN_ABS)
		dBest = fabs(dBest);
	for(size_t n=1;n<vKept.size();n++){
		double d = vCandidates[vKept[n]][m_nParameterIndex].GetValue();
		if(m_nCriterion==TRAJECTORY_START_MIN_ABS)
			d = fabs(d);
		//strictly better only, so that equal values keep the first candidate
		const bool bBetter = (m_nCriterion==TRAJECTORY_START_MAX) ? (d>dBest+dEPSILON) : (d<dBest-dEPSILON);
		if(bBetter){
			dBest = d;
			nBest = vKept[n];
		}
	}
	return nBest;
}

std::string CTrajectoryStart::Describe() const
{
	std::string s = "Start of the trajectory: ";
	if(m_pHint)
		s += "nearest to "+m_pHint->toString()+", ties broken by ";
	if(m_nCriterion==TRAJECTORY_START_RANDOM){
		s += "a random candidate (seed "+size_to_string(m_nRandomSeed)
			+"): no convention of any control, the machine may start anywhere";
		return s;
	}
	if(!FixesTheStart()){
		s += "whichever candidate makes the whole trajectory shortest";
		if(m_nCriterion!=TRAJECTORY_START_SHORTEST_PATH)
			s += std::string(" (there is no axis to apply \"")+TrajectoryStartCriterionName(m_nCriterion)+"\" to)";
		return s;
	}
	s += std::string("the value of axis ")+size_to_string(m_nParameterIndex)+" "
		+TrajectoryStartCriterionName(m_nCriterion);
	return s;
}

} // namespace kin5ax
