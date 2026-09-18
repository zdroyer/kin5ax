#include "../StdAfx.h"
#include "OrientationPostprocessor.h"

extern CR3Vector r3vOZ;
extern CApplicationConfiguration oConfiguration;

COrientationPostprocessor::COrientationPostprocessor(const CTransformation &m4x4F2, const CR3Vector &r3vV3)
{
	m_bAxesSet=false;
	SetAxes(m4x4F2,r3vV3);
}

COrientationPostprocessor::~COrientationPostprocessor(void)
{
}

COrientationPostprocessor::COrientationPostprocessor(void)
:m_bAxesSet(false)
{
	m_dPiValue = acos(-1.0);
}
void COrientationPostprocessor::SetAxes(const CTransformation &m4x4F2, const CR3Vector &r3vV3)
{
	m_m4x4F2 = m4x4F2;
	m_r3vV3 = r3vV3;
	m_r3vV2 = m_m4x4F2 * m_r3vV3;
	m_r3vN1 = m_m4x4F2 * r3vOZ;
	m_r3vM = CR3Vector(-m_r3vN1.GetX(),-m_r3vN1.GetY(),m_r3vN1.GetZ());

	CheckAxes();
	m_r3vV2 = m_r3vV2.GetVersor();
	m_r3vV3 = m_r3vV3.GetVersor();
	m_r3vN1 = m_r3vN1.GetVersor();
	m_r3vM = m_r3vM.GetVersor();

	m_dPhi = m_r3vN1.UndirectedAngle(r3vOZ);
	m_dCosPhi = m_r3vN1.scalarProduct(r3vOZ);
	
	m_dPsi = m_r3vN1.UndirectedAngle(m_r3vV2);
	m_dCosPsi = m_r3vN1.scalarProduct(m_r3vV2);

	m_bAxesSet=true;
}
void COrientationPostprocessor::CheckAxes() const
{
	if(m_r3vN1.IsZero())
		MY_THROW(CArgsMyException,"Zero length axis of second rotation");

	if(m_r3vV3.IsZero())
		MY_THROW(CArgsMyException,"Zero length tool axis (V3)");

	if(m_r3vV2.IsZero())
		MY_THROW(CArgsMyException,"Zero length tool axis (V2)");

	if(m_r3vN1.Colinearity(r3vOZ)!=0 )
		MY_THROW(CArgsMyException, "Axis of second rotation is parallel to OZ (axis of first rotation)");

	if(m_r3vV3.Colinearity(r3vOZ))
		MY_THROW(CGeneratorException,"V2 is colinear with second revolute axis.");
}


size_t COrientationPostprocessor::SolutionsNumber(const CR3Vector& r3vV0) const
{
	//m_dPhi: n1&OZ angle
	//m_dPsi: n1&v2 angle
	double dNi = r3vOZ.UndirectedAngle(r3vV0);

	//  |-----|
	//           |----|
	if(m_dPhi< (dNi - m_dPsi -dEPSILON) )
		return 0;
	//  |-----|
	//        |----|
	if(are_equal(m_dPhi,dNi-m_dPsi))
		return 1;

	//  |-----|
	//      |----|
	if(m_dPhi <  (dNi +m_dPsi-dEPSILON))
		return 2;

	if(are_equal(m_dPhi,dNi+m_dPsi)){
		// |-----|
		// |-----|
		if(is_zero(dNi))
			return INT_MAX;
			//  |-----|
			//   |----|  		
		return 1;
	}
	//  |-----|
	//   |---|
	return 0;
	
}

vector<CChainConfiguration> COrientationPostprocessor::SolveIndependently(const CR3Vector& V0) const
{
	vector<CChainParameter> vAlpha1; 
	vector<CChainParameter> vAlpha2;
	if(oConfiguration.m_nAlpha1Method==ALPHA1_AXIS_MULTIPLICATION)
        vAlpha1 = getAlpha1AxisMethod(V0);
	else 
		vAlpha1 = getAlpha1ConeMethod(V0);
	if(oConfiguration.m_nAlpha2Method==ALPHA2_AXIS_MULTIPLICATION)
        vAlpha2 = getAlpha2AxisMethod(V0);
	else
		vAlpha2 = getAlpha2SquareEquation(V0);

	vector<CChainConfiguration> vResult;
	CChainConfiguration oChainConfig(2);

	CRotation r1,r2;
	CR3Vector z0(0.0,0.0,1.0),v0comp;
    for(size_t i=0;i<vAlpha1.size(); i++ ){
		r1 = CRotation::OZRotation(vAlpha1[i].GetValue());
		for(size_t j=0; j<vAlpha2.size(); j++){
			if(vAlpha2[j].GetType()==CHAIN_PARAMETER_SINGLE_NUMBER){
				r2 = CRotation(m_r3vN1,vAlpha2[j].GetValue());
				v0comp = r1*r2*m_r3vV2;
				if(v0comp.Colinearity(V0)==1){
					oChainConfig[0]= vAlpha1[i];
					oChainConfig[1]=vAlpha2[j];
					vResult.push_back(oChainConfig);
				}
			} 			
		}
	}
	return vResult;


}
vector<CChainConfiguration> COrientationPostprocessor::SolveAlpha1ThenAlpha2(const CR3Vector& V0) const
{
	vector<CChainParameter> vAlpha1; 
	if(oConfiguration.m_nAlpha1Method==ALPHA1_AXIS_MULTIPLICATION)
        vAlpha1 = getAlpha1AxisMethod(V0);
	else 
		vAlpha1 = getAlpha1ConeMethod(V0);
	
	CChainConfiguration oChainConfig(2);
	vector<CChainConfiguration> vResult;
	for(size_t i=0;i<vAlpha1.size();i++){
		if(vAlpha1[i].GetType()==CHAIN_PARAMETER_SINGLE_NUMBER
		 ||vAlpha1[i].GetType()==CHAIN_PARAMETER_ANY_NUMBER){
			oChainConfig[0] = vAlpha1[i];
			if(getAlpha2(vAlpha1[i].GetValue(),V0,oChainConfig[1]))
				vResult.push_back(oChainConfig);
		} 

	}
	return vResult;
}
vector<CChainConfiguration> COrientationPostprocessor::SolveAlpha2ThenAlpha1(const CR3Vector& V0) const
{
	vector<CChainParameter> vAlpha2;
	if(oConfiguration.m_nAlpha2Method==ALPHA2_AXIS_MULTIPLICATION)
        vAlpha2 = getAlpha2AxisMethod(V0);
	else
		vAlpha2 = getAlpha2SquareEquation(V0);

	CChainConfiguration oChainConfig(2);
	vector<CChainConfiguration> vResult;
	for(size_t i=0;i<vAlpha2.size();i++){
		if(vAlpha2[i].GetType()==CHAIN_PARAMETER_SINGLE_NUMBER){
			CChainParameter alpha1(RADIAN);
			if(getAlpha1(vAlpha2[i].GetValue(),V0,alpha1)){
				oChainConfig[1] = vAlpha2[i];
				oChainConfig[0] = alpha1;
				vResult.push_back(oChainConfig);
			} 
		} else {
			MY_THROW(CGeneratorException,"Not SINGLE_NUMBER alpha2 type");
		}
	}
	return vResult;
}
vector<CChainConfiguration> COrientationPostprocessor::Solve(const CR3Vector& r3vV0) const
{
	if(!m_bAxesSet)
		MY_THROW(CGeneratorException,"Orientation postprocessor not initialized");
	if(r3vV0.IsZero())
		MY_THROW(CArgsMyException,"COrientationPostprocessor: Zero length tool orientation vector V0");
	CR3Vector V0 = r3vV0.GetVersor();

	size_t nResultNumber = SolutionsNumber(V0);
	switch(oConfiguration.m_nAnglesSolvingMethod){
		case ANGLES_SOLVING_INDEPENDENTLY:
			return SolveIndependently(V0);
		case ANGLES_SOLVING_ALPHA1_THEN_ALPHA2:
			return SolveAlpha1ThenAlpha2(V0);
		default:
			return SolveAlpha2ThenAlpha1(V0);			
	}
}
vector<CChainParameter> COrientationPostprocessor::getAlpha1AxisMethod(const CR3Vector & r3vV0) const	
{
	vector<CChainParameter> vResult;
	if(is_zero(r3vV0.GetXYLengthSquare(),dEPSILON*dEPSILON)){
		vResult.push_back(CChainParameter(RADIAN,0.0,CHAIN_PARAMETER_ANY_NUMBER));
		return vResult;
	}
	double dBeta = atan2(r3vV0.GetY(),r3vV0.GetX());
	double dGamma = atan2(m_r3vN1.GetY(),m_r3vN1.GetX());
	double dNumerator = m_r3vN1 * m_r3vV2 - m_r3vN1.GetZ()*r3vV0.GetZ();
	double dDenominator = sqrt(r3vV0.GetX()*r3vV0.GetX()+r3vV0.GetY()*r3vV0.GetY())
		* sqrt(m_r3vN1.GetX()*m_r3vN1.GetX()+m_r3vN1.GetY()*m_r3vN1.GetY());
	if(is_zero(dDenominator)){
		//dwie pokrywaj¹ce siê osie obrotów (nx=ny=0) - wyeliminowane w konstruktorze
		if(is_zero(m_r3vN1.GetX()*m_r3vN1.GetX()) && is_zero(m_r3vN1.GetY()*m_r3vN1.GetY()))
			MY_THROW(CArgsMyException ,"This is not valid 5C kinematic chain:\nparalel rotations axes");
		//pionowe po³o¿enie fraza (i=j=0)				
		vResult.push_back(CChainParameter(RADIAN,0.0,CHAIN_PARAMETER_ANY_NUMBER));
		return vResult;
	}
	if(fabs(dNumerator)>fabs(dDenominator)){
		dDenominator = dNumerator;
		//todo: WHY!!!?
	}
	static double d2Pi = 2.0*acos(-1.0);
	double dACos = acos(dNumerator/dDenominator);
	//double dACos2 = d2Pi - dACos;
	double dAlpha1_1 = (dBeta - dGamma + dACos);
	double dAlpha1_2 = (dBeta - dGamma - dACos);

	if(are_equal(fabs(dAlpha1_1),d2Pi))
		dAlpha1_1 = 0.0; 
	else{
		if(dAlpha1_1 < -dEPSILON )
				dAlpha1_1 += d2Pi;
		else
			if(dAlpha1_1 > d2Pi)
				dAlpha1_1 -= d2Pi;
	}
	if(are_equal(fabs(dAlpha1_2),d2Pi))
		dAlpha1_2 = 0.0; 
	else{
		while(dAlpha1_2 < -dEPSILON )
				dAlpha1_2 += d2Pi;
		if(dAlpha1_2 > d2Pi)
			dAlpha1_2 -= d2Pi;
	}
	vResult.push_back( CChainParameter(RADIAN,dAlpha1_1) );
	if(are_not_equal(dAlpha1_1,dAlpha1_2))
		vResult.push_back( CChainParameter(RADIAN,dAlpha1_2));
	return vResult;
}


vector<CChainParameter> COrientationPostprocessor::getAlpha1ConeMethod(const CR3Vector & V0) const
{
	vector<CChainParameter> vResult;
	double vx = V0.GetXYLength();
	if(is_zero(vx,dEPSILON)){
		if((V0.GetZ()>0.0 && are_equal(m_dPhi,m_dPsi))
			|| (V0.GetZ()<0.0 && are_equal(m_dPhi, m_dPiValue-m_dPsi))){
			vResult.push_back(CChainParameter(RADIAN,0.0,CHAIN_PARAMETER_ANY_NUMBER));		} 
		return vResult;
	}
	double dKappa = atan2(V0.GetY(),V0.GetX());
	CRotation r0 = CRotation::OZRotation(dKappa);

	double k = V0.GetZ();
	double nz = m_dCosPhi;
	double nx = (m_dCosPsi - k*m_dCosPhi)/vx;
	double dUnderSqRoot = 1.0 - nz*nz - nx*nx;
	if(is_zero(dUnderSqRoot))
		dUnderSqRoot = 0.0;
	if(dUnderSqRoot<0.0)
		MY_THROW(CGeneratorException,"Alpha1 conic method fails - negative number as a agument of square root");
	double ny = sqrt(dUnderSqRoot);
	CR3Vector n_1 = r0*CR3Vector(nx,ny,nz);
	CR3Vector n_2 = r0*CR3Vector(nx,-ny,nz);

	CChainParameter oParam(RADIAN);
	ComputeOZRotationAngle(m_r3vN1,n_1,oParam);
	vResult.push_back(oParam);
	if(!n_1.IsEqual(n_2)){
		ComputeOZRotationAngle(m_r3vN1,n_2,oParam);
		vResult.push_back(oParam);
	}
	return vResult;
}
vector<CChainParameter> COrientationPostprocessor::getAlpha2SquareEquation(const CR3Vector & r3vV0)const
{
	static double dPiD2 = acos(0.0);
	static double dPi = acos(-1.0);
	static double d2Pi = 2.0*acos(-1.0);


	vector<CChainParameter> result;
	double dA = -(2.0*m_r3vN1.GetZ()* (m_r3vN1.scalarProduct( m_r3vV2)) - m_r3vV2.GetZ()- r3vV0.GetZ());
	double dB = -2.0*(m_r3vN1.GetX()*m_r3vV2.GetY() - m_r3vN1.GetY()*m_r3vV2.GetX());
	double dC = r3vV0.GetZ() - m_r3vV2.GetZ();
	if(is_zero(dA)){
		if(is_zero(dB,1E-14)){
			return getAlpha2AxisMethod(r3vV0);
		} 
		double dTan = -dC/dB;
		double dAngle = 2.0*atan(dTan);
		if(dAngle<0.0 && is_not_zero( dAngle))
			dAngle=dPi+dAngle;
		
		if(are_equal(dAngle,dPi))
			dAngle = 0.0;
		result.push_back(CChainParameter(RADIAN,dAngle));
		result.push_back(CChainParameter(RADIAN,dAngle+dPi));

		return result;
	} else {
		double dDelta = dB*dB-4.0*dA*dC;
		if(dDelta<-1E-10){
			return result;
		}
		if(dDelta<1E-12){
			double dAlpha2 = 2.0*atan(-dB/(2.0*dA));
			if(dAlpha2<0.0 && is_not_zero( dAlpha2))
				dAlpha2 += dPi;
			if(are_equal(dAlpha2,dPi))
				dAlpha2 = 0.0;
			result.push_back(CChainParameter(RADIAN, dAlpha2));
			result.push_back(CChainParameter(RADIAN, dAlpha2+dPi));
		} else {

			dDelta = sqrt(dDelta);
			double dATan_1 = 2.0*atan((-dB-dDelta)/(2.0*dA));
			if(dATan_1<0.0 && is_not_zero(dATan_1))
				dATan_1+=dPi;
			if(are_equal(dATan_1,dPi))
				dATan_1 = 0.0;
			result.push_back(CChainParameter(RADIAN, dATan_1));
			result.push_back(CChainParameter(RADIAN, dATan_1+dPi));

			double dATan_2 = 2.0*atan((-dB+dDelta)/(2.0*dA));
			if(dATan_2<0.0 && is_not_zero(dATan_2))
				dATan_2 += dPi;
			if(are_equal(dATan_2,dPi))
				dATan_2 = 0.0;
			if(are_not_equal(dATan_2, dATan_1)){
				result.push_back(CChainParameter(RADIAN, dATan_2));		
				result.push_back(CChainParameter(RADIAN, dATan_2+dPi));
			}
		}
	}
	return result;
}



vector<CChainParameter> COrientationPostprocessor::getAlpha2AxisMethod(const CR3Vector & r3vV0)const
{
	double dLambda = atan2(m_r3vV3.GetY(),m_r3vV3.GetX());
	double dMi = atan2(m_r3vM.GetY(),m_r3vM.GetX());
	double dDenominator = m_r3vV3.GetXYLength()*m_r3vM.GetXYLength();
	if(is_zero(dDenominator))
		MY_THROW(CArgsMyException,"Alpha2 axis multiplication method error\n Difference between alpha2 axis and tool versor is too small for actual\n floating point number precision - probably improper chain structure");
	double dNumerator = r3vV0.GetZ()-m_r3vM.GetZ()*m_r3vV3.GetZ();
	double d = dNumerator/dDenominator;
	if(are_equal(d,1.0))
		d=1.0;
	if(are_equal(d,-1.0))
		d=-1.0;
	vector<CChainParameter> vResult;
	if(fabs(d)>1.0)//bo solution
		return vResult;
	double dACos = acos(d);
	double dAlpha2_1(dMi - dLambda -dACos);
	double dAlpha2_2(dMi - dLambda +dACos);

	static double d2Pi = 2.0*acos(-1.0);

	if(are_equal(fabs(dAlpha2_1),d2Pi))
		dAlpha2_1 = 0.0; 
	else{
		if(dAlpha2_1 < -dEPSILON )
				dAlpha2_1 += d2Pi;
		else
			if(dAlpha2_1 > d2Pi)
				dAlpha2_1 -= d2Pi;
	}
	if(are_equal(fabs(dAlpha2_2),d2Pi))
		dAlpha2_2 = 0.0; 
	else{
		while(dAlpha2_2 < -dEPSILON )
				dAlpha2_2 += d2Pi;
		if(dAlpha2_2 > d2Pi)
			dAlpha2_2 -= d2Pi;
	}
	vResult.push_back( CChainParameter(RADIAN,dAlpha2_1) );
	if(are_not_equal(dAlpha2_1,dAlpha2_2))
		vResult.push_back( CChainParameter(RADIAN,dAlpha2_2) );
	return vResult;
}



bool COrientationPostprocessor::getAlpha1(double dAlpha2, const CR3Vector & V0versor, CChainParameter &oParam) const 
{
	CR3Vector a = m_m4x4F2 * CRotation::OZRotation(dAlpha2) * m_r3vV3;
	const CR3Vector &b = V0versor;
	return ComputeOZRotationAngle(a,b,oParam);

}
bool COrientationPostprocessor::getAlpha2(double dAlpha1, const CR3Vector & V0versor,CChainParameter &oParam )const
{
	const CR3Vector &a = m_r3vV3;
	CR3Vector b = m_m4x4F2.T() * CRotation::OZRotation(-dAlpha1) * V0versor;
	return ComputeOZRotationAngle(a,b,oParam);
}
bool COrientationPostprocessor::ComputeOZRotationAngle(const CR3Vector & input,const CR3Vector & result,CChainParameter  &oParam)const
{
	if(are_not_equal(input.GetZ(), result.GetZ()))
		return false;
	if(is_zero(input.GetXYLengthSquare())){
		if(is_zero(result.GetXYLengthSquare()))
			if((input.GetZ()*result.GetZ())>0.0){
					oParam = CChainParameter(RADIAN,0.0,CHAIN_PARAMETER_ANY_NUMBER);
				return true;
			}
		return false;
	}
	if(is_zero(result.GetXYLength()))
		return false;
	if(input.GetZ()*result.GetZ()<-dEPSILON)
		return false;
	static double d2Pi = 2.0*acos(-1.0);
	double dAlpha2 = atan2(result.GetY(),result.GetX());
	double dAlpha0 = atan2(input.GetY(),input.GetX());
	double dAlpha = dAlpha2-dAlpha0;
	if(is_zero(dAlpha)||are_equal(dAlpha,d2Pi)){
		dAlpha=0.0;
	} else {
		if(dAlpha<0.0){
			dAlpha+=d2Pi;
		} else {
			if(dAlpha>d2Pi)
				dAlpha-=d2Pi;
		}
	}
	oParam = CChainParameter(RADIAN,dAlpha);
	return true;
}

