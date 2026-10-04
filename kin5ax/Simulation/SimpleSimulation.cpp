#include "../stdafx.h"
#include "SimpleSimulation.h"
#include "SphereCLDataGenerator.h"

namespace kin5ax {

const double CSimpleSimulationSetup::dFALLBACK_RANGE = 1.0;

double CSimpleSimulationSetup::WorkingRange(const CChain &oChain)
{
	double dRange = -1.0;
	const std::vector<CChainSegment> *pvParts[2] = {&oChain.m_vToolPart,&oChain.m_vMaterialHolderPart};
	for(size_t p=0;p<2;p++){
		for(size_t i=0;i<pvParts[p]->size();i++){
			const CChainSegment &oSegment = (*pvParts[p])[i];
			if(oSegment.GetType()!=CHAIN_SEGMENT_PRISMATIC_LINK || !oSegment.IsParameterConstrained())
				continue;
			double dTravel = oSegment.m_dParameterValueMaximum-oSegment.m_dParameterValueMinimum;
			if(dTravel>0.0 && (dRange<0.0 || dTravel<dRange))
				dRange = dTravel;
		}
	}
	return dRange>0.0 ? dRange : dFALLBACK_RANGE;
}

CSimpleSimulationSetup CSimpleSimulationSetup::ForMachine(const CChain &oChain, const CMillingTool &oTool)
{
	CSimpleSimulationSetup o;
	o.m_dWorkingRange = WorkingRange(oChain);
	double dToolRadius = std::max(oTool.m_dR,oTool.m_dRz);
	o.m_dRadius = std::max(o.m_dWorkingRange/8.0,3.0*dToolRadius);
	o.m_dCLRadius = o.m_dRadius + oTool.DefaultZeroPointZ();
	o.m_dLocationDistance = o.m_dRadius/8.0;
	o.m_dBoxSize = 2.5*o.m_dRadius;
	o.m_dBoxHeight = o.m_dRadius;
	o.m_dVoxelSize = o.m_dBoxSize/50.0;
	return o;
}

size_t CSimpleSimulationSetup::WriteCLData(const std::string &sFilePath) const
{
	return WriteSphereCLData(sFilePath,m_dCLRadius,m_dLocationDistance);
}

} // namespace kin5ax
