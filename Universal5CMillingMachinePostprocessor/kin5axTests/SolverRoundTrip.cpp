#include "stdafx.h"
#include "SolverRoundTrip.h"
#include "5CPostprocessor.h"
#include <random>

static double AngleDifference(double d1, double d2)
{
	double d = fmod(fabs(d1-d2),d2Pi);
	return min(d,d2Pi-d);
}

CSolverRoundTripResult RunSolverRoundTrip(CChain &oChain, CMillingTool &oTool, size_t nSamples, double dTolerance)
{
	CSolverRoundTripResult oResult = CSolverRoundTripResult();
	oResult.m_nSamples = nSamples;
	C5CPostprocessor oPostprocessor(oChain,CPostprocessorSettings());
	const int n = (int)oChain.GetNumberOfRegularLinks();

	std::mt19937 oGenerator(508);
	for(size_t s=0;s<nSamples;s++){
		//random joint values inside the limits (unlimited: +-0.1 or +-180 deg)
		CChainConfiguration oSet(n);
		for(int i=0;i<n;i++){
			const CChainSegment &oSegment = oChain[i];
			bool bRevolute = oSegment.GetType()==CHAIN_SEGMENT_REVOLUTE_LINK;
			double dMin = bRevolute ? -dPi : -0.1, dMax = bRevolute ? dPi : 0.1;
			if(oSegment.IsParameterConstrained()){
				dMin = oSegment.m_dParameterValueMinimum;
				dMax = oSegment.m_dParameterValueMaximum;
			}
			oSet[i].SetParameter(std::uniform_real_distribution<double>(dMin,dMax)(oGenerator),bRevolute ? RADIAN : DISTANCE_UNIT);
		}
		oChain.SetParameters(oSet);
		string sLog;
		if(oChain.HasCollision(oTool,sLog)){
			oResult.m_nCollisions++;
			continue;
		}
		oResult.m_nTargets++;
		CR3CaughtVector cvTarget = oChain.GetActualToolPositionAndOrientation();

		sLog = "";
		tChainConfigurationVector vSolutions = oPostprocessor.Solve(cvTarget.m_p,cvTarget.m_v,oChain,oTool,sLog);
		if(vSolutions.empty()){
			oResult.m_nNoSolution++;
			if(oResult.m_sFirstProblem.empty())
				oResult.m_sFirstProblem = "no solution for the joints "+oSet.toString()+"\n"+sLog;
			continue;
		}
		oResult.m_nSolutions+=vSolutions.size();
		bool bRecovered=false;
		for(size_t k=0;k<vSolutions.size();k++){
			oChain.SetParameters(vSolutions[k]);
			CR3CaughtVector cv = oChain.GetActualToolPositionAndOrientation();
			double dPosition = d_E(cv.m_p,cvTarget.m_p);
			double dOrientation = (cv.m_v-cvTarget.m_v).GetLength();
			oResult.m_dWorstPositionError = max(oResult.m_dWorstPositionError,dPosition);
			oResult.m_dWorstOrientationError = max(oResult.m_dWorstOrientationError,dOrientation);
			if(dPosition>dTolerance || dOrientation>dTolerance){
				oResult.m_nWrongSolutions++;
				if(oResult.m_sFirstProblem.empty())
					oResult.m_sFirstProblem = "wrong solution "+vSolutions[k].toString()+" for the joints "+oSet.toString()+"\n"+sLog;
			}
			bool bSame=true;
			for(int i=0;i<n && bSame;i++){
				if(vSolutions[k][i].IsAngle())
					bSame = AngleDifference(vSolutions[k][i].GetValue(),oSet[i].GetValue())<1E-7;
				else
					bSame = fabs(vSolutions[k][i].GetValue()-oSet[i].GetValue())<1E-7;
			}
			bRecovered = bRecovered || bSame;
		}
		if(!bRecovered){
			oResult.m_nNotRecovered++;
			if(oResult.m_sFirstProblem.empty())
				oResult.m_sFirstProblem = "the joints "+oSet.toString()+" are not among the solutions\n"+sLog;
		}
	}
	return oResult;
}
