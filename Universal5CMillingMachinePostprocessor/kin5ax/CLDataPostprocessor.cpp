#include "stdafx.h"
#include "CLDataPostprocessor.h"
#include "5CPostprocessor.h"
#include "string_tools.h"

CCLDataPostprocessor::CCLDataPostprocessor(const CPostprocessorSettings &oSettings)
:m_oSettings(oSettings)
{
}

CCLDataPostprocessor::~CCLDataPostprocessor(void)
{
}

CCLDataPostprocessingResult CCLDataPostprocessor::Process
	(CChain &oChain
	,CMillingTool &oTool
	,CCutterLocationData &oData
	,CSimulationData &oSimulationData
	,FILE *pDetailedReport
	,int nDetailedReportContents
	,CProgressCallback *pProgress)
{
	CCLDataPostprocessingResult oResult;
	FILE *pFile = pDetailedReport;
	C5CPostprocessor oPostprocessor(oChain,m_oSettings);
	CCutterLocationData &data = oData;
	CR3Point p1,p2,pi;
	CR3Vector v1,v2,vi;

	size_t n = max((size_t)1,data.GetLocationsCount()/300);
	vector<pair<double,CR3CaughtVector> > vIntermediateLocations;

	data.ResetLocationIterator();
	data.GetNextLocation(p1,v1);
	data.ResetLocationIterator();
	oSimulationData.DeleteContents();
	string sLog;
	bool bCollectingAmbiguousSolutions=false;
	vector<CR3CaughtVector> vAmbiguousCollection;
	vAmbiguousCollection.reserve(50);
	CR3CaughtVector lastInput(CR3Vector(0.0),CR3Point(0.0));
	for(int i=0; data.GetNextLocation(p2,v2);i++){
		InterpolateLocation(p1,v1,p2,v2,vIntermediateLocations);
		for(size_t j=0;j<vIntermediateLocations.size();j++){
			sLog="";
			tChainConfigurationVector vSolutions = oPostprocessor.Solve
				(vIntermediateLocations[j].second.m_p
				,vIntermediateLocations[j].second.m_v
				,oChain
				,oTool
				,sLog);
			if(vSolutions.size() && vSolutions[0].IsAmbiguous()){
				if(!bCollectingAmbiguousSolutions){
					vAmbiguousCollection.erase(vAmbiguousCollection.begin(),vAmbiguousCollection.end());
					vAmbiguousCollection.push_back(lastInput);
					bCollectingAmbiguousSolutions=true;
				}
				vAmbiguousCollection.push_back(vIntermediateLocations[j].second);
				if(j+1>=vIntermediateLocations.size() && !data.HasMoreLocations()){
					string sSingularSolutionLog="";
					vector<tChainConfigurationVector> vUnambiguous = oPostprocessor.SolveSingularOrientation(vAmbiguousCollection,oChain,oTool,sSingularSolutionLog);
					for(size_t q=0;q<vUnambiguous.size();q++)
						oSimulationData.AddConfigurationSet(vUnambiguous[q]);
					if(pFile)
						fprintf(pFile,"%s\n",sSingularSolutionLog.c_str());
					bCollectingAmbiguousSolutions=false;
				}
			} else {
				if(bCollectingAmbiguousSolutions){
					vAmbiguousCollection.push_back(vIntermediateLocations[j].second);
					string sSingularSolutionLog="";
					vector<tChainConfigurationVector> vUnambiguous = oPostprocessor.SolveSingularOrientation(vAmbiguousCollection,oChain,oTool,sSingularSolutionLog);
					for(size_t q=0;q<vUnambiguous.size();q++)
						oSimulationData.AddConfigurationSet(vUnambiguous[q]);
					if(pFile)
						fprintf(pFile,"%s\n",sSingularSolutionLog.c_str());
					bCollectingAmbiguousSolutions=false;
				} 
                oSimulationData.AddConfigurationSet(vSolutions);
			}
			lastInput.m_p = vIntermediateLocations[j].second.m_p;
			lastInput.m_v = vIntermediateLocations[j].second.m_v;
			oResult.m_nTotalProblemsCount++;

			for(size_t k=0;k<vSolutions.size();k++){
				oChain.ComputeAndSetError(vSolutions[k],lastInput);
				if(oResult.m_dMaxOrientationErrorVectorLength<vSolutions[k].GetOrientationErrorInDistanceUnit()){
					oResult.m_dMaxOrientationErrorVectorLength = vSolutions[k].GetOrientationErrorInDistanceUnit();
					oResult.m_oMaxOrientationErrorSolution = vSolutions[k];
					oResult.m_cvMaxOrientationErrorInput = lastInput;
				}
				if(oResult.m_dMaxPositionError < vSolutions[k].GetPositionError()){
					oResult.m_dMaxPositionError = vSolutions[k].GetPositionError();
					oResult.m_oMaxPositionErrorSolution = vSolutions[k];
					oResult.m_cvMaxPositionErrorInput = lastInput;
				}
			}
			
			if(pFile){
				string s = "XYZ:"+vIntermediateLocations[j].second.m_p.toString()+", IJK:"+vIntermediateLocations[j].second.m_v.toString()+", location no. "+size_to_string(i+1);
				string s2="";

				for(size_t k=0;k<vSolutions.size();k++){
					s2+=(""+vSolutions[k].toString()+"\n; ");
				}
				if(vSolutions.size()){
					s2+=sNL;
				}
				switch(nDetailedReportContents){
					case 0:
						fprintf(pFile,"%s\n%s%s\n",s.c_str(),sLog.c_str(),s2.c_str());
						break;
					case 1:
						if(vSolutions.size()<2)
							fprintf(pFile,"%s\n%s%s\n",s.c_str(),sLog.c_str(),s2.c_str());
						break;
					default:
						if(vSolutions.size()<2)
							fprintf(pFile,"%s\n%s",s.c_str(),sLog.c_str());
				}				

			}
		}
		if(i%n==0){
			if(pProgress)
				pProgress->SetProgress((double)i/(double)data.GetLocationsCount());
		}
		p1=p2;
		v1=v2;
	}
	return oResult;
}

void CCLDataPostprocessor::InterpolateLocation
	( const CR3Point &p1
	, const CR3Vector &v1
	, const CR3Point &p2
	, const CR3Vector &v2
	, vector<pair<double,CR3CaughtVector> > & result) const
{
	result.erase(result.begin(),result.end());
	if(p1==p2 && v1==v2){
		result.push_back(pair<double,CR3CaughtVector>(1.0,CR3CaughtVector(v2,p2)));	
		return;
	}
	double dAngle = (v1.UndirectedUnitVectorsAngle(v2)/m_oSettings.m_dMaxOrientationDifferenceRadians);
	double dDistance = (d_E(p1,p2)/m_oSettings.m_dMaxPositionDifference);
	double dInv = ceil(max(dAngle,dDistance));
	if(dInv<=1.0){
		result.push_back(pair<double,CR3CaughtVector>(1.0,CR3CaughtVector(v2,p2)));
		return;
	} 
	double t;
	result.reserve((size_t)fabs(dInv));
	for(double i=1.0;i<dInv-0.001; i++){
		t = i/dInv;
		result.push_back(pair<double,CR3CaughtVector>(t,CR3CaughtVector(((v2*t)+(v1*(1.0-t))).GetUnitVector(),(p2*t)+(p1*(1.0-t)))));
	}
	result.push_back(pair<double,CR3CaughtVector>(1.0,CR3CaughtVector(v2,p2)));
}
