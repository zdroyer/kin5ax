#pragma once

// What the opaque handles of the C interface really are. Nothing here is visible to a host.
#include "stdafx.h"		//the prelude of the kin5ax library: its headers expect it to come first
#include "Chain.h"
#include "ChainFileReader.h"
#include "MillingTool.h"
#include "CLDataPostprocessor.h"
#include "SimulationData.h"
#include <memory>

//A machine. The solver writes the joint values of the chain while it works, which is why a handle
//belongs to one thread at a time.
struct k5_chain_t
{
	kin5ax::CChain m_oChain;
};

//A milling tool with its holder, owned through the base class the way the library does it.
struct k5_tool_t
{
	std::unique_ptr<kin5ax::CMillingTool> m_pTool;
};

//What one batch run produced: the solution sets with the trajectory chosen through them, and one
//record per location solved. The two have the same length (KnownBugs #70).
struct k5_path_t
{
	kin5ax::CSimulationData m_oData;
	kin5ax::CCLDataPostprocessingResult m_oResult;
};
