#pragma once
#include "r3point.h"
#include "r3vector.h"
#include "chainconfiguration.h"


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
	vector<CChainConfiguration> Solve(const CR3Point & r3pP0)const ;


};
