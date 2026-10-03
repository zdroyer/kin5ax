#pragma once
// Round trip of the inverse kinematics solver (no test framework in here):
//   random joint values -> CChain forward kinematics -> C5CPostprocessor::Solve -> forward kinematics.

#include "Chain.h"
#include "MillingTool.h"
#include <random>

struct CSolverRoundTripResult
{
	size_t m_nSamples;			//random joint settings drawn (inside the joint limits)
	size_t m_nCollisions;		//... of them skipped: the pose collides, the solver is right to reject it
	size_t m_nTargets;			//... of them used as targets of the solver
	size_t m_nNoSolution;		//targets for which the solver returned nothing
	size_t m_nWrongSolutions;	//returned solutions that do not reproduce the target
	size_t m_nNotRecovered;		//targets for which the drawn joint values are not among the solutions
	size_t m_nSolutions;		//all returned solutions
	double m_dWorstPositionError;
	double m_dWorstOrientationError;
	std::string m_sFirstProblem;		//description of the first failing target, with the solver log
};

CSolverRoundTripResult RunSolverRoundTrip(kin5ax::CChain &oChain, kin5ax::CMillingTool &oTool, size_t nSamples, double dTolerance);

//random joint values inside the limits of the chain (unlimited: +-0.1 or +-180 deg)
kin5ax::CChainConfiguration RandomJointValues(const kin5ax::CChain &oChain, std::mt19937 &oGenerator);
