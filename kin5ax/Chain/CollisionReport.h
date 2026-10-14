#pragma once

#include <string>
#include <vector>
#include <cstddef>

//Which parts of a machine met, and how often. A collision check says yes or no; what a user needs in
//order to do anything about it is the pair - this box of that segment against that one - and over a
//whole tool path, how many locations each pair cost.
namespace kin5ax {

enum COLLIDING_KIND {
	COLLIDING_PARTS = 0,		//two solids of the machine
	COLLIDING_TOOL,				//the cutting part of the tool against a solid of the machine
	COLLIDING_TOOL_SHANK,		//its shank or its holder, which may not touch anything at all
	COLLIDING_KIND_COUNT
};

//"and", "cuts into", "is in": how the two sides of a pair of that kind are read between their names
const char *CollidingKindName(COLLIDING_KIND kind);

//Two solids that met, named the way the chain file names them, and the count of the checks - or, in
//the report of a whole path, of the locations - at which they did.
class CCollidingPair
{
public:
	CCollidingPair(const std::string &sFirst = "", const std::string &sSecond = ""
		,COLLIDING_KIND kind = COLLIDING_PARTS, size_t nCount = 1)
		:m_sFirst(sFirst)
		,m_sSecond(sSecond)
		,m_nKind(kind)
		,m_nCount(nCount)
	{}
	std::string m_sFirst;
	std::string m_sSecond;
	COLLIDING_KIND m_nKind;
	size_t m_nCount;
	//the same two solids met the same way, whatever the counts: what Add and Merge look for
	bool IsTheSamePairAs(const CCollidingPair &o) const;
	//"Z box 1 and Tool holder box 1", "the tool cuts into Material holder box 1"
	std::string Describe() const;
};

//The pairs met by one collision check, or by a whole run of them. Nothing is counted twice in one
//Add: a pair already there has its count raised instead.
class CCollisionReport
{
public:
	CCollisionReport(void);
	void Clear();
	bool Empty() const;
	//this pair met once more
	void Add(const std::string &sFirst, const std::string &sSecond, COLLIDING_KIND kind);
	//every pair of that report counted once here, whatever its count there: the way a report of a
	//location goes into the report of a path, where the count is the number of locations
	void MergeDistinct(const CCollisionReport &oOther);
	const std::vector<CCollidingPair> &GetPairs() const;
	//the pairs, most frequent first, one to a line: "Z box 1 and Tool holder box 1: 313"
	std::string Describe() const;

protected:
	std::vector<CCollidingPair> m_vPairs;
};

} // namespace kin5ax
