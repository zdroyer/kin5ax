#pragma once

#include "MillingTool.h"
#include "Material.h"
#include "CuttingSimulation.h"
#include "LocationInterpolation.h"
#include <vector>

//The cutting simulation with no machine at all: a tool path - programmed points and tool axes in the
//frame of the material, as a cutter location file carries them - made as dense as the settings ask
//(DensifyLocations), and the tool put at every location of it and stamped into the material. What a
//host that has a path and no chain wants: does this path, with the tool leaning the way it says, take
//off what it meant to, and does the shank or the holder go through what is left?
//The tool and the material belong to the caller and must outlive the simulation; the tool's position
//is changed by every step. Rudimentary in the same way as CMaterialSimulation: one cut per location.
namespace kin5ax {

class CToolPathSimulation : public CCuttingSimulation
{
public:
	//vLocations are made dense here, once; a location with an axis of length zero is refused with
	//CArgsMyException
	CToolPathSimulation(CMillingTool &oTool, const std::vector<CR3AnchoredVector> &vLocations
		, const CPostprocessorSettings &oSettings, CMaterial &oMaterial);

	void Reset();
	bool Step();

	size_t GetStepCount() const;
	size_t GetStepIndex() const;
	bool IsFinished() const;

	//the dense path, which is what the steps walk: GetStepCount() of them
	const std::vector<CDenseLocation> & GetLocations() const;

protected:
	CMillingTool &m_oTool;
	CMaterial &m_oMaterial;
	std::vector<CDenseLocation> m_vLocations;
	size_t m_nNext;		//the first location not yet passed
};

} // namespace kin5ax
