#include "../stdafx.h"
#include "Chain.h"
#include "string_tools.h"


namespace kin5ax {

extern CR3Vector r3vOZ;
extern CR3Point r3pZero;


CChain::CChain(void)
{
	m_dToolMountToToolPoint = 0.0;
	m_nActivePart=CHAIN_PART_SPINDLE;
	m_nActiveSegmentIndex=-1;

	SetActiveSegment(CHAIN_PART_TABLE,-1);
	AddSegmentAfterActiveSegment(CChainSegment(CHAIN_SEGMENT_TABLE,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0,0.0,0.0)));

	SetActiveSegment(CHAIN_PART_SPINDLE,-1);
	AddSegmentAfterActiveSegment(CChainSegment(CHAIN_SEGMENT_TOOL_SPINDLE_NOSE,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0,0.0,0.5)));
	m_sFileName = "<chain not saved>";
	m_nUnit = CHAIN_UNIT_M;		//what every model shipped is in; a file has to say for itself
}

const char *ChainUnitName(CHAIN_UNIT unit)
{
	switch(unit){
	case CHAIN_UNIT_MM:	return "mm";
	case CHAIN_UNIT_M:	return "m";
	case CHAIN_UNIT_IN:	return "in";
	default:			return "?";
	}
}

bool ParseChainUnit(const std::string &sName, CHAIN_UNIT &unit)
{
	//exactly the spelling of the file format, no case folding and no synonyms: a file that says
	//"MM" or "meter" was written by hand against the wrong description, and saying so is worth
	//more than guessing right
	for(int i=0;i<CHAIN_UNIT_COUNT;i++)
		if(sName==ChainUnitName((CHAIN_UNIT)i)){
			unit = (CHAIN_UNIT)i;
			return true;
		}
	return false;
}

double ChainUnitInMillimetres(CHAIN_UNIT unit)
{
	switch(unit){
	case CHAIN_UNIT_MM:	return 1.0;
	case CHAIN_UNIT_M:	return 1000.0;
	case CHAIN_UNIT_IN:	return 25.4;
	default:			return 0.0;
	}
}

CChain::~CChain(void)
{
}



size_t CChain::GetNumberOfRevoluteLinks() const
	{
		size_t n = 0;
		for(size_t i = 0; i<m_vToolPart.size(); i++){
			if(m_vToolPart[i].GetType() == CHAIN_SEGMENT_REVOLUTE_LINK)
				n++;
		}
		for(size_t i = 0; i<m_vMaterialHolderPart.size(); i++){
			if(m_vMaterialHolderPart[i].GetType() == CHAIN_SEGMENT_REVOLUTE_LINK)
				n++;
		}
		return n;
	}


size_t CChain::GetNumberOfPrismaticLinks() const
	{
		size_t n = 0;
		for(size_t i = 0; i<m_vToolPart.size(); i++){
			if(m_vToolPart[i].GetType() == CHAIN_SEGMENT_PRISMATIC_LINK)
				n++;
		}
		for(size_t i = 0; i<m_vMaterialHolderPart.size(); i++){
			if(m_vMaterialHolderPart[i].GetType() == CHAIN_SEGMENT_PRISMATIC_LINK)
				n++;
		}
		return n;
	}


bool CChain::Is5CChain() const
{
	if((GetNumberOfPrismaticLinks()!=3  
		|| GetNumberOfRevoluteLinks()!=2))
		return false;
	return true;
}

bool CChain::HasParameterConstraints() const
{
	for(int i=0;i<(int)GetNumberOfRegularLinks();i++)
		if((*this)[i].IsParameterConstrained())
			return true;
	return false;
}
void CChain::CheckStructure() const
{
	if(m_vToolPart.empty())
		MY_THROW(CGeneratorException,"the tool part has no segment");
	if(m_vMaterialHolderPart.empty())
		MY_THROW(CGeneratorException,"the material holder part has no segment");
	if(m_vToolPart.back().GetType()!=CHAIN_SEGMENT_TOOL_SPINDLE_NOSE)
		MY_THROW(CGeneratorException,"the last segment of the tool part must be the TOOL segment, '"
			+m_vToolPart.back().GetName()+"' is a "+m_vToolPart.back().GetTypeName());
	if(m_vMaterialHolderPart.back().GetType()!=CHAIN_SEGMENT_TABLE)
		MY_THROW(CGeneratorException,"the last segment of the material holder part must be the MATERIAL_HOLDER segment, '"
			+m_vMaterialHolderPart.back().GetName()+"' is a "+m_vMaterialHolderPart.back().GetTypeName());
	const std::vector<CChainSegment> *pvParts[2] = {&m_vToolPart,&m_vMaterialHolderPart};
	for(size_t p=0;p<2;p++){
		const std::vector<CChainSegment> &v = *pvParts[p];
		for(size_t i=0;i<v.size();i++){
			bool bLast = (i+1==v.size());
			if(!bLast && (v[i].GetType()==CHAIN_SEGMENT_TOOL_SPINDLE_NOSE || v[i].GetType()==CHAIN_SEGMENT_TABLE))
				MY_THROW(CGeneratorException,"segment '"+v[i].GetName()+"': a "+v[i].GetTypeName()
					+" segment may appear only once, at the end of its part");
			if(v[i].IsParameterConstrained() && v[i].m_dParameterValueMinimum>v[i].m_dParameterValueMaximum)
				MY_THROW(CGeneratorException,"segment '"+v[i].GetName()+"': the minimum of the parameter is above the maximum");
		}
	}
}

void CChain::AddSegmentAfterActiveSegment(const CChainSegment & oSegment)
{
	/*
	assert(pvPart == &m_vToolPart || pvPart == &m_vMaterialHolderPart);
	if(nNewIndex<0)
		nNewIndex=-1;
	if(nNewIndex > pvPart->size())
		nNewIndex= pvPart->size();
	vector<CChainSegment>::iterator it=pvPart->begin();
	for(int i=-1; i<= (int)pvPart->size();i++){
		if(i==(int)nNewIndex)
			break;
		else 
			it++;
	}
	pvPart->insert(it,oSegment);
//*/
	std::vector<CChainSegment> &vPart = GetActivePart();
	std::vector<CChainSegment>::iterator it=vPart.begin();
	for(size_t i=0; i<vPart.size();i++){
		if((int)i==m_nActiveSegmentIndex || m_nActiveSegmentIndex<0)
			break;
		else 
			it++;
	}
	vPart.insert(it,oSegment);


}
void CChain::SetVirtualToolPosition(const CR3Vector &r3v, const CR3Point &r3p)
{
	if( r3v.IsZero() )
		return;
	m_toolPosition.m_v =  (r3v.GetUnitVector()) * 0.5;
	m_toolPosition.m_p = r3p;

}
CChainSegment * CChain::GetActiveSegment()
{
	std::vector<CChainSegment> &vPart = GetActivePart();
	if(m_nActiveSegmentIndex<0 || (size_t)m_nActiveSegmentIndex >= vPart.size())
		return NULL;
	return & vPart[m_nActiveSegmentIndex];
}

void CChain::DeleteActiveSegment()
{
//	assert(pvPart == &m_vToolPart || pvPart == &m_vMaterialHolderPart);
//	assert(nSegmentIndex>=0 && nSegmentIndex < pvPart->size());
	if(m_nActiveSegmentIndex<0)
		return;
	std::vector<CChainSegment> &vPart = GetActivePart();
	std::vector<CChainSegment>::iterator it=vPart.begin();
	for(size_t i=0; i<=vPart.size();i++){
		if((int)i==m_nActiveSegmentIndex)
			break;
		else 
			it++;
	}
	vPart.erase(it);
}

void CChain::GetTranslationAxesAndOffset(
		CR3Vector &r3vT1,
		CR3Vector &r3vT2,
		CR3Vector &r3vT3,
		CR3Point &r3pP1, 
		const CChainConfiguration & oConfig) const
{
	CTransformation m4x4F = m_vMaterialHolderPart.back().GetInvertedConstantTransformation();
	size_t k=0;
	bool bWasT1=false, bWasT2=false;
	for(size_t i=m_vMaterialHolderPart.size()-1; i>0; i--,k++){
		if(m_vMaterialHolderPart[i-1].GetType()==CHAIN_SEGMENT_PRISMATIC_LINK){
			if(bWasT1) {
				if(bWasT2){
					r3vT3 = (m4x4F * r3vOZ);					
				} else {
					r3vT2 = (m4x4F * r3vOZ);
					bWasT2 = true;
				}
			} else {
				r3vT1 = (m4x4F * r3vOZ);
				bWasT1 = true;
			}
			m4x4F = m4x4F * m_vMaterialHolderPart[i-1].GetInvertedConstantTransformation();
		} else {
			m4x4F = m4x4F * m_vMaterialHolderPart[i-1].GetInvertedParametrizedTransformation(oConfig[k].GetValue());
		}		
	}
	for(size_t i = 0; i<m_vToolPart.size()-1;i++,k++){
		if(m_vToolPart[i].GetType() == CHAIN_SEGMENT_PRISMATIC_LINK){
			m4x4F = m4x4F * m_vToolPart[i].GetConstantTransformation();
			if(bWasT1) {
				if(bWasT2){
					r3vT3 = (m4x4F * r3vOZ);					
				} else {
					r3vT2 = (m4x4F * r3vOZ);
					bWasT2 = true;
				}
			} else {
				r3vT1 = (m4x4F * r3vOZ);
				bWasT1 = true;
			}
		} else {
			m4x4F = m4x4F * m_vToolPart[i].GetParametrizedTransformation(oConfig[k].GetValue());
		}		
	}
	m4x4F = m4x4F * m_vToolPart.back().GetConstantTransformation() * GetToolMountToToolPointTransformation();
	r3pP1 = m4x4F * CR3Point(0.0);

}
CChainSegment & CChain::operator[](int nIndex)
{
	if(nIndex<-1 || nIndex > (int)this->GetNumberOfRegularLinks())
		MY_THROW(CArgsMyException,"Invalid index of segment in the chain");
	if(nIndex<=(int)m_vMaterialHolderPart.size()-2)
		return m_vMaterialHolderPart[m_vMaterialHolderPart.size()-(size_t)nIndex-2];
	return m_vToolPart[(size_t)nIndex-m_vMaterialHolderPart.size()+1];
}


const CChainSegment & CChain::operator[](int nIndex) const
{
	if(nIndex<-1 || nIndex > (int)this->GetNumberOfRegularLinks())
		MY_THROW(CArgsMyException,"Invalid index of segment in the chain");
	if(nIndex<=(int)m_vMaterialHolderPart.size()-2)
		return m_vMaterialHolderPart[m_vMaterialHolderPart.size()-(size_t)nIndex-2];
	return m_vToolPart[(size_t)nIndex-m_vMaterialHolderPart.size()+1];
}
void CChain::GetTranslationsIndexes(size_t &nFirst, size_t &nSecond, size_t &nThird) const
{
	assert(Is5CChain());
	bool bWasFirst = false, bWasSecond = false;
	for(size_t i=0; i<5; i++){
		if((*this)[(int)i].GetType()==CHAIN_SEGMENT_PRISMATIC_LINK){
			if(bWasFirst){
				if(bWasSecond){
					nThird = i;
					return;
				} else {
					nSecond = i;
					bWasSecond=true;
				}
			} else {
				nFirst= i;
				bWasFirst=true;
			}
		}
	}
}

void CChain::GetRotationsIndexes(size_t &nFirst, size_t &nSecond) const
{
	assert(Is5CChain());
	bool bWasFirst = false;
	for(size_t i=0; i<5; i++){
		if((*this)[(int)i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
			if(bWasFirst){
					nSecond = i;
					return;
			} else {
				nFirst= i;
				bWasFirst=true;
			}
		}
	}
}


size_t CChain::GetFirstRotaryParameterIndex(CHAIN_PART nCountFrom) const
{
	if(!Is5CChain())
		MY_THROW(CGeneratorException,"The first rotary axis is not defined: this is not a 5C kinematic chain");
	size_t nFirst = 0, nSecond = 0;
	GetRotationsIndexes(nFirst,nSecond);
	//the parameters run from the material terminal through the base to the tool terminal (operator[]),
	//so the rotary axis nearest the tool is the second of the two and the one nearest the part the first
	return (nCountFrom==CHAIN_PART_SPINDLE) ? nSecond : nFirst;
}

CTrajectoryStart CChain::GetTrajectoryStart(CHAIN_PART nCountFrom
	,TRAJECTORY_START_CRITERION nCriterion, const CChainConfiguration *pHint) const
{
	CTrajectoryStart oStart;
	oStart.m_nCriterion = nCriterion;
	oStart.m_pHint = pHint;
	oStart.m_nParameterIndex = GetFirstRotaryParameterIndex(nCountFrom);
	return oStart;
}

CTransformation CChain::GetActualToMaterialInvertedTransformation() const
{
	CTransformation m4x4Result(1.0);
	for(size_t i = m_vMaterialHolderPart.size(); i>0; i--){
		m4x4Result = m4x4Result * 
			m_vMaterialHolderPart[i-1].GetInvertedParametrizedTransformation(
				m_vMaterialHolderPart[i-1].m_dActualParameterValue);
	}
	return m4x4Result;
}

CTransformation CChain::GetActualToToolTransformation() const
{
	CTransformation m4x4Result(1.0);
	for(size_t i = 0; i< m_vToolPart.size(); i++){
		m4x4Result = m4x4Result * 
			m_vToolPart[i].GetParametrizedTransformation(
				m_vToolPart[i].m_dActualParameterValue);
	}
	return m4x4Result * GetToolMountToToolPointTransformation();
}

double CChain::GetToolMountToToolPointDistance() const
{
	return m_dToolMountToToolPoint;
}

void CChain::SetToolMountToToolPointDistance(double dDistance)
{
	m_dToolMountToToolPoint = dDistance;
}

CTransformation CChain::GetToolMountToToolPointTransformation() const
{
	return CTransformation(CTranslation(0.0,0.0,-m_dToolMountToToolPoint));
}

CTransformation CChain::GetToolPointToToolMountTransformation() const
{
	return CTransformation(CTranslation(0.0,0.0,m_dToolMountToToolPoint));
}

bool CChain::HasDefaultToolHolder() const
{
	return !m_vToolPart.empty() && m_vToolPart.back().IsToolHolderDeclared();
}

bool CChain::HasDefaultWorkpieceHolder() const
{
	return !m_vMaterialHolderPart.empty() && m_vMaterialHolderPart.back().IsWorkpieceHolderDeclared();
}

const CWorkpieceHolder & CChain::GetDefaultWorkpieceHolder() const
{
	static const CWorkpieceHolder oNone;
	return m_vMaterialHolderPart.empty() ? oNone : m_vMaterialHolderPart.back().GetWorkpieceHolder();
}

void CChain::SetDefaultWorkpieceHolder(const CWorkpieceHolder &oHolder)
{
	if(!m_vMaterialHolderPart.empty())
		m_vMaterialHolderPart.back().SetWorkpieceHolder(oHolder);
}

bool CChain::HasDefaultTool() const
{
	return !m_vToolPart.empty() && m_vToolPart.back().GetTool().IsGiven();
}

const CToolDescription & CChain::GetDefaultTool() const
{
	static const CToolDescription oNone;
	return m_vToolPart.empty() ? oNone : m_vToolPart.back().GetTool();
}

void CChain::SetDefaultTool(const CToolDescription &oTool)
{
	if(!m_vToolPart.empty())
		m_vToolPart.back().SetTool(oTool);
}

const CToolHolder & CChain::GetDefaultToolHolder() const
{
	static const CToolHolder oNone;
	return m_vToolPart.empty() ? oNone : m_vToolPart.back().GetToolHolder();
}

void CChain::SetDefaultToolHolder(const CToolHolder &oHolder)
{
	if(m_vToolPart.empty())
		MY_THROW(CGeneratorException,"the chain has no TOOL segment to clamp a tool holder to");
	m_vToolPart.back().SetToolHolder(oHolder);
}

void CChain::SetToolAssembly(const CMillingTool &oTool)
{
	m_dToolMountToToolPoint = HasDefaultToolHolder() ? oTool.GetMountToToolPointDistance() : 0.0;
}

CR3AnchoredVector CChain::GetActualToolPositionAndOrientation() const
{
	CTransformation t = GetActualToMaterialInvertedTransformation() 
		* GetActualToToolTransformation();
	return CR3AnchoredVector(t*r3vOZ,t*r3pZero);
}


CTransformation CChain::GetFirstConstantRotation(size_t &nFirstRotaryLinkIndex) const
{
	nFirstRotaryLinkIndex=0;
	if(!Is5CChain())
		MY_THROW(CGeneratorException, "Chain is not valid 5C kinematic chain");
	CTransformation res(1.0);
	for(size_t i = m_vMaterialHolderPart.size() ; i>0; i--){
		if(m_vMaterialHolderPart[i-1].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
			nFirstRotaryLinkIndex=m_vMaterialHolderPart.size()-i-1;
			return res;
		}
		res = res * m_vMaterialHolderPart[i-1].GetInvertedConstantRotation();
	}
	for( size_t i=0; i< m_vToolPart.size(); i++){
		res = res * m_vToolPart[i].GetConstantRotation();
		if(m_vToolPart[i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
			nFirstRotaryLinkIndex = m_vMaterialHolderPart.size()+i-1;
			return res;
		}

	}

	//to get rid of warning  C4715
	return CTransformation(1.0);
}
CTransformation CChain::GetMiddleConstantRotation() const
{
	if(!Is5CChain())
		MY_THROW(CGeneratorException, "Chain is not valid 5C kinematic chain");
	CTransformation res(1.0);
	bool bWasFirstParametricRotation = false;
	for(size_t i = m_vMaterialHolderPart.size() ; i>0 ; i--){
		if(bWasFirstParametricRotation){
			if(m_vMaterialHolderPart[i-1].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				return res;
			}
			res = res * m_vMaterialHolderPart[i-1].GetInvertedConstantRotation();
		} else {
			if(m_vMaterialHolderPart[i-1].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				bWasFirstParametricRotation = true;
				res = res * m_vMaterialHolderPart[i-1].GetInvertedConstantRotation();
			}
		}
	}
	for( size_t i=0; i< m_vToolPart.size(); i++){
		if(bWasFirstParametricRotation){
			res = res * m_vToolPart[i].GetConstantRotation();
			if(m_vToolPart[i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				return res;
			}
		} else {
			if(m_vToolPart[i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				bWasFirstParametricRotation = true;
			}
		}
	}
	//to get rid of warning C4715
	return CTransformation(1.0);
}

CTransformation CChain::GetMiddleConstantInvertedRotation() const
{
	if(!Is5CChain())
		MY_THROW(CGeneratorException, "Chain is not valid 5C kinematic chain");
	CTransformation res(1.0);
	bool bWasFirstParametricRotation = false;
	for(size_t i = m_vMaterialHolderPart.size() ; i>0 ; i--){
		if(bWasFirstParametricRotation){
			if(m_vMaterialHolderPart[i-1].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				return res;
			}
			res = m_vMaterialHolderPart[i-1].GetConstantRotation() * res;
		} else {
			if(m_vMaterialHolderPart[i-1].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				bWasFirstParametricRotation = true;
				res = m_vMaterialHolderPart[i-1].GetConstantRotation() * res;
			}
		}
	}
	for( size_t i=0; i< m_vToolPart.size(); i++){
		if(bWasFirstParametricRotation){
			res = m_vToolPart[i].GetInvertedConstantRotation() * res;
			if(m_vToolPart[i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				return res;
			}
		} else {
			if(m_vToolPart[i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
				bWasFirstParametricRotation = true;
			}
		}
	}
	//to get rid of warning C4715
	return CTransformation(1.0);
}


CTransformation CChain::GetLastConstantRotation(size_t &nLastRotaryLinkIndex) const
{
	if(!Is5CChain())
		MY_THROW(CGeneratorException, "Chain is not valid 5C kinematic chain");
	CTransformation res(1.0);
	for(size_t i = m_vToolPart.size() ; i>0; i--){
		if(m_vToolPart[i-1].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
			nLastRotaryLinkIndex = (m_vMaterialHolderPart.size()-1)+(i-1);
			return res;
		}
		res = m_vToolPart[i-1].GetConstantRotation() * res;
	}
	for( size_t i=0; i < m_vMaterialHolderPart.size() ; i++){
		res = m_vMaterialHolderPart[i].GetInvertedConstantRotation() * res;
		if(m_vMaterialHolderPart[i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
			nLastRotaryLinkIndex = m_vMaterialHolderPart.size()-i-2;
			return res;
		}

	}

	//to get rid of warning C4715
	return CTransformation(1.0);
}


void CChain::ZeroAllParameters(){

	for(size_t i=m_vMaterialHolderPart.size(); i>0; i--){
		m_vMaterialHolderPart[i-1].SetParameterValue(0.0);
	}
	for(size_t i = 0; i<m_vToolPart.size();i++){
		m_vToolPart[i].SetParameterValue(0.0);
	}
}
CChainConfiguration	CChain::GetParameters() const
{
	return CChainConfiguration( m_oActualParameters );
}	
void CChain::SetParameters(const CChainConfiguration & oConfig){
	if(oConfig.GetParametersCount()!= (m_vMaterialHolderPart.size()+m_vToolPart.size()-2))
		MY_THROW(CArgsMyException,"Invalid count of parameters in ChainConfiguration object");
	m_oActualParameters = oConfig;
	size_t k=0;
	for(size_t i=m_vMaterialHolderPart.size()-1; i>0; i--,k++){
		m_vMaterialHolderPart[i-1].SetParameterValue(oConfig[k].GetValue());
	}
	//k--;
	for(size_t i = 0; i<m_vToolPart.size()-1;i++,k++){
		m_vToolPart[i].SetParameterValue(oConfig[k].GetValue());
	}
}


void CChain::SetActiveSegment(CHAIN_PART part, int nActiveSegmentIndex)
{
	m_nActivePart = part;
	if(nActiveSegmentIndex >= (int)GetActivePart().size())
		nActiveSegmentIndex = (int)GetActivePart().size()-1;
	if(nActiveSegmentIndex<0)
		m_nActiveSegmentIndex = -1;
	m_nActiveSegmentIndex = nActiveSegmentIndex;
}

 std::vector<CChainSegment> & CChain::GetActivePart() 
{
	return GetPart(m_nActivePart);
}

std::vector<CChainSegment> & CChain::GetPart(CHAIN_PART part)
{
	if(part==CHAIN_PART_SPINDLE)
		return m_vToolPart;
	return m_vMaterialHolderPart;
}

const std::vector<CChainSegment> & CChain::GetPart(CHAIN_PART part) const
{
	if(part==CHAIN_PART_SPINDLE)
		return m_vToolPart;
	return m_vMaterialHolderPart;
}

std::vector<CSegmentFrame> CChain::GetActualFrames(CHAIN_PART part) const
{
	const std::vector<CChainSegment> &vPart = GetPart(part);
	std::vector<CSegmentFrame> vFrames;
	vFrames.reserve(vPart.size());
	CTransformation trInput(1.0);		//the frame of the machine base
	for(size_t i=0;i<vPart.size();i++){
		CSegmentFrame oFrame;
		//the same two factors GetParametrizedTransformation multiplies, kept apart: the boxes of the
		//segment live between them
		oFrame.m_trJoint = trInput * vPart[i].GetConstantTransformation();
		oFrame.m_trOutput = oFrame.m_trJoint
			* vPart[i].GetVariablePartTransformation(vPart[i].m_dActualParameterValue);
		vFrames.push_back(oFrame);
		trInput = oFrame.m_trOutput;
	}
	return vFrames;
}

CSegmentFrame CChain::GetActualFrame(CHAIN_PART part, size_t nIndex) const
{
	const std::vector<CChainSegment> &vPart = GetPart(part);
	if(nIndex>=vPart.size())
		MY_THROW(CArgsMyException,"No segment "+size_to_string(nIndex)+" in this part of the chain: it has "
			+size_to_string(vPart.size())+" of them");
	CTransformation trInput(1.0);
	CSegmentFrame oFrame;
	for(size_t i=0;i<=nIndex;i++){
		oFrame.m_trJoint = trInput * vPart[i].GetConstantTransformation();
		oFrame.m_trOutput = oFrame.m_trJoint
			* vPart[i].GetVariablePartTransformation(vPart[i].m_dActualParameterValue);
		trInput = oFrame.m_trOutput;
	}
	return oFrame;
}

//Brings the state derived from the actual parameters up to date (to be called before rendering):
// - the parameters vector is resized to the number of regular links (after the chain was edited),
// - the colliding parts of the segments are marked and the collision log of the tool is filled,
// - the milling tool is placed at the end of the tool part.
void CChain::Update(CMillingTool *pTool)
{
	if(m_oActualParameters.GetParametersCount()!=GetNumberOfRegularLinks())
		SetParameters(CChainConfiguration((int)this->GetNumberOfRegularLinks()));

	if(pTool){
		SetToolAssembly(*pTool);
		DetectCollisions(*pTool);
		pTool->SetPositionAndOrientation(GetActualToolPositionAndOrientation());
	}
}

void CChain::SetName(const std::string &sName)
{
	m_sName = sName;
}

std::string CChain::GetName() const
{
	return m_sName;
}

void CChain::SetMachine(const std::string &sMachine)
{
	m_sMachine = sMachine;
}

std::string CChain::GetMachine() const
{
	return m_sMachine;
}

void CChain::SetAuthor(const std::string &sAuthor)
{
	m_sAuthor = sAuthor;
}

std::string CChain::GetAuthor() const
{
	return m_sAuthor;
}

void CChain::SetModified(const std::string &sModified)
{
	m_sModified = sModified;
}

std::string CChain::GetModified() const
{
	return m_sModified;
}

void CChain::SetFileName(const std::string &sName)
{
	m_sFileName= std::string(sName);
}

std::string CChain::GetFileName() const
{
	return m_sFileName;
}

void CChain::SetUnit(CHAIN_UNIT unit)
{
	m_nUnit = unit;
}

CHAIN_UNIT CChain::GetUnit() const
{
	return m_nUnit;
}

double CChain::GetUnitInMillimetres() const
{
	return ChainUnitInMillimetres(m_nUnit);
}

//the parameters of the chain are numbered from the base: the material holder part backwards
//(its segment i has the parameter size-2-i), then the tool part forwards (see SetParameters)
double CChain::GetActualParameter(size_t nIndex, bool bToolPart) const
{
	
	if(bToolPart){
		assert(nIndex<m_vToolPart.size());	
		if(nIndex>=m_vToolPart.size()-1)
			return 0.0;
		return m_oActualParameters[m_vMaterialHolderPart.size()-1+nIndex].GetValue();

	} else {
		assert(nIndex<m_vMaterialHolderPart.size());
		if(nIndex>=m_vMaterialHolderPart.size()-1)
			return 0.0;
		return m_oActualParameters[m_vMaterialHolderPart.size()-2-nIndex].GetValue();
	}


}
CR3Box CChain::GetBoundingBox() const
{
	std::list<CR3Point> lstPoints;
	CR3Point p;
	CR3Box box;
	CTransformation tr;
	for(size_t i = 0 ; i< m_vToolPart.size(); i++){
		tr.ZeroRotation(1.0);
		tr.ZeroTranslation();
		for(size_t j=0;j<=i;j++)
			tr = tr* m_vToolPart[j].GetConstantTransformation();
		p = tr*CR3Point(0.0);
		box.ExpandToPoint(p);
	}
	p.SetPoint(0.0);
	for( size_t i=0; i < m_vMaterialHolderPart.size() ; i++){
		tr.ZeroRotation(1.0);
		tr.ZeroTranslation();
		for(size_t j=0;j<=i;j++)
			tr = tr* m_vMaterialHolderPart[j].GetConstantTransformation();
		p = tr*CR3Point(0.0);
		box.ExpandToPoint(p);
	}
	return box;
}

//an empty name becomes S<index>; a name equal to the name of an earlier segment gets "_1" appended
//until it differs from all of them
void CChain::CheckAndCorrectSegmentsNames(void)
{
	int n = (int)GetNumberOfRegularLinks();
	for(int i=0;i<n;i++){
		if((*this)[i].GetName().empty())
			((*this)[i]).SetName(string_printf("S%d",i));
	}
	for(int i=0;i<n;i++){
		for(int j=0;j<i;j++){
			if((*this)[i].GetName()==(*this)[j].GetName()){
				((*this)[i]).SetName((*this)[i].GetName()+"_1");
				j=-1;	//the new name against all the earlier segments again
			}
		}
	}
}
size_t CChain::GetNumberOfRegularLinks() const
{
	return GetNumberOfPrismaticLinks()+GetNumberOfRevoluteLinks();
}



CHAIN_CONSTRAINTS_RESULT CChain::MeetsParametersConstraints(const CChainConfiguration &oChainConfig,std::string &sExceededConstraints) const
{
	CHAIN_CONSTRAINTS_RESULT result=CHAIN_CONSTRAINTS_MET;
	for(int i =0; i<(int)GetNumberOfRegularLinks();i++){
		if((*this)[i].MeetsConstraints(oChainConfig[i].GetValue())){
			continue;
		}else{
			if((*this)[i].GetType()==CHAIN_SEGMENT_PRISMATIC_LINK){
				if(result==CHAIN_CONSTRAINTS_ROTATIONS_EXCEEDED || result == CHAIN_CONSTRAINTS_SHIFTS_AND_ROTATION_EXCEEDED)
					result = CHAIN_CONSTRAINTS_SHIFTS_AND_ROTATION_EXCEEDED;
				else
					result = CHAIN_CONSTRAINTS_SHIFTS_EXCEEDED;
			} else {
				if(result == CHAIN_CONSTRAINTS_SHIFTS_EXCEEDED || result == CHAIN_CONSTRAINTS_SHIFTS_AND_ROTATION_EXCEEDED)
					result = CHAIN_CONSTRAINTS_SHIFTS_AND_ROTATION_EXCEEDED;
				else
					result = CHAIN_CONSTRAINTS_ROTATIONS_EXCEEDED;
			}
			sExceededConstraints+=((*this)[i].GetConstraint()+"; ");
		}
	}
	return result;
}
bool CChain::HasCollision(CMillingTool &oMillingTool,std::string &sLog)
{
	DetectCollisions(oMillingTool);

	std::string s="";
	std::string s2;
	for(int i=-1; i<(int)GetNumberOfRegularLinks()+1; i++)
		if((*this)[i].HasPartInCollision(s2))
			s+=("\t"+s2+"\n");
	if(oMillingTool.GetCollisionLog().size())
		s+=("\t"+oMillingTool.GetCollisionLog()+"\n");
	if(s.length()){
		sLog=("Chain parameters "+m_oActualParameters.toString()+"\n"+s);	
		return true;
	}
	return false;
}

//see below
std::string SolidName(const CChainSegment &oSegment, size_t nBox, size_t nBoxes
	,CHAIN_PART part, size_t nIndex);

const CCollisionReport &CChain::GetCollisions() const
{
	return m_oCollisions;
}

std::string CChain::GetSolidName(CHAIN_PART part, size_t nSegment, size_t nBox) const
{
	const std::vector<CChainSegment> &vPart = GetPart(part);
	if(nSegment>=vPart.size())
		MY_THROW(CArgsMyException,"No segment "+size_to_string(nSegment)+" in this part of the chain: it has "
			+size_to_string(vPart.size())+" of them");
	return SolidName(vPart[nSegment],nBox,vPart[nSegment].GetPartVector().size(),part,nSegment);
}

//The name of one solid of a segment, the way a report reads it: the name the file gives the segment,
//or its place in its branch when it has none, and the number of the box when the segment has more
//than one.
std::string SolidName(const CChainSegment &oSegment, size_t nBox, size_t nBoxes
	,CHAIN_PART part, size_t nIndex)
{
	std::string sName = oSegment.GetName();
	if(sName.empty())
		sName = std::string(part==CHAIN_PART_SPINDLE ? "tool segment " : "material segment ")
			+size_to_string(nIndex+1);
	if(nBoxes>1)
		sName += (" box "+size_to_string(nBox+1));
	return sName;
}

void CChain::DetectCollisions(CMillingTool &oMillingTool)
{
	for(int i = -1; i<=(int)this->GetNumberOfRegularLinks();i++)
		(*this)[i].ClearAllPartCollision();

	m_oCollisions.Clear();
	oMillingTool.ClearCollisionLog();
	//material holder part
	for(size_t i=0;i<m_vMaterialHolderPart.size();i++){
		const std::vector<CR3Box> &boxes = m_vMaterialHolderPart[i].GetPartVector();
		if(boxes.empty())
			continue;
		// material part <-> material part
		CTransformation trVariable = m_vMaterialHolderPart[i].GetVariablePartTransformation(m_vMaterialHolderPart[i].m_dActualParameterValue);
		CTransformation trVariableInv = m_vMaterialHolderPart[i].GetInvertedVariablePartTransformation(m_vMaterialHolderPart[i].m_dActualParameterValue);
		for(size_t k=i+1; k<m_vMaterialHolderPart.size();k++){
			const std::vector<CR3Box> &boxes2 = m_vMaterialHolderPart[k].GetPartVector();
			if(boxes2.empty())
				continue;
			CTransformation tr = trVariable;
			CTransformation trInv = trVariableInv;
			for(size_t l = i+1;l<k;l++){
				tr = tr * m_vMaterialHolderPart[l].GetParametrizedTransformation(m_vMaterialHolderPart[l].m_dActualParameterValue);
				trInv = m_vMaterialHolderPart[l].GetInvertedParametrizedTransformation(m_vMaterialHolderPart[l].m_dActualParameterValue) * trInv;
			}
			tr = tr * m_vMaterialHolderPart[k].GetConstantTransformation();
			trInv = m_vMaterialHolderPart[k].GetInvertedConstantTransformation() * trInv;
			for(size_t j=0; j<boxes.size();j++){
				for(size_t m=0;m<boxes2.size();m++){
					if(boxes[j].HasIntersectionWithBox(boxes2[m],tr,trInv)){
						m_vMaterialHolderPart[i].SetPartCollision(j);
						m_vMaterialHolderPart[k].SetPartCollision(m);
						m_oCollisions.Add(SolidName(m_vMaterialHolderPart[i],j,boxes.size(),CHAIN_PART_TABLE,i)
							,SolidName(m_vMaterialHolderPart[k],m,boxes2.size(),CHAIN_PART_TABLE,k)
							,COLLIDING_PARTS);
					}
				}
			}
		}
		// material part <-> tool part
		CTransformation trToI(1.0); 
		CTransformation trToIInv(1.0) ;
        for(size_t n=0;n<i;n++){
			trToIInv = trToIInv * m_vMaterialHolderPart[n].GetParametrizedTransformation(m_vMaterialHolderPart[n].m_dActualParameterValue);
			trToI = m_vMaterialHolderPart[n].GetInvertedParametrizedTransformation(m_vMaterialHolderPart[n].m_dActualParameterValue) * trToI;
		}
		//trToI: from the base frame to the frame of the boxes of segment i (after its constant transformation)
		trToI = m_vMaterialHolderPart[i].GetInvertedConstantTransformation() * trToI;
		trToIInv =trToIInv * m_vMaterialHolderPart[i].GetConstantTransformation();
		for(size_t k=0; k<m_vToolPart.size();k++){
			const std::vector<CR3Box> &boxes2 = m_vToolPart[k].GetPartVector();
			if(boxes2.empty() && m_vToolPart[k].GetType()!=CHAIN_SEGMENT_TOOL_SPINDLE_NOSE)
				continue;	//a TOOL segment without boxes still carries the milling tool (KnownBugs #38)
			CTransformation tr = trToI;
			CTransformation trInv = trToIInv;
			for(size_t l = 0; l<k; l++){
				tr = tr * m_vToolPart[l].GetParametrizedTransformation(m_vToolPart[l].m_dActualParameterValue);
				trInv = m_vToolPart[l].GetInvertedParametrizedTransformation(m_vToolPart[l].m_dActualParameterValue) * trInv;
			}
			tr = tr * m_vToolPart[k].GetConstantTransformation();
			trInv = m_vToolPart[k].GetInvertedConstantTransformation() * trInv;
			for(size_t j=0; j<boxes.size();j++){
				for(size_t m=0;m<boxes2.size();m++){
					if(boxes[j].HasIntersectionWithBox(boxes2[m],tr,trInv)){
						m_vMaterialHolderPart[i].SetPartCollision(j);
						m_vToolPart[k].SetPartCollision(m);
						m_oCollisions.Add(SolidName(m_vMaterialHolderPart[i],j,boxes.size(),CHAIN_PART_TABLE,i)
							,SolidName(m_vToolPart[k],m,boxes2.size(),CHAIN_PART_SPINDLE,k)
							,COLLIDING_PARTS);
					}
				}
			}
			if(m_vToolPart[k].GetType()==CHAIN_SEGMENT_TOOL_SPINDLE_NOSE){
				//the solids of the tool are measured from the tool point, which hangs below the mount
				CTransformation trTool = tr * GetToolMountToToolPointTransformation();
				CTransformation trToolInv = GetToolPointToToolMountTransformation() * trInv;
				for(size_t j=0; j<boxes.size();j++){
					if(oMillingTool.CauseCollision(boxes[j],trTool,trToolInv)){
						m_vMaterialHolderPart[i].SetPartCollision(j);
						oMillingTool.AddToCollisionLog((m_vMaterialHolderPart[i].GetName()+", box_"+size_to_string(j+1)));
						m_oCollisions.Add("the tool"
							,SolidName(m_vMaterialHolderPart[i],j,boxes.size(),CHAIN_PART_TABLE,i)
							,COLLIDING_TOOL);
					} else if(oMillingTool.CauseNonCuttingCollision(boxes[j],trTool,trToolInv)){
						m_vMaterialHolderPart[i].SetPartCollision(j);
						oMillingTool.AddToCollisionLog((m_vMaterialHolderPart[i].GetName()+", box_"+size_to_string(j+1)+" (holder or shank)"));
						m_oCollisions.Add("the shank or the holder"
							,SolidName(m_vMaterialHolderPart[i],j,boxes.size(),CHAIN_PART_TABLE,i)
							,COLLIDING_TOOL_SHANK);
					}
				}
			}
		}
	}
	// tool part <-> tool part
	for(size_t i = 0; i<m_vToolPart.size();i++){
		const std::vector<CR3Box> &boxes = m_vToolPart[i].GetPartVector();
		if(boxes.empty())
			continue;
		CTransformation trVariable = m_vToolPart[i].GetVariablePartTransformation(m_vToolPart[i].m_dActualParameterValue);
		CTransformation trVariableInv = m_vToolPart[i].GetInvertedVariablePartTransformation(m_vToolPart[i].m_dActualParameterValue);
		for(size_t k=i+1; k<m_vToolPart.size();k++){
			const std::vector<CR3Box> &boxes2 = m_vToolPart[k].GetPartVector();
			if(boxes2.empty() && m_vToolPart[k].GetType()!=CHAIN_SEGMENT_TOOL_SPINDLE_NOSE)
				continue;	//a TOOL segment without boxes still carries the milling tool (KnownBugs #38)
			CTransformation tr = trVariable;
			CTransformation trInv = trVariableInv;
			for(size_t l = i+1;l<k;l++){
				tr = tr * m_vToolPart[l].GetParametrizedTransformation(m_vToolPart[l].m_dActualParameterValue);
				trInv = m_vToolPart[l].GetInvertedParametrizedTransformation(m_vToolPart[l].m_dActualParameterValue) * trInv;
			}
			tr = tr * m_vToolPart[k].GetConstantTransformation();
			trInv = m_vToolPart[k].GetInvertedConstantTransformation() * trInv;
			for(size_t j=0; j<boxes.size();j++){
				for(size_t m=0;m<boxes2.size();m++){
					if(boxes[j].HasIntersectionWithBox(boxes2[m],tr,trInv)){
						m_vToolPart[i].SetPartCollision(j);
						m_vToolPart[k].SetPartCollision(m);
						m_oCollisions.Add(SolidName(m_vToolPart[i],j,boxes.size(),CHAIN_PART_SPINDLE,i)
							,SolidName(m_vToolPart[k],m,boxes2.size(),CHAIN_PART_SPINDLE,k)
							,COLLIDING_PARTS);
					}
				}
			}
			if(m_vToolPart[k].GetType()==CHAIN_SEGMENT_TOOL_SPINDLE_NOSE){
				CTransformation trTool = tr * GetToolMountToToolPointTransformation();
				CTransformation trToolInv = GetToolPointToToolMountTransformation() * trInv;
				for(size_t j=0; j<boxes.size();j++){
					if(oMillingTool.CauseCollision(boxes[j],trTool,trToolInv)){
						m_vToolPart[i].SetPartCollision(j);
						oMillingTool.AddToCollisionLog((m_vToolPart[i].GetName()+", box_"+size_to_string(j+1)));
						m_oCollisions.Add("the tool",SolidName(m_vToolPart[i],j,boxes.size(),CHAIN_PART_SPINDLE,i)
							,COLLIDING_TOOL);
					} else if(oMillingTool.CauseNonCuttingCollision(boxes[j],trTool,trToolInv)){
						m_vToolPart[i].SetPartCollision(j);
						oMillingTool.AddToCollisionLog((m_vToolPart[i].GetName()+", box_"+size_to_string(j+1)+" (holder or shank)"));
						m_oCollisions.Add("the shank or the holder"
							,SolidName(m_vToolPart[i],j,boxes.size(),CHAIN_PART_SPINDLE,i),COLLIDING_TOOL_SHANK);
					}
				}
			}

		}
	}
}


void CChain::ComputeAndSetError(CChainConfiguration &oConfig, const CR3AnchoredVector &cv)
{
	CChainConfiguration old = m_oActualParameters;
	SetParameters(oConfig);
	CR3AnchoredVector cvComputed = GetActualToolPositionAndOrientation();
	oConfig.SetErrors(d_E(cv.m_p,cvComputed.m_p),(cv.m_v.GetUnitVector()-cvComputed.m_v).GetLength());
	SetParameters(old);    
}

} // namespace kin5ax
