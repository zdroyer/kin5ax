#pragma once
// CL-data test file: cutter locations on a hemisphere, tool axis normal to the sphere.
// Based on the console tool SphereCLDGenerator.cpp (its interactive main became a function).

//Writes "GOTO/x,y,z,i,j,k" lines: circles of latitude from the equator (z=0) up to the pole
//(0,0,dR) with the tool axis (0,0,1) - the singular orientation of the machines with a
//vertical rotary axis. dD: distance between neighbouring cutter locations.
//Returns the number of the written locations, 0 if the file cannot be written.
size_t WriteSphereCLData(const std::string &sFileName, double dR, double dD);

//Writes nPoints (odd, at most 9 to stay below 10) "GOTO/x,y,z,i,j,k" lines along a meridian of the
//northern hemisphere in the XZ plane: from the equator (-dR,0,0) over the north pole (0,0,dR) -
//the middle point - to the opposite equator (dR,0,0); the tool axis is normal to the sphere.
//The path crosses the singular orientation (0,0,1) of the machines with a vertical rotary axis.
//Returns the number of the written locations, 0 if the file cannot be written.
size_t WriteMeridianCLData(const std::string &sFileName, double dR, size_t nPoints);
