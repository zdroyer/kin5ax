#pragma once

#include "Chain.h"
#include "MillingTool.h"
#include "SimulationData.h"
#include "Material.h"
#include "CuttingSimulation.h"

//The cutting simulation on a machine: the trajectory (one configuration per cutter location, the one
//chosen by CSimulationData::FindShortestPath) is replayed on the chain, the tool follows the tool frame
//of the chain, and the material is cut at every location. It has no thread of its own: the caller
//drives it with Step() (one location) or Run() (all remaining locations, with progress and cancellation).
//The four objects belong to the caller and must outlive the simulation; the chain's parameters and
//the tool's position are changed by every step.
//A rudimentary simulation for checking the geometry of the postprocessor: one cut per location, no
//sweep in between, no feed or time; see CMaterial. The same with no chain is CToolPathSimulation.
namespace kin5ax {

class CMaterialSimulation : public CCuttingSimulation
{
public:
	CMaterialSimulation(CChain &oChain, CMillingTool &oTool, CSimulationData &oTrajectory, CMaterial &oMaterial);

	void Reset();
	//the next location that has a solution: the chain takes its configuration, the tool its frame, the
	//material is cut. False when the trajectory is exhausted (nothing is done then).
	bool Step();

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
};

} // namespace kin5ax
