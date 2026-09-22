#pragma once
#include "R3Vector.h"
#include "ChainConfiguration.h"
#include "Rotation.h"
#include "PostprocessorSettings.h"


namespace kin5ax {

class COrientationPostprocessor
{
public:
	COrientationPostprocessor();
	COrientationPostprocessor(const CTransformation &m4x4F2, const CR3Vector& r3vV3);
	~COrientationPostprocessor(void);
	void CheckAxes() const;
	void SetAxes(const CTransformation &m4x4F2, const CR3Vector& r3vV3);
	void SetSettings(const CPostprocessorSettings &oSettings);
	std::vector<CChainConfiguration> Solve(const CR3Vector& r3vV0) const;
	size_t SolutionsNumber(const CR3Vector& r3vV0) const;

protected:

	std::vector<CChainConfiguration> SolveIndependently(const CR3Vector& r3vV0) const;
	std::vector<CChainConfiguration> SolveAlpha1ThenAlpha2(const CR3Vector& r3vV0) const;
	std::vector<CChainConfiguration> SolveAlpha2ThenAlpha1(const CR3Vector& r3vV0) const;

	bool m_bAxesSet;

	//copy of the solver settings (methods of the angles computation)
	CPostprocessorSettings m_oSettings;

	//const: PI value
	double m_dPiValue;

	//in radians: n1&oz angle
	double m_dPhi;
	double m_dCosPhi;

	//in radians: n1&v2 angle		
	double m_dPsi;
	double m_dCosPsi;


	// axis of second rotation in first rotation frame
	CR3Vector m_r3vN1;

	// axis of first rotation in second rotation frame
	CR3Vector m_r3vM;


	//tool axis in first rotation frame
	CR3Vector m_r3vV2;

	//tool axis in seconde rotation frame
	CR3Vector m_r3vV3;

	//transformation-rotation from N1 to OZ
	CTransformation m_m4x4F2;

	std::vector<CChainParameter> getAlpha1AxisMethod	(const CR3Vector & r3vV0) const ;
	std::vector<CChainParameter> getAlpha1ConeMethod(const CR3Vector & r3vV0) const ;
	
	std::vector<CChainParameter> getAlpha2SquareEquation(const CR3Vector & r3vV0) const ;
	std::vector<CChainParameter> getAlpha2AxisMethod(const CR3Vector & r3vV0) const;

	bool getAlpha1(double dAlpha2, const CR3Vector & V0unit, CChainParameter &) const ; 
	bool getAlpha2(double dAlpha1, const CR3Vector & V0unit,CChainParameter &)const;
	bool ComputeOZRotationAngle(const CR3Vector & input,const CR3Vector & result,CChainParameter &)const;


};

} // namespace kin5ax
