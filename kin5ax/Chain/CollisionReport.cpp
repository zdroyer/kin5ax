#include "stdafx.h"
#include "CollisionReport.h"
#include "string_tools.h"
#include <algorithm>

namespace kin5ax {

const char *CollidingKindName(COLLIDING_KIND kind)
{
	switch(kind){
	case COLLIDING_PARTS:		return "and";
	case COLLIDING_TOOL:		return "cuts into";
	case COLLIDING_TOOL_SHANK:	return "is in";
	default:					return "and";
	}
}

bool CCollidingPair::IsTheSamePairAs(const CCollidingPair &o) const
{
	return m_nKind==o.m_nKind && m_sFirst==o.m_sFirst && m_sSecond==o.m_sSecond;
}

std::string CCollidingPair::Describe() const
{
	return m_sFirst+" "+CollidingKindName(m_nKind)+" "+m_sSecond;
}

CCollisionReport::CCollisionReport(void)
{
}

void CCollisionReport::Clear()
{
	m_vPairs.clear();
}

bool CCollisionReport::Empty() const
{
	return m_vPairs.empty();
}

void CCollisionReport::Add(const std::string &sFirst, const std::string &sSecond, COLLIDING_KIND kind)
{
	//two solids of the machine make the same pair whichever of them was looked at first, so the names
	//are put in an order of their own; a tool is always the first side of its pair
	CCollidingPair oPair(sFirst,sSecond,kind,1);
	if(kind==COLLIDING_PARTS && sSecond<sFirst){
		oPair.m_sFirst = sSecond;
		oPair.m_sSecond = sFirst;
	}
	for(size_t i=0;i<m_vPairs.size();i++)
		if(m_vPairs[i].IsTheSamePairAs(oPair)){
			m_vPairs[i].m_nCount++;
			return;
		}
	m_vPairs.push_back(oPair);
}

void CCollisionReport::MergeDistinct(const CCollisionReport &oOther)
{
	const std::vector<CCollidingPair> &vOther = oOther.GetPairs();
	for(size_t i=0;i<vOther.size();i++){
		bool bFound = false;
		for(size_t k=0;k<m_vPairs.size();k++)
			if(m_vPairs[k].IsTheSamePairAs(vOther[i])){
				m_vPairs[k].m_nCount++;
				bFound = true;
				break;
			}
		if(!bFound){
			CCollidingPair oPair = vOther[i];
			oPair.m_nCount = 1;
			m_vPairs.push_back(oPair);
		}
	}
}

const std::vector<CCollidingPair> &CCollisionReport::GetPairs() const
{
	return m_vPairs;
}

std::string CCollisionReport::Describe() const
{
	std::vector<CCollidingPair> vSorted = m_vPairs;
	//most frequent first, and the ties in the order they were met, which std::stable_sort keeps
	std::stable_sort(vSorted.begin(),vSorted.end()
		,[](const CCollidingPair &a, const CCollidingPair &b){ return a.m_nCount>b.m_nCount; });
	std::string s;
	for(size_t i=0;i<vSorted.size();i++){
		if(i)
			s += "\n";
		s += (vSorted[i].Describe()+": "+size_to_string(vSorted[i].m_nCount));
	}
	return s;
}

} // namespace kin5ax
