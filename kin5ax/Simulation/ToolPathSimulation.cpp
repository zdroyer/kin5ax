#include "stdafx.h"
#include "ToolPathSimulation.h"
#include "ArgsMyException.h"
#include "string_tools.h"

namespace kin5ax {

CToolPathSimulation::CToolPathSimulation(CMillingTool &oTool, const std::vector<CR3AnchoredVector> &vLocations
	, const CPostprocessorSettings &oSettings, CMaterial &oMaterial)
:m_oTool(oTool)
,m_oMaterial(oMaterial)
,m_nNext(0)
{
	for(size_t i=0;i<vLocations.size();i++)
		if(vLocations[i].m_v.IsZero(1E-12))
			MY_THROW(CArgsMyException,"The tool axis of location "+size_to_string(i+1)+" has no length: no direction to hold the tool in");
	m_vLocations = DensifyLocations(vLocations,oSettings);
}

void CToolPathSimulation::Reset()
{
	m_nNext = 0;
	ForgetGouges();
}

bool CToolPathSimulation::Step()
{
	if(m_nNext>=m_vLocations.size())
		return false;
	const CR3AnchoredVector &cv = m_vLocations[m_nNext++].m_cv;
	m_oTool.SetPositionAndOrientation(cv.m_p,cv.m_v.GetUnitVector());
	m_oMaterial.MachineCut(m_oTool);
	CR3Point r3pGouge;
	if(m_oMaterial.HasMaterialInNonCuttingPart(m_oTool,r3pGouge))
		NoteGouge(m_nNext-1,r3pGouge);
	return true;
}

size_t CToolPathSimulation::GetStepCount() const
{
	return m_vLocations.size();
}

size_t CToolPathSimulation::GetStepIndex() const
{
	return m_nNext;
}

bool CToolPathSimulation::IsFinished() const
{
	return m_nNext>=m_vLocations.size();
}

const std::vector<CDenseLocation> & CToolPathSimulation::GetLocations() const
{
	return m_vLocations;
}

} // namespace kin5ax
