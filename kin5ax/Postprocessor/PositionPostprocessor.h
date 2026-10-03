#pragma once
#include "R3Point.h"
#include "R3Vector.h"
#include "ChainConfiguration.h"


namespace kin5ax {

class CPositionPostprocessor
{
protected:
	//frame  T1 = Oz
	CR3Vector m_r3vT1;

	//axis of second translation
	CR3Vector m_r3vT2;

	//axis of third translation
	CR3Vector m_r3vT3;

	//offset point
	CR3Point m_r3pP1;

	double Get3x3Determinant(const CR3Vector &v0, const CR3Vector &v1,const CR3Vector &v2) const;
public:
	CPositionPostprocessor(void);
	~CPositionPostprocessor(void);
	void SetAxesAndOffset(const CR3Vector & r3vT1,const CR3Vector & r3vT2, const CR3Vector & r3vT3, const CR3Point & r3pP1);
	std::vector<CChainConfiguration> Solve(const CR3Point & r3pP0)const ;


};

} // namespace kin5ax
