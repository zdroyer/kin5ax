#include "../stdafx.h"
#include "SolveReport.h"

namespace kin5ax {

const char *SolveStatusName(SOLVE_STATUS status)
{
	static const char *szNames[SOLVE_STATUS_COUNT] = {
		"solved", "no orientation", "no position", "limits exceeded", "collision", "singular section unsolved"
	};
	return (status>=0 && status<SOLVE_STATUS_COUNT) ? szNames[status] : "unknown";
}

const char *LocationOriginName(LOCATION_ORIGIN origin)
{
	static const char *szNames[LOCATION_ORIGIN_COUNT] = {
		"from the file", "interpolated", "singular section"
	};
	return (origin>=0 && origin<LOCATION_ORIGIN_COUNT) ? szNames[origin] : "unknown";
}

CSolveReport::CSolveReport(void)
{
	Clear();
}

void CSolveReport::Clear()
{
	m_nOrientationSolutions = m_nTheoreticalSolutions = m_nWithinLimits = m_nCollisionFree = 0;
	m_bAmbiguous = false;
}

SOLVE_STATUS CSolveReport::Status() const
{
	if(m_nCollisionFree)
		return SOLVE_STATUS_SOLVED;
	if(!m_nOrientationSolutions)
		return SOLVE_STATUS_NO_ORIENTATION;
	if(!m_nTheoreticalSolutions)
		return SOLVE_STATUS_NO_POSITION;
	if(!m_nWithinLimits)
		return SOLVE_STATUS_LIMITS_EXCEEDED;
	return SOLVE_STATUS_COLLISION;
}

} // namespace kin5ax
