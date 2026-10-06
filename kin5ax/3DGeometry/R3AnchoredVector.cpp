#include "../stdafx.h"
#include "R3AnchoredVector.h"

namespace kin5ax {

CR3AnchoredVector::CR3AnchoredVector(void)
{
}

CR3AnchoredVector::CR3AnchoredVector(const CR3Vector &v, const CR3Point &p)
:m_v(v)
,m_p(p)
{
}

CR3AnchoredVector::~CR3AnchoredVector(void)
{
}

} // namespace kin5ax
