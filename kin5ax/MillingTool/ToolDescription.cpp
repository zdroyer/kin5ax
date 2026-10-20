#include "stdafx.h"
#include "ToolDescription.h"
#include "MillingToolBall.h"
#include "MillingToolCylinder.h"
#include "MillingToolToroidal.h"

#include <cstring>

namespace kin5ax {

namespace {

struct CNamedType { const char *szName; MILLING_TOOL_TYPE nType; };

//the three shapes a chain file describes; the library knows of more by name than it has solids for
const CNamedType vTYPES[] = {
	{"ball_end",  MILLING_TOOL_BALL},
	{"cylinder",  MILLING_TOOL_CYLINDER},
	{"toroidal",  MILLING_TOOL_TORUS},
};
const size_t nTYPES = sizeof(vTYPES)/sizeof(vTYPES[0]);

struct CNamedReference { const char *szName; TOOL_REFERENCE_POINT nReference; };

const CNamedReference vREFERENCES[] = {
	{"tip",         TOOL_REFERENCE_TIP},
	{"ball_centre", TOOL_REFERENCE_BALL_CENTRE},
	{"custom",      TOOL_REFERENCE_CUSTOM},
};
const size_t nREFERENCES = sizeof(vREFERENCES)/sizeof(vREFERENCES[0]);

} // namespace

const char *MillingToolTypeName(MILLING_TOOL_TYPE nType)
{
	for(size_t i=0;i<nTYPES;i++)
		if(vTYPES[i].nType==nType)
			return vTYPES[i].szName;
	return "";
}

bool MillingToolTypeFromName(const std::string &sName, MILLING_TOOL_TYPE &nType)
{
	for(size_t i=0;i<nTYPES;i++)
		if(sName==vTYPES[i].szName){
			nType = vTYPES[i].nType;
			return true;
		}
	return false;
}

const char *ToolReferencePointName(TOOL_REFERENCE_POINT nReference)
{
	for(size_t i=0;i<nREFERENCES;i++)
		if(vREFERENCES[i].nReference==nReference)
			return vREFERENCES[i].szName;
	return "";
}

bool ToolReferencePointFromName(const std::string &sName, TOOL_REFERENCE_POINT &nReference)
{
	for(size_t i=0;i<nREFERENCES;i++)
		if(sName==vREFERENCES[i].szName){
			nReference = vREFERENCES[i].nReference;
			return true;
		}
	return false;
}

CToolDescription::CToolDescription()
	:m_nType(MILLING_TOOL_CYLINDER)
	,m_dRadius(0.0)
	,m_dCornerRadius(0.0)
	,m_dCuttingHeight(0.0)
	,m_dLength(0.0)
	,m_nReference(TOOL_REFERENCE_TIP)
	,m_dReferenceHeight(0.0)
	,m_bGiven(false)
{
}

double CToolDescription::GetReferencePointZ() const
{
	switch(m_nReference){
	case TOOL_REFERENCE_BALL_CENTRE:
		//the centre of the ball of a ball nose tool, which is its radius above the tip
		return m_dCornerRadius;
	case TOOL_REFERENCE_CUSTOM:
		return m_dReferenceHeight;
	default:
		return 0.0;
	}
}

double CToolDescription::GetShankLength() const
{
	const double d = m_dLength-m_dCuttingHeight;
	return d>0.0 ? d : 0.0;
}

std::string CToolDescription::DimensionProblem() const
{
	if(!m_bGiven)
		return "no tool is described";
	if(m_dRadius<0.0 || m_dCornerRadius<0.0 || m_dCuttingHeight<0.0 || m_dLength<0.0
		|| m_dReferenceHeight<0.0)
		return "a dimension is negative";
	if(m_dLength<m_dCuttingHeight-dEPSILON)
		return "the tool sticks out of the holder by less than its cutting part is long";
	if(m_nReference==TOOL_REFERENCE_BALL_CENTRE && m_nType!=MILLING_TOOL_BALL)
		return "only a ball nose tool is programmed at the centre of its ball";
	//the ball of a ball nose tool is its corner radius: the two are the same measurement
	const double dR = (m_nType==MILLING_TOOL_CYLINDER) ? 0.0 : m_dCornerRadius;
	return CMillingTool::DimensionProblem(m_nType,m_dRadius,dR,m_dCuttingHeight
		,GetReferencePointZ(),GetShankLength());
}

CMillingTool * CToolDescription::MakeTool(const CToolHolder &oHolder) const
{
	if(!DimensionProblem().empty())
		return NULL;
	const double dReference = GetReferencePointZ();
	const double dShank = GetShankLength();
	switch(m_nType){
	case MILLING_TOOL_BALL:
		return new CMillingToolBall(m_dRadius,m_dCornerRadius,m_dCuttingHeight,dReference,dShank,oHolder);
	case MILLING_TOOL_TORUS:
		return new CMillingToolToroidal(m_dRadius,m_dCornerRadius,m_dCuttingHeight,dReference,dShank,oHolder);
	default:
		return new CMillingToolCylinder(m_dRadius,0.0,m_dCuttingHeight,dReference,dShank,oHolder);
	}
}

bool CToolDescription::operator==(const CToolDescription &o) const
{
	return m_bGiven==o.m_bGiven
		&& m_nType==o.m_nType
		&& m_dRadius==o.m_dRadius
		&& m_dCornerRadius==o.m_dCornerRadius
		&& m_dCuttingHeight==o.m_dCuttingHeight
		&& m_dLength==o.m_dLength
		&& m_nReference==o.m_nReference
		&& m_dReferenceHeight==o.m_dReferenceHeight;
}

} // namespace kin5ax
