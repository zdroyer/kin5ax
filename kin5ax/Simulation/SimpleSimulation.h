#pragma once

#include "Chain.h"
#include "MillingTool.h"

//A ready-made cutting job sized for a machine: a hemisphere that fits easily into the working space
//of the machine and suits its tool, and a material block around it. Used by the "Simple simulation" of
//the application and by the tests. Lengths are the unit of the chain file.
namespace kin5ax {

class CSimpleSimulationSetup
{
public:
	double m_dWorkingRange;		//the shortest travel of a limited linear axis, or the fallback when none is limited
	double m_dRadius;			//of the hemisphere: centred at the workpiece origin, equator at z = 0, pole at z = radius
	//The radius the cutter locations are written on: the programmed point of a ball nose tool is the centre of
	//its ball, so that centre has to travel one ball radius outside the surface for the tool to machine the
	//hemisphere of m_dRadius and not one ball radius less of it (KnownBugs #66).
	double m_dCLRadius;
	double m_dLocationDistance;	//between neighbouring cutter locations
	double m_dBoxSize;			//the material block: a square of this side around the hemisphere, ...
	double m_dBoxHeight;		//... from the equator plane up to the pole
	double m_dVoxelSize;

	static const double dFALLBACK_RANGE;	//1.0: the working range assumed when no linear axis is limited

	//The sizes for a machine: the radius is an eighth of the working range and at least three tool
	//radii; the locations are an eighth of the radius apart; the block is 2.5 radii wide and one radius
	//high, fifty voxels across.
	static CSimpleSimulationSetup ForMachine(const CChain &oChain, const CMillingTool &oTool);

	//the shortest travel of the limited linear axes of the chain, dFALLBACK_RANGE when none is limited
	static double WorkingRange(const CChain &oChain);

	//writes the hemisphere as CL data; the number of locations, 0 when the file cannot be written
	size_t WriteCLData(const std::string &sFilePath) const;
	//the lower corner of the block in x and y (the block is centred on the hemisphere)
	double GetBoxStart() const { return -0.5*m_dBoxSize; }
};

} // namespace kin5ax
