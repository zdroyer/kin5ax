#include "StdAfx.h"
#include ".\positionpostprocessor.h"
#include "argsmyexception.h"


extern CR3Vector r3vOZ;

CPositionPostprocessor::CPositionPostprocessor(void)
{
}

CPositionPostprocessor::~CPositionPostprocessor(void)
{
}


double CPositionPostprocessor::Get3x3Determinant(
	const CR3Vector &v0, 
	const CR3Vector &v1,
	const CR3Vector &v2) const
{
	double d = v0.GetX()*v1.GetY()*v2.GetZ()+
			   v1.GetX()*v2.GetY()*v0.GetZ()+
			   v2.GetX()*v0.GetY()*v1.GetZ()-
			   v0.GetZ()*v1.GetY()*v2.GetX()-
			   v1.GetZ()*v2.GetY()*v0.GetX()-
			   v2.GetZ()*v0.GetY()*v1.GetX();
    return d;
}

void CPositionPostprocessor::SetAxesAndOffset(
	const CR3Vector & r3vT1, 
	const CR3Vector & r3vT2, 
	const CR3Vector & r3vT3, 
	const CR3Point & r3pP1)
{
	if(r3vT1.IsZero() || r3vT2.IsZero() || r3vT3.IsZero() )
		MY_THROW(CArgsMyException,"At least one translation axis i zero length");
	m_r3vT1 = r3vT1.GetVersor();
	m_r3vT2 = r3vT2.GetVersor();
	m_r3vT3 = r3vT3.GetVersor();
    m_r3pP1 = r3pP1;

}


vector<CChainConfiguration> CPositionPostprocessor::Solve(const CR3Point& r3pP0) const
{
	CR3Vector r3vP0 = (r3pP0 - m_r3pP1);

	vector<CChainConfiguration> vConfiguration;
	double dW = Get3x3Determinant(m_r3vT1,m_r3vT2,m_r3vT3);
	double dW1 = Get3x3Determinant(r3vP0,m_r3vT2,m_r3vT3);
	double dW2 = Get3x3Determinant(m_r3vT1,r3vP0,m_r3vT3);
	double dW3 = Get3x3Determinant(m_r3vT1,m_r3vT2,r3vP0);
	if(is_zero(dW)){
		MY_THROW(CGeneratorException,"Zero main determinant - prismatic links axes are linear dependent\n\tProbably	invalid kinematic chain structure\n");
	} else {
		vConfiguration.push_back(CChainConfiguration(3));
		vConfiguration.back()[0].SetParameter(dW1/dW,DISTANCE_UNIT);
		vConfiguration.back()[1].SetParameter(dW2/dW,DISTANCE_UNIT);
		vConfiguration.back()[2].SetParameter(dW3/dW,DISTANCE_UNIT);
	}
	return vConfiguration;
}
