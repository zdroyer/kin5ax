#include "../stdafx.h"
#include "Chain.h"
#include "string_tools.h"


namespace kin5ax {

extern CR3Vector r3vOZ;
extern CR3Point r3pZero;


CChain::CChain(void)
{
	m_dToolMountToToolPoint = 0.0;
	m_nActivePart=CHAIN_PART_TOOL;
	m_nActiveSegmentIndex=-1;

	SetActiveSegment(CHAIN_PART_MATERIAL_HOLDER,-1);
	AddSegmentAfterActiveSegment(CChainSegment(CHAIN_SEGMENT_MATERIAL_HOLDER,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0,0.0,0.0)));

	SetActiveSegment(CHAIN_PART_TOOL,-1);
	AddSegmentAfterActiveSegment(CChainSegment(CHAIN_SEGMENT_TOOL,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0,0.0,0.5)));
	m_sFileName = "<chain not saved>";
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
std::string CChain::toXML() const
{
	std::string result;
	result += "<?xml version=\"1.0\"?>\n";
	result += "<!DOCTYPE "+std::string(CHAIN_XML_ROOT_NODE_TYPE_NAME)+" [\n";
//	result += "<!ENTITY %% segment_type \"(REVOLUTE_LINK | PRISMATIC_LINK | TOOL | MATERIAL_HOLDER)\">\n";
	result += "\n";
	result += "<!ELEMENT kinematic_chain (tool_part, material_holder_part)>\n";
	result += "<!ATTLIST kinematic_chain\n";
	result += "	name CDATA #IMPLIED\n";
	result += "	machine CDATA #IMPLIED\n";
	result += "	author CDATA #IMPLIED\n";
	result += "	modified CDATA #IMPLIED\n";
	result += ">\n";
	result += "<!ELEMENT tool_part (segment+)>\n";
	result += "<!ELEMENT material_holder_part (segment+)>\n";
	result += "<!ELEMENT segment (initial_translation, initial_rotation, tool_holder?, box*)>\n";
	result += "<!ATTLIST segment\n";
	result += "	type CDATA #REQUIRED\n";
	result += "	name CDATA #REQUIRED\n";
//	result += "	parameter_value_offset CDATA #REQUIRED\n";
//	result += "	default_parameter_value CDATA #REQUIRED\n";
	result += "	parameter_value_minimum CDATA #IMPLIED\n";
	result += "	parameter_value_maximum CDATA #IMPLIED\n";
	result += ">\n";
	result += "<!ELEMENT tool_holder EMPTY>\n";
	result += "<!ATTLIST tool_holder\n";
	result += "	length CDATA #REQUIRED\n";
	result += "	radius CDATA #REQUIRED\n";
	result += ">\n";
	result += "<!ELEMENT box EMPTY>\n";
	result += "<!ATTLIST box\n";
	result += "	x1 CDATA #REQUIRED\n";
	result += "	x2 CDATA #REQUIRED\n";
	result += "	y1 CDATA #REQUIRED\n";
	result += "	y2 CDATA #REQUIRED\n";
	result += "	z1 CDATA #REQUIRED\n";
	result += "	z2 CDATA #REQUIRED\n";
	result += ">\n";
	result += "<!ELEMENT initial_translation EMPTY>\n";
	result += "<!ATTLIST initial_translation\n";
	result += "	x CDATA #REQUIRED\n";
	result += "	y CDATA #REQUIRED\n";
	result += "	z CDATA #REQUIRED\n";
	result += ">\n";
	result += "<!ELEMENT initial_rotation EMPTY>\n";
	result += "<!ATTLIST initial_rotation\n";
	result += "	x CDATA #REQUIRED\n";
	result += "	y CDATA #REQUIRED\n";
	result += "	z CDATA #REQUIRED\n";
	result += "	alpha CDATA #REQUIRED\n";
	result += ">\n";
	result += "]>\n";
	result += "<"+std::string(CHAIN_XML_ROOT_NODE_TYPE_NAME);
	if(!m_sName.empty())
		result += " name=\""+xml_escape(m_sName)+"\"";
	if(!m_sMachine.empty())
		result += " machine=\""+xml_escape(m_sMachine)+"\"";
	if(!m_sAuthor.empty())
		result += " author=\""+xml_escape(m_sAuthor)+"\"";
	if(!m_sModified.empty())
		result += " modified=\""+xml_escape(m_sModified)+"\"";
	result += ">\n";
	result += "<tool_part>\n";
	for(size_t i=0; i<m_vToolPart.size(); i++){
		result += m_vToolPart[i].toXML();
	}
	result += "</tool_part>\n";
	result += "<material_holder_part>\n";
	for(size_t i=0; i<m_vMaterialHolderPart.size(); i++){
		result += m_vMaterialHolderPart[i].toXML();
	}
	result += "</material_holder_part>\n";
	result +="</"+std::string(CHAIN_XML_ROOT_NODE_TYPE_NAME)+">\n";
	return result;
}
void CChain::CheckStructure() const
{
	if(m_vToolPart.empty())
		MY_THROW(CGeneratorException,"the tool part has no segment");
	if(m_vMaterialHolderPart.empty())
		MY_THROW(CGeneratorException,"the material holder part has no segment");
	if(m_vToolPart.back().GetType()!=CHAIN_SEGMENT_TOOL)
		MY_THROW(CGeneratorException,"the last segment of the tool part must be the TOOL segment, '"
			+m_vToolPart.back().GetName()+"' is a "+m_vToolPart.back().GetTypeName());
	if(m_vMaterialHolderPart.back().GetType()!=CHAIN_SEGMENT_MATERIAL_HOLDER)
		MY_THROW(CGeneratorException,"the last segment of the material holder part must be the MATERIAL_HOLDER segment, '"
			+m_vMaterialHolderPart.back().GetName()+"' is a "+m_vMaterialHolderPart.back().GetTypeName());
	const std::vector<CChainSegment> *pvParts[2] = {&m_vToolPart,&m_vMaterialHolderPart};
	for(size_t p=0;p<2;p++){
		const std::vector<CChainSegment> &v = *pvParts[p];
		for(size_t i=0;i<v.size();i++){
			bool bLast = (i+1==v.size());
			if(!bLast && (v[i].GetType()==CHAIN_SEGMENT_TOOL || v[i].GetType()==CHAIN_SEGMENT_MATERIAL_HOLDER))
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

bool CChain::IsToolMountDeclared() const
{
	return !m_vToolPart.empty() && m_vToolPart.back().IsToolHolderDeclared();
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
	m_dToolMountToToolPoint = IsToolMountDeclared() ? oTool.GetMountToToolPointDistance() : 0.0;
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
	if(part==CHAIN_PART_TOOL)
		return m_vToolPart;
	return m_vMaterialHolderPart;
}

const std::vector<CChainSegment> & CChain::GetPart(CHAIN_PART part) const
{
	if(part==CHAIN_PART_TOOL)
		return m_vToolPart;
	return m_vMaterialHolderPart;
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

void CChain::DetectCollisions(CMillingTool &oMillingTool)
{
	for(int i = -1; i<=(int)this->GetNumberOfRegularLinks();i++)
		(*this)[i].ClearAllPartCollision();

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
			if(boxes2.empty() && m_vToolPart[k].GetType()!=CHAIN_SEGMENT_TOOL)
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
					}
				}
			}
			if(m_vToolPart[k].GetType()==CHAIN_SEGMENT_TOOL){
				//the solids of the tool are measured from the tool point, which hangs below the mount
				CTransformation trTool = tr * GetToolMountToToolPointTransformation();
				CTransformation trToolInv = GetToolPointToToolMountTransformation() * trInv;
				for(size_t j=0; j<boxes.size();j++){
					if(oMillingTool.CauseCollision(boxes[j],trTool,trToolInv)){
						m_vMaterialHolderPart[i].SetPartCollision(j);
						oMillingTool.AddToCollisionLog((m_vMaterialHolderPart[i].GetName()+", box_"+size_to_string(j+1)));
					} else if(oMillingTool.CauseNonCuttingCollision(boxes[j],trTool,trToolInv)){
						m_vMaterialHolderPart[i].SetPartCollision(j);
						oMillingTool.AddToCollisionLog((m_vMaterialHolderPart[i].GetName()+", box_"+size_to_string(j+1)+" (holder or shank)"));
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
			if(boxes2.empty() && m_vToolPart[k].GetType()!=CHAIN_SEGMENT_TOOL)
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
					}
				}
			}
			if(m_vToolPart[k].GetType()==CHAIN_SEGMENT_TOOL){
				CTransformation trTool = tr * GetToolMountToToolPointTransformation();
				CTransformation trToolInv = GetToolPointToToolMountTransformation() * trInv;
				for(size_t j=0; j<boxes.size();j++){
					if(oMillingTool.CauseCollision(boxes[j],trTool,trToolInv)){
						m_vToolPart[i].SetPartCollision(j);
						oMillingTool.AddToCollisionLog((m_vToolPart[i].GetName()+", box_"+size_to_string(j+1)));
					} else if(oMillingTool.CauseNonCuttingCollision(boxes[j],trTool,trToolInv)){
						m_vToolPart[i].SetPartCollision(j);
						oMillingTool.AddToCollisionLog((m_vToolPart[i].GetName()+", box_"+size_to_string(j+1)+" (holder or shank)"));
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
