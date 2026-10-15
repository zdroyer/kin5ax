#pragma once

// What the opaque handles of the C interface really are. Nothing here is visible to a host.
#include "stdafx.h"		//the prelude of the kin5ax library: its headers expect it to come first
#include "Chain.h"
#include "ChainFcm.h"
#include "ChainFileReader.h"
#include "MillingTool.h"
#include "CLDataPostprocessor.h"
#include "SimulationData.h"
#include "Simulation/Material.h"
#include "Simulation/MaterialSimulation.h"
#include "Simulation/ToolPathSimulation.h"
#include <memory>

//A machine. The solver writes the joint values of the chain while it works, which is why a handle
//belongs to one thread at a time.
struct k5_chain_t
{
	kin5ax::CChain m_oChain;
	//what the last .fcm conversion of this chain could not carry across, read or written; empty
	//for a chain that has been through neither, and read back by k5_fcm_remark
	kin5ax::CFcmReport m_oFcmReport;
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

//A block of material, in the frame of the material of the chain.
struct k5_material_t
{
	kin5ax::CMaterial m_oMaterial;
};

//A cutting simulation: on a chain over a solved path, or with no chain over the path itself. What it
//was made of belongs to the host, which keeps it alive for as long as the simulation lives.
struct k5_simulation_t
{
	k5_simulation_t(k5_chain_t *chain, k5_tool_t *tool, k5_path_t *path, k5_material_t *material)
		: m_pSimulation(new kin5ax::CMaterialSimulation(chain->m_oChain,*tool->m_pTool,path->m_oData,material->m_oMaterial))
		, m_pFree(NULL), m_pChain(chain), m_pTool(tool), m_pPath(path) {}
	k5_simulation_t(k5_tool_t *tool, const std::vector<kin5ax::CR3AnchoredVector> &vLocations
		, const kin5ax::CPostprocessorSettings &oSettings, k5_material_t *material)
		: m_pSimulation(), m_pFree(NULL), m_pChain(NULL), m_pTool(tool), m_pPath(NULL)
	{
		m_pFree = new kin5ax::CToolPathSimulation(*tool->m_pTool,vLocations,oSettings,material->m_oMaterial);
		m_pSimulation.reset(m_pFree);
	}
	std::unique_ptr<kin5ax::CCuttingSimulation> m_pSimulation;
	kin5ax::CToolPathSimulation *m_pFree;		//the same object when it is the kind with no chain, else NULL
	k5_chain_t *m_pChain;
	k5_tool_t *m_pTool;
	k5_path_t *m_pPath;
};
