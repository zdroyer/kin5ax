#include "../stdafx.h"
#include "R3CaughtVector.h"

namespace kin5ax {

CR3CaughtVector::CR3CaughtVector(void)
{
}

CR3CaughtVector::CR3CaughtVector(const CR3Vector &v, const CR3Point &p)
:m_v(v)
,m_p(p)
{
}

CR3CaughtVector::~CR3CaughtVector(void)
{
}

} // namespace kin5ax
