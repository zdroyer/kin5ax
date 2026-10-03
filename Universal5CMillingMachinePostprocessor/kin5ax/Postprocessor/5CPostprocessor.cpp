#include "../stdafx.h"
#include "5CPostprocessor.h"
#include "string_tools.h"

namespace kin5ax {

extern CR3Vector r3vOZ;

C5CPostprocessor::C5CPostprocessor(const CChain &oChain, const CPostprocessorSettings &oSettings)
:m_oSettings(oSettings)
,m_vSigns(5,1.0)
,m_m4x4InitialOrientationTransformation(1.0)
,m_m4x4InitialPositionTransformation(1.0)									
{
	m_oOrientationPostprocessor.SetSettings(m_oSettings);
	CreateSigns(oChain);
	CreateOrientationPostprocessor(oChain);
	CreatePositionPostprocessor(oChain);
}
void C5CPostprocessor::CreateSigns(const CChain &oChain)
{
	//links in material holder part act inverted (links are indexed from material holder to tool)
	size_t nMaterialHolderLinks = oChain.m_vMaterialHolderPart.size()-1;
	for(size_t i=0;i<m_vSigns.size();i++)
		m_vSigns[i] = (i<nMaterialHolderLinks) ? -1.0 : 1.0;
}
void C5CPostprocessor::CreateOrientationPostprocessor(const CChain &oChain)
{

	size_t index;
	CTransformation F1 = oChain.GetFirstConstantRotation(index);
	m_nFirstRotationIndex = index;
	F1.ZeroTranslation();
	m_m4x4InitialOrientationTransformation =  F1.T();
	CTransformation F2 = oChain.GetMiddleConstantRotation();
	F2.ZeroTranslation();
	CTransformation F3 = oChain.GetLastConstantRotation(index);
	m_nSecondRotationIndex = index;
	F3.ZeroTranslation();
	m_oOrientationPostprocessor.SetAxes(F2,F3*r3vOZ);

}
void C5CPostprocessor::CreatePositionPostprocessor(const CChain &oChain)
{
	CR3Vector r3vT1, r3vT2,r3vT3;
	CR3Point r3pP1;
	oChain.GetTranslationAxesAndOffset(
		r3vT1,r3vT2,r3vT3,r3pP1,
		CChainConfiguration(5));
	m_oPositionPostprocessor.SetAxesAndOffset(
		r3vT1,r3vT2,r3vT3,r3pP1);
	oChain.GetTranslationsIndexes(
		m_nFirstTranslationIndex,
		m_nSecondTranslationIndex,
		m_nThirdTranslationIndex);
}

C5CPostprocessor::~C5CPostprocessor(void)
{
}



std::vector<tChainConfigurationWithResult> C5CPostprocessor::SolveWithLocations(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,CMillingTool &oMillingTool, std::string &sLog)
{
	tChainConfigurationVector vResult2 = Solve(r3pPosition,r3vOrientation,oChain,oMillingTool,sLog);
	std::vector<tChainConfigurationWithResult> v(vResult2.size());
	for(size_t i=0;i<vResult2.size();i++){
		v[i].first = vResult2[i];
		oChain.SetParameters(vResult2[i]);
		v[i].second = oChain.GetActualToolPositionAndOrientation();
	}
	return v;

}


tChainConfigurationVector C5CPostprocessor::Solve(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,CMillingTool &oMillingTool, std::string &sLog)
{
	CSolveReport oReport;
	return Solve(r3pPosition,r3vOrientation,oChain,oMillingTool,sLog,oReport);
}

tChainConfigurationVector C5CPostprocessor::Solve(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,CMillingTool &oMillingTool, std::string &sLog, CSolveReport &oReport)
{
	oReport.Clear();
	tChainConfigurationVector  vResult = SolveTheoretically(r3pPosition,r3vOrientation,oChain,sLog,&oReport);
	vResult = ApplyParameterConstraints(vResult,oChain,sLog);
	oReport.m_nWithinLimits = vResult.size();
	vResult = ApplyCollisionDetection(vResult,oChain,oMillingTool,sLog);
	oReport.m_nCollisionFree = vResult.size();
	for(size_t i=0;i<vResult.size();i++)
		if(vResult[i].IsAmbiguous())
			oReport.m_bAmbiguous = true;
	sLog+=(size_to_string(vResult.size())+ " solution(s) meet(s) constraints and do(es)n't cause collisions\n");
    return vResult;
}

tChainConfigurationVector C5CPostprocessor::ApplyParameterConstraints(const tChainConfigurationVector &vSolutions, CChain &oChain,std::string &sLog)
{
	if(!oChain.HasParameterConstraints())
		return vSolutions;

	tChainConfigurationVector vResults;
	for(size_t i=0; i<vSolutions.size();i++){
		tChainConfigurationVector vEquivalents = vSolutions[i].GetEquivalents();
		for(size_t j=0;j<vEquivalents.size();j++){
			std::string sExceededConstraints="";
			CHAIN_CONSTRAINTS_RESULT result = oChain.MeetsParametersConstraints(vEquivalents[j],sExceededConstraints);
			if(result==CHAIN_CONSTRAINTS_MET){
				vResults.push_back(vEquivalents[j]);
				break;
			} else {
				if(result==CHAIN_CONSTRAINTS_ROTATIONS_EXCEEDED
					&& vEquivalents.size()>0
					&& j<vEquivalents.size()-1){
					continue;
				}else{
					sLog+=(std::string("Solution: ")+vSolutions[i].toString()+" (and equivalents) rejected;\nexceeded constraint(s):"+sExceededConstraints+sNL);
					break;
				}
			}
		}

	}
	return vResults;
}

tChainConfigurationVector C5CPostprocessor::ApplyCollisionDetection(const tChainConfigurationVector &vSolutions, CChain &oChain,CMillingTool &oMillingTool,std::string &sLog)
{
	std::string sCollisionLog;
	tChainConfigurationVector vResults;
	for(size_t i=0;i<vSolutions.size();i++){
		oChain.SetParameters(vSolutions[i]);
		if(oChain.HasCollision(oMillingTool,sCollisionLog)){
			sLog+=sCollisionLog;
		} else {
			vResults.push_back(vSolutions[i]);
		}
	}
	return vResults;

}


tChainConfigurationVector C5CPostprocessor::SolveWithKnownAlpha1(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,std::string &sLog,std::vector<double> vAlpha1,CMillingTool &oMillingTool)
{
	if(vAlpha1.size()==0)
		vAlpha1.push_back(0.0);
	tChainConfigurationVector vSolutions = Solve(r3pPosition,r3vOrientation,oChain,oMillingTool,sLog);
	tChainConfigurationVector vSolutions2;
	tChainConfigurationVector vResult;
	vResult.reserve(4);
	vSolutions2.reserve(4);
	for(size_t i=0; i<vSolutions.size();i++){
		for(size_t j=0;j<vAlpha1.size();j++){
			vSolutions2.push_back(vSolutions[i]);
			vSolutions2.back().SetAmbiguousParameter(vAlpha1[j]);
		}
	}

	CR3Vector r3vT1,r3vT2,r3vT3;
	CR3Point r3pP1;
	std::vector<CChainConfiguration> vShifts;
	
	for(size_t i = 0; i<vSolutions2.size(); i++){
		oChain.GetTranslationAxesAndOffset(r3vT1,r3vT2,r3vT3,r3pP1,vSolutions2[i]);
		std::string sExceptionMessage="";
		try{
			m_oPositionPostprocessor.SetAxesAndOffset(r3vT1,r3vT2,r3vT3,r3pP1);
			vShifts = m_oPositionPostprocessor.Solve(r3pPosition);

		} catch (CGeneratorException e){
			sLog+=(" Error occurred during position solving for rotation no."+size_to_string(i+1)+":"+vSolutions2[i].toString()+"\n");
			sLog+=("  "+e.Message());
			continue;
		}
		sLog+=(" found "+size_to_string(vShifts.size())+" position solution for orientation no."+size_to_string(i+1)+"\n");
		for(size_t j=0;j<vShifts.size();j++){
			vResult.push_back(vSolutions2[i]);
			vResult.back()[m_nFirstTranslationIndex] = vShifts[j][0];
			vResult.back()[m_nFirstTranslationIndex].ApplySign(m_vSigns[m_nFirstTranslationIndex]);
			vResult.back()[m_nSecondTranslationIndex] = vShifts[j][1];
			vResult.back()[m_nSecondTranslationIndex].ApplySign(m_vSigns[m_nSecondTranslationIndex]);
			vResult.back()[m_nThirdTranslationIndex] = vShifts[j][2];
			vResult.back()[m_nThirdTranslationIndex].ApplySign(m_vSigns[m_nThirdTranslationIndex]);
		}
	}
	return vResult;


}
tChainConfigurationVector C5CPostprocessor::SolveTheoretically(const CR3Point  r3pPosition, const CR3Vector  r3vOrientation, CChain &oChain,std::string &sLog, CSolveReport *pReport)
{
	CChainConfiguration oChainConfig(5);

	CR3Vector r3vV0 =  m_m4x4InitialOrientationTransformation * r3vOrientation;
	std::vector<CChainConfiguration> vRotations;
	try{
		std::vector<CChainConfiguration> vAngles = m_oOrientationPostprocessor.Solve(r3vV0);
		if(pReport)
			pReport->m_nOrientationSolutions = vAngles.size();
		sLog += ("found "+size_to_string(vAngles.size())+" generic orientation solution\n");
		for(size_t i=0;i<vAngles.size();i++){
			vRotations.push_back(CChainConfiguration(5));
			vRotations.back()[m_nFirstRotationIndex] = vAngles[i][0];
			vRotations.back()[m_nFirstRotationIndex].ApplySign(m_vSigns[m_nFirstRotationIndex]);
			vRotations.back()[m_nSecondRotationIndex] = vAngles[i][1];
			vRotations.back()[m_nSecondRotationIndex].ApplySign(m_vSigns[m_nSecondRotationIndex]);	
		}
	} catch (CGeneratorException e){
		sLog+=("Error occurred during orientation solving:\n"+e.Message());
		return std::vector<CChainConfiguration>(0);
	}

	CR3Vector r3vT1, r3vT2,r3vT3;	//not static: one postprocessor per thread (KnownBugs #46)
	CR3Point r3pP1;
	std::vector<CChainConfiguration> vShifts;
	std::vector<CChainConfiguration> vResult2;
	for(size_t i = 0; i<vRotations.size(); i++){
		oChain.GetTranslationAxesAndOffset(r3vT1,r3vT2,r3vT3,r3pP1,vRotations[i]);
		std::string sExceptionMessage="";
		try{
			m_oPositionPostprocessor.SetAxesAndOffset(r3vT1,r3vT2,r3vT3,r3pP1);
			vShifts = m_oPositionPostprocessor.Solve(r3pPosition);

		} catch (CGeneratorException e){
			sLog+=(" Error occurred during position solving for rotation no."+size_to_string(i+1)+":"+vRotations[i].toString()+"\n");
			sLog+=("  "+e.Message());
			continue;
		}
		sLog+=(" found "+size_to_string(vShifts.size())+" position solution for orientation no."+size_to_string(i+1)+"\n");
		for(size_t j=0;j<vShifts.size();j++){
			vResult2.push_back(vRotations[i]);
			vResult2.back()[m_nFirstTranslationIndex] = vShifts[j][0];
			vResult2.back()[m_nFirstTranslationIndex].ApplySign(m_vSigns[m_nFirstTranslationIndex]);
			vResult2.back()[m_nSecondTranslationIndex] = vShifts[j][1];
			vResult2.back()[m_nSecondTranslationIndex].ApplySign(m_vSigns[m_nSecondTranslationIndex]);
			vResult2.back()[m_nThirdTranslationIndex] = vShifts[j][2];
			vResult2.back()[m_nThirdTranslationIndex].ApplySign(m_vSigns[m_nThirdTranslationIndex]);
		}
	}
	if(pReport)
		pReport->m_nTheoreticalSolutions = vResult2.size();
	return vResult2;
}



std::vector<tChainConfigurationVector> C5CPostprocessor::SolveSingularOrientation(const std::vector<CR3CaughtVector> &vInput, CChain &oChain,CMillingTool &oMillingTool,std::string &sLog)
{
	std::vector<tChainConfigurationVector> vResult;
	if(vInput.size()==0)
		return vResult; 
	if(vInput.size()==1){
		sLog+="Only one cutter location for hypothetical singular solution:\n";
		tChainConfigurationVector v = Solve(vInput[0].m_p,vInput[0].m_v,oChain,oMillingTool,sLog);
		for(size_t i=0;i<v.size();i++){
			if(v[i].IsAmbiguous())
				v[i].SetAmbiguousParameter(0.0);
		}
		vResult.push_back(v);
		return vResult;
	}
	std::string sFirstLog;
	std::string sLastLog;
	tChainConfigurationVector vFirst,vLast;
	size_t nBegin=0;

	for(;nBegin<vInput.size();nBegin++){
		vFirst = Solve(vInput[nBegin].m_p,vInput[nBegin].m_v,oChain,oMillingTool,sFirstLog);
		if(vFirst.size())
			break;
	}
	if(vFirst.empty()){
		sLog+="No cutter location of the singular section can be solved:\n"+sFirstLog;
		return vResult;
	}
	std::vector<double> vAlpha1_1;
	for(size_t i=0;i<vFirst.size();i++)
		vAlpha1_1.push_back(vFirst[i].GetFirstAngleValue());
	if(!vFirst.front().IsAmbiguous())
		nBegin++;

	size_t nEnd = vInput.size();
	for(;nEnd>nBegin;nEnd--){
		vLast = Solve(vInput[nEnd-1].m_p,vInput[nEnd-1].m_v,oChain,oMillingTool,sLastLog);
		if(vLast.size())
			break;
	}	
	nEnd--;
	if(vLast.size()==0){
		for(size_t i = 0; i<vFirst.size();i++)
			vFirst[i].SetAmbiguousParameter(0.0);
		vResult.push_back(vFirst);
		return vResult;
	}
	std::vector<double> vAlpha1_2;
	for(size_t i=0;i<vLast.size();i++)
		vAlpha1_2.push_back( vLast[i].GetFirstAngleValue());
	if(!vLast.front().IsAmbiguous())
		nEnd--;
	assert(nBegin<=nEnd);
	double dTotalDistance=0.0;
	CR3Point p = vInput[nBegin].m_p;
	for(size_t i=nBegin+1;i<=nEnd;i++){
		dTotalDistance += d_E(p,vInput[i].m_p);
		p = vInput[i].m_p;
	}
	double dActD = 0;
	double t=0.0;
	double n = (double)nEnd-nBegin+2;
	p = vInput[nBegin].m_p;
	std::vector<double> vAlpha1;
	vAlpha1.reserve(4);
	std::string sIntermediateLog;
	for(size_t i=nBegin;i<=nEnd;i++){
		dActD += d_E(p,vInput[i].m_p);
		p=vInput[i].m_p;
		if(is_zero(dTotalDistance))
			t = (double)(i-nBegin+1)/n;
		else 
			t = dActD/dTotalDistance;
		vAlpha1.erase(vAlpha1.begin(),vAlpha1.end());
		for(size_t k=0;k<vAlpha1_1.size();k++){
			for(size_t l=0;l<vAlpha1_2.size();l++){
				double dA = (1.0-t)*vAlpha1_1[k]+t*vAlpha1_2[l];
				dA= fmod(dA,d2Pi);
				if(dA<0)
					dA= d2Pi+dA;
				vAlpha1.push_back(dA);
			}
		}
		vResult.push_back(SolveWithKnownAlpha1(vInput[i].m_p,vInput[i].m_v,oChain,sIntermediateLog,vAlpha1,oMillingTool));
	}
	return vResult;
}


} // namespace kin5ax
