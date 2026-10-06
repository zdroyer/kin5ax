#pragma once
#include "R3Vector.h"
#include "R3Point.h"

namespace kin5ax {

//A vector anchored at a point: the pair the solver works in, a direction with the place it applies at.
//The tool axis anchored at the tool point is the input of the inverse problem; the joint axis anchored
//at the origin of its segment is what a chain segment is made of. Named CR3CaughtVector until 2026-10-06.
class CR3AnchoredVector
{
public:
	CR3AnchoredVector();
	CR3AnchoredVector(const CR3Vector &v, const CR3Point &p);
	~CR3AnchoredVector(void);
	
	CR3Vector m_v;
	CR3Point m_p;
};

} // namespace kin5ax
