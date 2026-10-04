#pragma once

#include "Chain.h"
#include "MillingTool.h"
#include "SimulationData.h"
#include "Material.h"
#include "ProgressCallback.h"

//The cutting simulation: the trajectory (one configuration per cutter location, the one chosen by
//CSimulationData::FindShortestPath) is replayed on the chain, the tool follows the tool frame of the
//chain, and the material is cut at every location. It has no thread of its own: the caller drives it
//with Step() (one location) or Run() (all remaining locations, with progress and cancellation).
//The four objects belong to the caller and must outlive the simulation; the chain's parameters and
//the tool's position are changed by every step.
//A rudimentary simulation for checking the geometry of the postprocessor: one cut per location, no
//sweep in between, no feed or time; see CMaterial.
namespace kin5ax {

class CMaterialSimulation
{
public:
	CMaterialSimulation(CChain &oChain, CMillingTool &oTool, CSimulationData &oTrajectory, CMaterial &oMaterial);

	//back to the first location; the material is not touched (CMaterial::ResetMaterial is the caller's decision)
	void Reset();
	//the next location that has a solution: the chain takes its configuration, the tool its frame, the
	//material is cut. False when the trajectory is exhausted (nothing is done then).
	bool Step();
	//all remaining locations. pProgress (may be NULL) receives the fraction done and may stop the run
	//with Continue() == false; false when stopped, true when the trajectory is exhausted.
	bool Run(CProgressCallback *pProgress);

	//---- gouges: locations where the shank or the holder of the tool met material that was still there.
	//A gouge does not stop the simulation; the counts tell the caller how bad the trajectory is.
	size_t GetGougeCount() const;
	bool HasGouged() const;
	//the location and the place of the first gouge (only meaningful when GetGougeCount() is not zero)
	size_t GetFirstGougeStep() const;
	const CR3Point & GetFirstGougePoint() const;

	//the locations of the trajectory, with and without a solution
	size_t GetStepCount() const;
	//the locations passed so far (those without a solution are passed without a cut)
	size_t GetStepIndex() const;
	bool IsFinished() const;

protected:
	CChain &m_oChain;
	CMillingTool &m_oTool;
	CSimulationData &m_oTrajectory;
	CMaterial &m_oMaterial;
	size_t m_nNext;		//the first location not yet passed
	size_t m_nGougeCount;
	size_t m_nFirstGougeStep;
	CR3Point m_r3pFirstGouge;
};

} // namespace kin5ax
