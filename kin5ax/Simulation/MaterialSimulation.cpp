#include "stdafx.h"
#include "MaterialSimulation.h"

namespace kin5ax {

CMaterialSimulation::CMaterialSimulation(CChain &oChain, CMillingTool &oTool, CSimulationData &oTrajectory, CMaterial &oMaterial)
:m_oChain(oChain)
,m_oTool(oTool)
,m_oTrajectory(oTrajectory)
,m_oMaterial(oMaterial)
,m_nNext(0)
{
}

void CMaterialSimulation::Reset()
{
	m_nNext = 0;
	ForgetGouges();
}

bool CMaterialSimulation::Step()
{
	//skip the locations without a solution
	while(m_nNext<m_oTrajectory.GetSize() && !m_oTrajectory.HasSolution(m_nNext))
		m_nNext++;
	if(m_nNext>=m_oTrajectory.GetSize())
		return false;
	const CChainConfiguration &oConfiguration = m_oTrajectory[m_nNext++];
	m_oChain.SetParameters(oConfiguration);
	CR3AnchoredVector cv = m_oChain.GetActualToolPositionAndOrientation();
	m_oTool.SetPositionAndOrientation(cv.m_p,cv.m_v);
	m_oMaterial.MachineCut(m_oTool);
	//after the cut of this location: whatever the shank or the holder still touches is a gouge
	CR3Point r3pGouge;
	if(m_oMaterial.HasMaterialInNonCuttingPart(m_oTool,r3pGouge))
		NoteGouge(m_nNext-1,r3pGouge);
	return true;
}

size_t CMaterialSimulation::GetStepCount() const
{
	return m_oTrajectory.GetSize();
}

size_t CMaterialSimulation::GetStepIndex() const
{
	return m_nNext;
}

bool CMaterialSimulation::IsFinished() const
{
	for(size_t n=m_nNext;n<m_oTrajectory.GetSize();n++)
		if(m_oTrajectory.HasSolution(n))
			return false;
	return true;
}

} // namespace kin5ax
