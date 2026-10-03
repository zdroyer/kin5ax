#pragma once

#include <string>

//What C5CPostprocessor::Solve did for one cutter location, and why the location has the solutions
//it has: the stage at which the last candidate was lost.
namespace kin5ax {

enum SOLVE_STATUS {
	SOLVE_STATUS_SOLVED = 0,			//at least one solution
	SOLVE_STATUS_NO_ORIENTATION,		//the orientation solver found no joint angles for the tool axis (unreachable, or an error)
	SOLVE_STATUS_NO_POSITION,			//angles found, but the linear axes reach the point for none of them
	SOLVE_STATUS_LIMITS_EXCEEDED,		//every configuration, and every equivalent of it, violates a joint limit
	SOLVE_STATUS_COLLISION,				//every configuration within the limits collides
	SOLVE_STATUS_SINGULAR_UNSOLVED,		//a location of a singular section (first angle free) that the interpolated angle did not solve
	SOLVE_STATUS_COUNT
};

//a short name for logs and reports: "solved", "no orientation", "no position", "limits exceeded",
//"collision", "singular section unsolved"
const char *SolveStatusName(SOLVE_STATUS status);

//The counts of the stages of C5CPostprocessor::Solve for one location. Every stage keeps a subset of
//the stage before it, so the counts never grow.
class CSolveReport
{
public:
	CSolveReport(void);
	void Clear();
	size_t m_nOrientationSolutions;	//angle pairs found by the orientation solver
	size_t m_nTheoreticalSolutions;	//complete configurations, before any check
	size_t m_nWithinLimits;			//... of them inside the joint limits (one representative of each)
	size_t m_nCollisionFree;		//... of them without a collision: the returned solutions
	bool m_bAmbiguous;				//the orientation is singular: the first angle of the solutions is free (ANY)
	//the status that follows from the counts
	SOLVE_STATUS Status() const;
};

} // namespace kin5ax
