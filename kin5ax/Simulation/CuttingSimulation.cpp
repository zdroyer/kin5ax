#include "stdafx.h"
#include "CuttingSimulation.h"

namespace kin5ax {

CCuttingSimulation::CCuttingSimulation()
:m_nGougeCount(0)
,m_nFirstGougeStep(0)
{
}

CCuttingSimulation::~CCuttingSimulation()
{
}

bool CCuttingSimulation::Run(CProgressCallback *pProgress)
{
	const size_t nCount = GetStepCount();
	while(true){
		if(pProgress){
			pProgress->SetProgress(nCount ? (double)GetStepIndex()/(double)nCount : 1.0);
			if(!pProgress->Continue())
				return false;
		}
		if(!Step())
			break;
	}
	if(pProgress)
		pProgress->SetProgress(1.0);
	return true;
}

size_t CCuttingSimulation::GetGougeCount() const
{
	return m_nGougeCount;
}

bool CCuttingSimulation::HasGouged() const
{
	return m_nGougeCount>0;
}

size_t CCuttingSimulation::GetFirstGougeStep() const
{
	return m_nFirstGougeStep;
}

const CR3Point & CCuttingSimulation::GetFirstGougePoint() const
{
	return m_r3pFirstGouge;
}

void CCuttingSimulation::NoteGouge(size_t nStep, const CR3Point &r3pWhere)
{
	if(m_nGougeCount==0){
		m_nFirstGougeStep = nStep;
		m_r3pFirstGouge = r3pWhere;
	}
	m_nGougeCount++;
}

void CCuttingSimulation::ForgetGouges()
{
	m_nGougeCount = 0;
	m_nFirstGougeStep = 0;
}

} // namespace kin5ax
