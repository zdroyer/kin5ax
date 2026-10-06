#include "stdafx.h"
#include "CLDataPostprocessor.h"
#include "5CPostprocessor.h"
#include "string_tools.h"

namespace kin5ax {

size_t CCLDataPostprocessingResult::CountOf(SOLVE_STATUS status) const
{
	size_t n = 0;
	for(size_t i=0;i<m_vLocations.size();i++)
		if(m_vLocations[i].m_nStatus==status)
			n++;
	return n;
}

size_t CCLDataPostprocessingResult::CountOf(LOCATION_ORIGIN origin) const
{
	size_t n = 0;
	for(size_t i=0;i<m_vLocations.size();i++)
		if(m_vLocations[i].m_nOrigin==origin)
			n++;
	return n;
}

//Hands the solution sets of a singular section over, one per location the section was responsible for.
//SolveSingularOrientation returns a set only for the locations it could use - it starts at the first one
//it can solve and stops at the last - so the locations it dropped are padded with empty sets here.
//Without the padding the simulation data and the records were shorter than the number of locations
//solved, and nothing said which locations had gone missing (KnownBugs #70).
static void AddSingularSectionSets(CCLDataPostprocessingResult &oResult, CSimulationData &oSimulationData
	,const std::vector<tChainConfigurationVector> &vUnambiguous, size_t nOwedLocations
	,size_t nFileLocation, size_t nFileLine)
{
	size_t nPushed = 0;
	for(;nPushed<vUnambiguous.size();nPushed++){
		oSimulationData.AddConfigurationSet(vUnambiguous[nPushed]);
		oResult.m_vLocations.push_back(CSolvedLocation(
			vUnambiguous[nPushed].empty() ? SOLVE_STATUS_SINGULAR_UNSOLVED : SOLVE_STATUS_SOLVED
			,LOCATION_SINGULAR_SECTION,nFileLocation,nFileLine));
	}
	for(;nPushed<nOwedLocations;nPushed++){
		oSimulationData.AddConfigurationSet(tChainConfigurationVector());
		oResult.m_vLocations.push_back(CSolvedLocation(SOLVE_STATUS_SINGULAR_UNSOLVED
			,LOCATION_SINGULAR_SECTION,nFileLocation,nFileLine));
	}
}

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
	return Process(oChain,&oTool,oData,oSimulationData,pDetailedReport,nDetailedReportContents,pProgress);
}

CCLDataPostprocessingResult CCLDataPostprocessor::Process
	(CChain &oChain
	,CMillingTool *pTool
	,CCutterLocationData &oData
	,CSimulationData &oSimulationData
	,FILE *pDetailedReport
	,int nDetailedReportContents
	,CProgressCallback *pProgress)
{
	CCLDataPostprocessingResult oResult;
	FILE *pFile = pDetailedReport;
	if(pTool)
		oChain.SetToolAssembly(*pTool);	//the tool point hangs below the tool mount by the length of the assembly
	C5CPostprocessor oPostprocessor(oChain,m_oSettings);
	CCutterLocationData &data = oData;
	CR3Point p1,p2,pi;
	CR3Vector v1,v2,vi;

	size_t n = std::max((size_t)1,data.GetLocationsCount()/300);
	std::vector<std::pair<double,CR3AnchoredVector> > vIntermediateLocations;

	data.ResetLocationIterator();
	data.GetNextLocation(p1,v1);
	data.ResetLocationIterator();
	oSimulationData.DeleteContents();
	std::string sLog;
	bool bCollectingAmbiguousSolutions=false;
	std::vector<CR3AnchoredVector> vAmbiguousCollection;
	vAmbiguousCollection.reserve(50);
	CR3AnchoredVector lastInput(CR3Vector(0.0),CR3Point(0.0));
	CSolveReport oReport;
	//which cutter location of the file a solution set belongs to: the one being solved, the one before it
	//(which is what everything interpolated on the way to it carries) and the one a singular section began at
	size_t nFileLocation = 0, nFileLine = 0, nPreviousLocation = 0, nPreviousLine = 0;
	size_t nSectionLocation = 0, nSectionLine = 0;
	for(int i=0; data.GetNextLocation(p2,v2,nFileLine);i++){
		nFileLocation = (size_t)i+1;
		if(pProgress && !pProgress->Continue()){
			oResult.m_bCancelled = true;
			break;
		}
		InterpolateLocation(p1,v1,p2,v2,vIntermediateLocations);
		for(size_t j=0;j<vIntermediateLocations.size();j++){
			sLog="";
			tChainConfigurationVector vSolutions = oPostprocessor.Solve
				(vIntermediateLocations[j].second.m_p
				,vIntermediateLocations[j].second.m_v
				,oChain
				,pTool
				,sLog
				,oReport);
			if(vSolutions.size() && vSolutions[0].IsAmbiguous()){
				if(!bCollectingAmbiguousSolutions){
					vAmbiguousCollection.erase(vAmbiguousCollection.begin(),vAmbiguousCollection.end());
					vAmbiguousCollection.push_back(lastInput);
					bCollectingAmbiguousSolutions=true;
					//the collection starts at the location solved before this one
					nSectionLocation = nPreviousLocation ? nPreviousLocation : nFileLocation;
					nSectionLine = nPreviousLocation ? nPreviousLine : nFileLine;
				}
				vAmbiguousCollection.push_back(vIntermediateLocations[j].second);
				if(j+1>=vIntermediateLocations.size() && !data.HasMoreLocations()){
					std::string sSingularSolutionLog="";
					std::vector<tChainConfigurationVector> vUnambiguous = oPostprocessor.SolveSingularOrientation(vAmbiguousCollection,oChain,pTool,sSingularSolutionLog);
					//the first location of the collection is the one solved before the section and has its
					//record already; every other one is ambiguous and waiting for its set
					AddSingularSectionSets(oResult,oSimulationData,vUnambiguous
						,vAmbiguousCollection.size()-1,nSectionLocation,nSectionLine);
					if(pFile)
						fprintf(pFile,"%s\n",sSingularSolutionLog.c_str());
					bCollectingAmbiguousSolutions=false;
				}
			} else {
				if(bCollectingAmbiguousSolutions){
					vAmbiguousCollection.push_back(vIntermediateLocations[j].second);
					std::string sSingularSolutionLog="";
					std::vector<tChainConfigurationVector> vUnambiguous = oPostprocessor.SolveSingularOrientation(vAmbiguousCollection,oChain,pTool,sSingularSolutionLog);
					//the first location of the collection had its record before the section, and the last one is
					//this location, which gets its own a few lines below
					AddSingularSectionSets(oResult,oSimulationData,vUnambiguous
						,vAmbiguousCollection.size()>=2 ? vAmbiguousCollection.size()-2 : 0
						,nSectionLocation,nSectionLine);
					if(pFile)
						fprintf(pFile,"%s\n",sSingularSolutionLog.c_str());
					bCollectingAmbiguousSolutions=false;
				} 
				//the last step of a stretch is the cutter location of the file itself, everything before it
				//was invented on the way to it
				const bool bFromFile = (j+1==vIntermediateLocations.size());
                oSimulationData.AddConfigurationSet(vSolutions);
				oResult.m_vLocations.push_back(CSolvedLocation(oReport.Status()
					,bFromFile ? LOCATION_FROM_FILE : LOCATION_INTERPOLATED
					,bFromFile ? nFileLocation : (nPreviousLocation ? nPreviousLocation : nFileLocation)
					,bFromFile ? nFileLine : nPreviousLine));
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
				std::string s = "XYZ:"+vIntermediateLocations[j].second.m_p.toString()+", IJK:"+vIntermediateLocations[j].second.m_v.toString()+", location no. "+size_to_string(i+1)
					+" - "+SolveStatusName(oReport.Status());
				std::string s2="";

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
		nPreviousLocation = nFileLocation;
		nPreviousLine = nFileLine;
	}
	if(pProgress && !oResult.m_bCancelled)
		pProgress->SetProgress(1.0);
	return oResult;
}

//a unit vector perpendicular to the unit vector u: u x (the coordinate axis u is least aligned with)
static CR3Vector PerpendicularUnitVector(const CR3Vector &u)
{
	double dX = fabs(u.GetX()), dY = fabs(u.GetY()), dZ = fabs(u.GetZ());
	CR3Vector axis = (dX<=dY && dX<=dZ) ? CR3Vector(1.0,0.0,0.0) : (dY<=dZ ? CR3Vector(0.0,1.0,0.0) : CR3Vector(0.0,0.0,1.0));
	return u.vectorProduct(axis).GetUnitVector();
}

void CCLDataPostprocessor::InterpolateLocation
	( const CR3Point &p1
	, const CR3Vector &v1
	, const CR3Point &p2
	, const CR3Vector &v2
	, std::vector<std::pair<double,CR3AnchoredVector> > & result) const
{
	result.erase(result.begin(),result.end());
	if(p1==p2 && v1==v2){
		result.push_back(std::pair<double,CR3AnchoredVector>(1.0,CR3AnchoredVector(v2,p2)));	
		return;
	}
	double dInv = m_oSettings.InterpolationSteps(d_E(p1,p2),v1.UndirectedUnitVectorsAngle(v2));
	if(dInv<=1.0){
		result.push_back(std::pair<double,CR3AnchoredVector>(1.0,CR3AnchoredVector(v2,p2)));
		return;
	} 
	//the axis turns at a constant rate from u1 towards u2 in the plane of both: u(a) = u1 cos a + w sin a,
	//w the unit vector perpendicular to u1 in that plane (any perpendicular one when the axes are antiparallel,
	//KnownBugs #42: the normalised linear blend was the zero vector half way, and not uniform in between)
	CR3Vector u1 = v1.GetUnitVector(), u2 = v2.GetUnitVector();
	double dAngle = u1.UndirectedUnitVectorsAngle(u2);
	CR3Vector w = u2 - u1*(u1*u2);
	w = w.IsZero(1E-9) ? PerpendicularUnitVector(u1) : w.GetUnitVector();
	double t;
	result.reserve((size_t)fabs(dInv));
	for(double i=1.0;i<dInv-0.001; i++){
		t = i/dInv;
		CR3Vector u = u1*cos(t*dAngle) + w*sin(t*dAngle);
		result.push_back(std::pair<double,CR3AnchoredVector>(t,CR3AnchoredVector(u,(p2*t)+(p1*(1.0-t)))));
	}
	result.push_back(std::pair<double,CR3AnchoredVector>(1.0,CR3AnchoredVector(v2,p2)));
}

} // namespace kin5ax
