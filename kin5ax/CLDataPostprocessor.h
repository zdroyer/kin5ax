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
	//solved problems: the cutter locations of the file and the interpolated ones
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

	//why every solution set handed to the CSimulationData is what it is: one status per set, in the
	//order of the sets (a location of a singular section counts as SOLVED or SINGULAR_UNSOLVED)
	std::vector<SOLVE_STATUS> m_vStatuses;
	//how many sets have the status
	size_t CountOf(SOLVE_STATUS status) const;
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

	//interpolation between two cutter locations, as dense as the settings demand: the position moves on a straight
	//line, the tool axis turns at a constant rate in the plane of both axes;
	//result: pairs of (0..1 parameter, location), the last one is always (1.0, second location)
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
