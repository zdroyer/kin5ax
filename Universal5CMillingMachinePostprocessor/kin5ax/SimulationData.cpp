#include "stdafx.h"
#include "SimulationData.h"
#include "string_tools.h"

namespace kin5ax {

CSimulationData::CSimulationData(size_t n)
:m_nEmptySolutionsCount(0)
,m_nActualSolution(0)
{
	ReserveMemory(n);
}

CSimulationData::~CSimulationData(void)
{
}

int CSimulationData::ExportToGCode(
					   FILE *pFile,
						size_t nXIndex, 
						size_t nYIndex,
						size_t nZIndex,
						char cAngle1Char,
						size_t nAngle1Index,
						char cAngle2Char,
						size_t nAngle2Index,
						bool bRadians) const
{
	size_t n = 1;
	for(size_t i=0; i<m_vSolutions.size();i++){
		if(m_vSolutions[i].size() && m_vIndexes[i]<m_vSolutions[i].size()) {
			const CChainConfiguration &oConfig = m_vSolutions[i][m_vIndexes[i]];
			double a1 = oConfig[nAngle1Index].GetValue();
			double a2 = oConfig[nAngle2Index].GetValue();
			if(!bRadians){
				a1=radians_to_degrees(a1);
				a2=radians_to_degrees(a2);
			}
			int res = fprintf(pFile,"N%zu G1 X%lf Y%lf Z%lf %c%lf %c%lf\n"
							,10*n
							,oConfig[nXIndex].GetValue()
							,oConfig[nYIndex].GetValue()
							,oConfig[nZIndex].GetValue()
							,cAngle1Char
							,a1
							,cAngle2Char	
							,a2);
			if(res <0)
				return res;
			n++;
		}
	}

	return 0;
}
const CChainConfiguration &CSimulationData::operator[](size_t n) const
{
	if(m_vSolutions[n].size())
		return m_vSolutions[n][m_vIndexes[n]];
	MY_THROW(CGeneratorException,"Empty ChainConfiguration set in SimulationData at index "+size_to_string(n));
}
void CSimulationData::ResetConfigurationsIterator()
{
	m_nActualSolution=0;
}
bool CSimulationData::NextConfiguration(CChainConfiguration &oChainConfiguration)
{
	while(m_nActualSolution<m_vSolutions.size()){
		if(m_vSolutions[m_nActualSolution++].size()){
			oChainConfiguration = m_vSolutions[m_nActualSolution-1][m_vIndexes[m_nActualSolution-1]];
			return true;
		}
	}
	return false;
}

size_t CSimulationData::GetSize() const
{
	return m_vSolutions.size();
}
void CSimulationData::ReserveMemory(size_t n)
{
	m_vSolutions.reserve(n);
	m_vIndexes.reserve(n);
}
void CSimulationData::AddConfigurationSet(const tChainConfigurationVector &vConfigurations)
{
	m_vSolutions.push_back(vConfigurations);
	m_vIndexes.push_back(0);
	if(vConfigurations.size()==0)
		m_nEmptySolutionsCount++;
}

tChainConfigurationVector CSimulationData::GetLastNonemptyConfigurationSet() const
{
	for(size_t i=m_vSolutions.size(); i>0; i--){
		if(m_vSolutions[i-1].size())
            return tChainConfigurationVector(m_vSolutions[i-1]);

	}
	return tChainConfigurationVector();
}

typedef std::vector<size_t> tSizeTVector;

static void ReportProgress(CProgressCallback *pProgress, double dFraction)
{
	if(pProgress)
		pProgress->SetProgress(dFraction);
}

void CSimulationData::FindShortestPath(const CPostprocessorSettings &oSettings, CProgressCallback *pProgress)
{
	ReportProgress(pProgress,0.0);
	for(size_t k =0; k< m_vIndexes.size();k++)
		m_vIndexes[k]=0;
	if(m_vIndexes.size()<2){	//one set: nothing to choose (two are enough, KnownBugs #43)
		ReportProgress(pProgress,1.0);
		return;
	}
	std::vector<tSizeTVector> vvClosestNeighbourIndexes(m_vSolutions.size(),tSizeTVector());
	vvClosestNeighbourIndexes.reserve(m_vSolutions.size());


	//searching for first unambiguous solution set
	size_t i=0;
	for(;i< m_vSolutions.size();i++){
		if(m_vSolutions[i].size()==0)
			continue;
		bool bIsAmbiguous = false;
		for(size_t k=0;k<m_vSolutions[i].size();k++){
			if(m_vSolutions[i][k].IsAmbiguous()){
				bIsAmbiguous=true;
				break;
			}
		}
		if(bIsAmbiguous)
			MY_THROW(CArgsMyException,"Ambiguous solution as an input for 'FindShortestPath'");
		break;
	}
	size_t j=i+1;
	for(;j< m_vSolutions.size();j++){
		if(m_vSolutions[j].size()==0)
			continue;
		bool bIsAmbiguous = false;
		for(size_t k=0;k<m_vSolutions[j].size();k++){
			if(m_vSolutions[j][k].IsAmbiguous()){
				bIsAmbiguous=true;
				break;
			}
		}
		if(bIsAmbiguous)
			MY_THROW(CArgsMyException,"Ambiguous solution as an input for 'FindShortestPath'");
		break;
	}
	std::vector<double> vDistances1,vDistances2;
	vDistances1.reserve(18);
	vDistances2.reserve(18);

	double dMinD=0;
	size_t nMinII;
	double dProgressCoef = 0.9;

	while(j<m_vSolutions.size()){
		ReportProgress(pProgress,(double)j*dProgressCoef/(double)m_vSolutions.size());
		vDistances1 = vDistances2;
		vDistances2.erase(vDistances2.begin(),vDistances2.end());
		vDistances2.reserve(m_vSolutions[j].size());
		if(vDistances1.size()==0){
			vDistances1 = std::vector<double>(m_vSolutions[i].size(),0.0);
			size_t n = vDistances1.size();;
			n++;
		}
		vvClosestNeighbourIndexes[j]=std::vector<size_t>(m_vSolutions[j].size(),0);
		vDistances2 = std::vector<double>(m_vSolutions[j].size(),dMinD);
		for(size_t l=0;l<m_vSolutions[j].size();l++){
			double d0 = vDistances1[0];
			dMinD = m_vSolutions[i][0].Distance(m_vSolutions[j][l],oSettings)+d0;
			nMinII=0;
			for(size_t k=1; k<m_vSolutions[i].size ();k++){
				double dd0 = vDistances1[k];
				double d = m_vSolutions[i][k].Distance(m_vSolutions[j][l],oSettings) + dd0;
				if(d<dMinD){
					dMinD=d;
					nMinII = k;
				}
			}
			vDistances2[l]=dMinD;
			vvClosestNeighbourIndexes[j][l] = nMinII;
		}
		i=j;
		for(j++;j< m_vSolutions.size();j++){
			if(m_vSolutions[j].size()==0)
				continue;
			bool bIsAmbiguous = false;
			for(size_t k=0;k<m_vSolutions[j].size();k++){
				if(m_vSolutions[j][k].IsAmbiguous()){
					bIsAmbiguous=true;
					break;
				}
			}
			if(bIsAmbiguous)
				MY_THROW(CArgsMyException,"Ambiguous solution as an input for 'FindShortestPath'");
			break;
		}
	}
	size_t nInd = 0;
	for(size_t m=1;m<vDistances2.size();m++){
		if(vDistances2[m]<vDistances2[nInd])
			nInd = m;
	}
	//backtracking from the last non-empty set (i) down to the first one;
	//vDistances2 is empty when less than two non-empty sets exist: nothing to choose then
	if(vDistances2.size()){
		m_vIndexes[i]=nInd;
		size_t nNext = i;	//the nearest non-empty set after m
		for(size_t m=i; m-->0; ){
			ReportProgress(pProgress,dProgressCoef+(1.0-dProgressCoef)*(double)(i-m)/(double)(i+1));
			if(m_vSolutions[m].empty())
				continue;	//the index of an empty set stays 0, such sets are skipped everywhere
			m_vIndexes[m]=vvClosestNeighbourIndexes[nNext][m_vIndexes[nNext]];
			nNext = m;
		}
	}
	ReportProgress(pProgress,1.0);
}		

void CSimulationData::DeleteContents()
{
	m_vSolutions.erase(m_vSolutions.begin(),m_vSolutions.end());
	m_vIndexes.erase(m_vIndexes.begin(),m_vIndexes.end());
	m_nEmptySolutionsCount=0;

}
size_t CSimulationData::GetEmptySolutionsCount(void)
{
	return m_nEmptySolutionsCount;
}

} // namespace kin5ax
