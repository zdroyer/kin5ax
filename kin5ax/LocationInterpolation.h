#pragma once

#include "R3AnchoredVector.h"
#include "PostprocessorSettings.h"
#include <vector>
#include <utility>

//The densification of a tool path, with no machine in sight: between two cutter locations the position
//moves on a straight line and the tool axis turns at a constant rate in the plane of both axes, in as
//many steps as the settings demand (CPostprocessorSettings::InterpolationSteps). The solver uses this
//to invent the locations it solves between two of a file; a simulation without a chain uses it to make
//a path as dense as the voxels it cuts.
namespace kin5ax {

//The locations between (p1,v1) and (p2,v2), as pairs of (0..1 parameter, location); the last pair is
//always (1.0, the second location) and the first location itself is never among them. One pair only
//when the settings ask for no interpolation (m_bInterpolateBetweenLocations) or when the two are one
//step apart or less.
void InterpolateLocation
	( const CR3Point &p1
	, const CR3Vector &v1
	, const CR3Point &p2
	, const CR3Vector &v2
	, const CPostprocessorSettings &oSettings
	, std::vector<std::pair<double,CR3AnchoredVector> > &result);

//One location of a dense path: where the tool goes, and which location of the original path it is or
//is on the way to. An invented location carries the index of the location before it, the way the
//records of the solver do (CSolvedLocation::m_nFileLocation); the arriving location carries its own.
struct CDenseLocation
{
	CR3AnchoredVector m_cv;		//the programmed point of the tool and its axis
	size_t m_nSource;			//index into the path this was made from
	bool m_bInvented;			//true for a location that is not of the path
};

//The whole path, dense: the first location as it is, then between each pair what InterpolateLocation
//gives. An empty path gives an empty result; a path of one location gives that one.
std::vector<CDenseLocation> DensifyLocations
	( const std::vector<CR3AnchoredVector> &vLocations
	, const CPostprocessorSettings &oSettings);

} // namespace kin5ax
