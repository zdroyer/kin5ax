#pragma once

#include "Chain.h"
#include "MillingTool.h"
#include "CutterLocationData.h"
#include "SimulationData.h"
#include "PostprocessorSettings.h"
#include "ProgressCallback.h"
#include "SolveReport.h"

//Statistics of one CCLDataPostprocessor::Process run
namespace kin5ax {

class CCLDataPostprocessingResult
{
public:
	CCLDataPostprocessingResult(void)
		:m_nTotalProblemsCount(0)
		,m_bCancelled(false)
		,m_dMaxPositionError(-1.0)
		,m_dMaxOrientationErrorVectorLength(-1.0)
	{}
	//Solved problems: the cutter locations of the file and the interpolated ones. Equals the length of
	//m_vLocations and the size of the CSimulationData - every location solved has one record and one
	//(possibly empty) solution set (KnownBugs #70).
	size_t m_nTotalProblemsCount;
	//the progress callback said stop: the simulation data holds the locations solved so far
	bool m_bCancelled;

	//the biggest errors of the returned solutions (-1.0: no solution at all) with their inputs
	double m_dMaxPositionError;
	CChainConfiguration m_oMaxPositionErrorSolution;
	CR3AnchoredVector m_cvMaxPositionErrorInput;
	double m_dMaxOrientationErrorVectorLength;
	CChainConfiguration m_oMaxOrientationErrorSolution;
	CR3AnchoredVector m_cvMaxOrientationErrorInput;

	//One record per solution set handed to the CSimulationData, in the order of the sets: the status, the
	//origin and the cutter location of the file it belongs to. The length is the number of sets in the
	//CSimulationData and equals m_nTotalProblemsCount.
	std::vector<CSolvedLocation> m_vLocations;
	//Which solids of the machine met on the way, and at how many locations each pair did - a location
	//whose every configuration was rejected by the same pair counts once. Empty when the collision
	//check was off or nothing met.
	CCollisionReport m_oCollisions;
	//how many records have that status / that origin
	size_t CountOf(SOLVE_STATUS status) const;
	size_t CountOf(LOCATION_ORIGIN origin) const;
};

//Batch postprocessing of CL-data: every cutter location (and the locations interpolated
//between two of them) is solved for the given kinematic chain; the solution sets go to
//a CSimulationData (the shortest path through them is found later by CSimulationData).
class CCLDataPostprocessor
{
public:
	//a copy of the settings is kept (see C5CPostprocessor)
	CCLDataPostprocessor(const CPostprocessorSettings &oSettings);
	~CCLDataPostprocessor(void);

	//oChain, oTool: their actual parameters/position are changed during solving (the tool may be NULL
	//when the settings do not check collisions)
	//oSimulationData: its contents are replaced
	//pDetailedReport: NULL or an open text file for the log of the problems
	//nDetailedReportContents: 0 - all problems, 1 - those with less than 2 solutions (with the solutions),
	//                         2 - those with less than 2 solutions (the solver log only)
	//pProgress: NULL or the receiver of the progress
	CCLDataPostprocessingResult Process
		(CChain &oChain
		,CMillingTool &oTool
		,CCutterLocationData &oData
		,CSimulationData &oSimulationData
		,FILE *pDetailedReport
		,int nDetailedReportContents
		,CProgressCallback *pProgress);
	CCLDataPostprocessingResult Process
		(CChain &oChain
		,CMillingTool *pTool
		,CCutterLocationData &oData
		,CSimulationData &oSimulationData
		,FILE *pDetailedReport
		,int nDetailedReportContents
		,CProgressCallback *pProgress);

	//Interpolation between two cutter locations, as dense as the settings demand: kin5ax::InterpolateLocation
	//of LocationInterpolation.h with the settings of this postprocessor, kept here for the callers it has
	void InterpolateLocation
		( const CR3Point &p1
		, const CR3Vector &v1
		, const CR3Point &p2
		, const CR3Vector &v2
		, std::vector<std::pair<double,CR3AnchoredVector> > & result) const;
protected:
	CPostprocessorSettings m_oSettings;
};

} // namespace kin5ax
