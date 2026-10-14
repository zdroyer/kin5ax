#pragma once

#include "CollisionReport.h"
#include "R3AnchoredVector.h"
#include <string>
#include <cstddef>

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

//Where a location that was solved came from: a line of the CL file, the interpolation on the way to one
//of them, or the singular-section solver, which chooses its own samples out of what it is handed.
enum LOCATION_ORIGIN {
	LOCATION_FROM_FILE = 0,
	LOCATION_INTERPOLATED,
	LOCATION_SINGULAR_SECTION,
	LOCATION_ORIGIN_COUNT
};

//a short name for logs and reports: "from the file", "interpolated", "singular section"
const char *LocationOriginName(LOCATION_ORIGIN origin);

//One location handed to the CSimulationData by CCLDataPostprocessor::Process, in the order they were
//handed over: why it has the solutions it has, where it came from, which cutter location of the file it
//belongs to, and what it was solved for. One of these per solution set, always.
class CSolvedLocation
{
public:
	CSolvedLocation(SOLVE_STATUS status = SOLVE_STATUS_SOLVED
		,LOCATION_ORIGIN origin = LOCATION_FROM_FILE
		,size_t nFileLocation = 0
		,size_t nFileLine = 0
		,const CR3AnchoredVector &cvInput = CR3AnchoredVector())
		:m_nStatus(status)
		,m_nOrigin(origin)
		,m_nFileLocation(nFileLocation)
		,m_nFileLine(nFileLine)
		,m_cvInput(cvInput)
	{}
	SOLVE_STATUS m_nStatus;
	LOCATION_ORIGIN m_nOrigin;
	//The cutter location of the file this one belongs to, counted from 1: its own number for a location
	//of the file, the number of the one before it for everything interpolated on the way to it (the
	//locations between the 3rd and the 4th carry 3), and the number the section started at for a
	//singular section. 0 only when the file had no location at all.
	size_t m_nFileLocation;
	//The line of the CL file that cutter location was read from, counted from 1 the way an editor shows
	//it; 0 when it is not known.
	size_t m_nFileLine;
	//The point and the tool axis this location was solved for, in the units of the chain file: those
	//of the cutter location for one of the file, the ones invented on the way for anything interpolated.
	//A location without a solution has them all the same - they say where the tool was asked to be.
	CR3AnchoredVector m_cvInput;
};

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
	//Which solids met while the configurations of this location were being checked, and in how many of
	//those checks. Empty when the collision check was off or nothing met. What a location costs is one
	//count per pair however many of its configurations collided, which is CCollisionReport::MergeDistinct.
	CCollisionReport m_oCollisions;
	//the status that follows from the counts
	SOLVE_STATUS Status() const;
};

} // namespace kin5ax
