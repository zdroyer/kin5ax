#include "stdafx.h"
#include "ToolAssembly.h"

namespace kin5ax {

const char *szNO_TOOL = "NO_TOOL";

CToolDescription NoTool()
{
	return CToolDescription();			//given nothing, which is a tool of no length
}

CToolAssembly::CToolAssembly()
	:m_oHolder(CToolHolder(0.0,0.0))
	,m_oTool(NoTool())
{
	m_oHolder.SetModel(szNO_HOLDER);
}

CToolAssembly::CToolAssembly(const CToolHolder &oHolder, const CToolDescription &oTool)
	:m_oHolder(oHolder)
	,m_oTool(oTool)
{
}

CToolAssembly CToolAssembly::Empty()
{
	return CToolAssembly();
}

double CToolAssembly::GetLength() const
{
	//the holder hanging from the nose, and the tool sticking out of it
	return m_oHolder.GetLength() + (m_oTool.IsGiven() ? m_oTool.m_dLength : 0.0);
}

double CToolAssembly::GetToolOffset() const
{
	const double d = GetLength() - (m_oTool.IsGiven() ? m_oTool.GetReferencePointZ() : 0.0);
	return d>0.0 ? d : 0.0;
}

bool CToolAssembly::IsEmpty() const
{
	return !m_oTool.IsGiven() && m_oHolder.IsNoHolder();
}

bool CToolAssembly::HasBody() const
{
	return m_oHolder.HasBody() || m_oTool.IsGiven();
}

std::string CToolAssembly::Problem() const
{
	if(IsEmpty())
		return "";							//nothing in the spindle is a perfectly good answer
	if(m_oTool.IsGiven())
		return m_oTool.DimensionProblem();
	return "";								//a holder with no tool in it: a machine between jobs
}

CMillingTool * CToolAssembly::MakeTool() const
{
	if(!m_oTool.IsGiven())
		return NULL;						//no solids to make, whatever the holder is
	return m_oTool.MakeTool(m_oHolder);
}

} // namespace kin5ax
