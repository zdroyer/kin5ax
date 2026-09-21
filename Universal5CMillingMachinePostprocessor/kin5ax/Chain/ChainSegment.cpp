#include "../StdAfx.h"
#include "chainsegment.h"
#include "string_tools.h"
#include "rotation.h"
#include "translation.h"
#include "r3box.h"

extern CR3Vector r3vOZ;

CChainSegment::CChainSegment(CHAIN_SEGMENT_TYPE type
					,const CR3Vector &r3vConstRevoluteAxis
					,const double & dConstRevoluteAngle
					,const CR3Vector &r3vConstTranslation
					,bool bParameterIsConstrained
					,double dParameterMinimum
					,double dParameterMaximum)
		:m_type(type)
		,m_r3vConstRevoluteAxis(r3vConstRevoluteAxis)
		,m_dConstRevoluteAngle(dConstRevoluteAngle)
		,m_r3vConstTranslation(r3vConstTranslation)
		,m_dActualParameterValue(dParameterMinimum)
		,m_bParameterIsConstrained(bParameterIsConstrained)
		,m_dParameterValueMaximum(dParameterMaximum)
		,m_dParameterValueMinimum(dParameterMinimum)
		,m_nHighlightedPart(-1)
{
	SetType(type);
}

string CChainSegment::GetTypeName() const
{

	return m_sTypeName;
}



CChainSegment::~CChainSegment(void)
{

}

CTransformation CChainSegment::GetConstantTransformation() const
{
	return CTranslation(m_r3vConstTranslation)*(CRotation(m_r3vConstRevoluteAxis,m_dConstRevoluteAngle,0.0));
}
CTransformation CChainSegment::GetInvertedConstantTransformation() const
{
	return CRotation(m_r3vConstRevoluteAxis,-m_dConstRevoluteAngle,0.0)*CTranslation(-m_r3vConstTranslation);

}
CRotation CChainSegment::GetConstantRotation() const
{
	return CRotation(m_r3vConstRevoluteAxis,m_dConstRevoluteAngle,0.0);
}

CRotation CChainSegment::GetInvertedConstantRotation() const
{
	return CRotation(m_r3vConstRevoluteAxis,-m_dConstRevoluteAngle,0.0);
}

bool CChainSegment::HasPartInCollision(string &sLog) const
{
	if(m_vParts.empty())
		return false;
	if(m_vParts.size()!= m_vCollisions.size()){
		return false;
	}
	string s = "";
	for(size_t i=0; i<m_vCollisions.size();i++){
		if(m_vCollisions[i]){
			if(i)
				s+=", ";
			s+=("box_"+size_to_string(i+1));
		}
	}
	if(s.length()){
		sLog="detected collision in segment '"+GetName()+"', parts: "+s;
		return true;
	}
	return false;

}

CTransformation CChainSegment::GetVariablePartTransformation(double dParam) const
{
	if(m_type == CHAIN_SEGMENT_REVOLUTE_LINK)
		return CRotation::OZRotation(dParam);
	if(m_type == CHAIN_SEGMENT_PRISMATIC_LINK)
		return (CTranslation(0.0,0.0,dParam));
	return CTransformation(1.0);

}
CTransformation CChainSegment::GetInvertedVariablePartTransformation(double dParam) const
{
	if(m_type == CHAIN_SEGMENT_REVOLUTE_LINK)
		return CRotation::OZRotation(-dParam);
	if(m_type == CHAIN_SEGMENT_PRISMATIC_LINK)
		return (CTranslation(0.0,0.0,-dParam));
	return CTransformation(1.0);
}

CTransformation CChainSegment::GetParametrizedTransformation(double dParam) const
{
	if(m_type == CHAIN_SEGMENT_REVOLUTE_LINK)
		return GetConstantTransformation() * CRotation(r3vOZ,dParam);
	if(m_type == CHAIN_SEGMENT_PRISMATIC_LINK)
		return GetConstantTransformation()*(CTranslation(0.0,0.0,dParam));
	return GetConstantTransformation();
}
CTransformation CChainSegment::GetInvertedParametrizedTransformation(double dParam) const
{
	if(m_type == CHAIN_SEGMENT_REVOLUTE_LINK)
		return CRotation(r3vOZ,-dParam)*GetInvertedConstantTransformation();
	if(m_type == CHAIN_SEGMENT_PRISMATIC_LINK)
		return CTranslation(0.0,0.0,-dParam) * GetInvertedConstantTransformation() ;
	return GetInvertedConstantTransformation();

}

bool CChainSegment::IsParameterConstrained() const
{
	return m_bParameterIsConstrained;
}

bool CChainSegment::MeetsConstraints(double dParam) const
{
	if(!m_bParameterIsConstrained)
		return true;
	if(m_dParameterValueMinimum-dEPSILON>dParam
		|| m_dParameterValueMaximum+dEPSILON<dParam)
		return false;
	return true;

}

string CChainSegment::GetConstraint() const
{
	string sResult;
	if(m_bParameterIsConstrained){
		sResult=GetName()+" in ["+double_to_string(m_dParameterValueMinimum)+"; "+double_to_string(m_dParameterValueMaximum)+"]";
	} else {
		sResult="no constraint";
	}
	return sResult;
}

bool CChainSegment::IsRegularJoint() const
{
	return (m_type == CHAIN_SEGMENT_PRISMATIC_LINK 
		|| m_type == CHAIN_SEGMENT_REVOLUTE_LINK);

}

void CChainSegment::SetParameterValue(double dValue)
{
	/*
	if(m_bParameterIsConstrained){
		if(dValue<m_dParameterValueMinimum || dValue >m_dParameterValueMaximum)
			MY_THROW(CArgsMyException, "Invalid value of chain segment parameter");
	}
	//*/
	m_dActualParameterValue = dValue;	
}


double string_to_double(const string &s)
{
	double d;
	sscanf(s.c_str(),"%lG",&d);
	return d;
}



CHAIN_SEGMENT_TYPE CChainSegment::GetType() const
{
	return m_type;
}

void CChainSegment::SetType(CHAIN_SEGMENT_TYPE type)
{
	m_type=type;
	switch (type) {
		case CHAIN_SEGMENT_REVOLUTE_LINK:
			m_sTypeName="REVOLUTE_LINK";
			break;
		case CHAIN_SEGMENT_PRISMATIC_LINK: 
			m_sTypeName="PRISMATIC_LINK";
			break;
		case CHAIN_SEGMENT_MATERIAL_HOLDER:
			m_sTypeName="MATERIAL_HOLDER";
			break ;
		case CHAIN_SEGMENT_TOOL:
			m_sTypeName="TOOL";
			break; 
		default:
			MY_THROW(CGeneratorException, "Undefined chain segment type");
	}
	
}

void CChainSegment::SetType(const string &sType)
{
	if(0==strcmp("REVOLUTE_LINK",sType.c_str())){
		SetType(CHAIN_SEGMENT_REVOLUTE_LINK);
		return;
	}
	if(0==strcmp("PRISMATIC_LINK",sType.c_str())){
		SetType(CHAIN_SEGMENT_PRISMATIC_LINK);
		return;
	}
	if(0==strcmp("MATERIAL_HOLDER",sType.c_str())){
		SetType(CHAIN_SEGMENT_MATERIAL_HOLDER);
		return;
	}
	if(0==strcmp("TOOL",sType.c_str())){
		SetType(CHAIN_SEGMENT_TOOL);
		return;
	}
	MY_THROW(CGeneratorException, "Undefined chain segment type");
}


void CChainSegment::SetName(const string &sName)
{
	m_sName = sName;
}

string CChainSegment::GetName() const
{
	if(IsRegularJoint())
		return m_sName;
	if(m_type == CHAIN_SEGMENT_MATERIAL_HOLDER)
		return "Material holder";
	return "Tool holder";

}

string CChainSegment::toXML(void) const
{
	string sResult;
	sResult += "<segment type=\""+GetTypeName();
	sResult += "\" name=\""+GetName()+"\" " ;
	if(m_bParameterIsConstrained){
		if(m_type==CHAIN_SEGMENT_PRISMATIC_LINK){
			sResult += "parameter_value_minimum=\""+double_to_string(m_dParameterValueMinimum)+"\" ";
			sResult += "parameter_value_maximum=\""+double_to_string(m_dParameterValueMaximum)+"\" ";
		} else {
			sResult += "parameter_value_minimum=\""+double_to_string(radians_to_degrees(m_dParameterValueMinimum))+"\" ";
			sResult += "parameter_value_maximum=\""+double_to_string(radians_to_degrees(m_dParameterValueMaximum))+"\" ";

		}
	}
	sResult += ">\n";
	sResult += "<initial_translation x=\"";
	sResult += double_to_string(m_r3vConstTranslation.GetX());
	sResult += "\" y=\"";
	sResult += double_to_string(m_r3vConstTranslation.GetY());
	sResult += "\" z=\"";
	sResult += double_to_string(m_r3vConstTranslation.GetZ());
	sResult += "\"/>\n";

	sResult +="<initial_rotation x=\"";
	sResult +=double_to_string(m_r3vConstRevoluteAxis.GetX());
	sResult +="\" y=\"";
	sResult +=double_to_string(m_r3vConstRevoluteAxis.GetY());
	sResult +="\" z=\"";
	sResult +=double_to_string(m_r3vConstRevoluteAxis.GetZ());
	sResult +="\" alpha=\"";
	sResult +=double_to_string(m_dConstRevoluteAngle);
	sResult +="\"/>\n";
	if(m_vParts.size()){
		for(size_t i=0;i<m_vParts.size();i++){
			sResult+=m_vParts[i].toXML();
		}
	}
	sResult += "</segment>\n";	
	return sResult;
}


void CChainSegment::SetPartVector(vector<CR3Box> vNewParts)
{
	m_vParts = vNewParts;
	m_vCollisions = vector<bool>(vNewParts.size(),false);
}
const vector<CR3Box> &CChainSegment::GetPartVector() const
{
	return m_vParts;
}
void CChainSegment::HighlightPart(int n) 
{
	if(n<0 || n>=(int)m_vParts.size())
		m_nHighlightedPart=-1;
	else
		m_nHighlightedPart = n;
}

void CChainSegment::SetPartCollision(size_t n)
{
	if(n>m_vCollisions.size())
		return;
	m_vCollisions[n]=true;
}

void CChainSegment::ClearAllPartCollision()
{
	m_vCollisions = vector<bool>(m_vCollisions.size(),false);
}
CR3Box *CChainSegment::GetActivePartPointer()
{
	if(m_nHighlightedPart<0 || m_nHighlightedPart>=(int)m_vParts.size())
		return 0;
	return &m_vParts[m_nHighlightedPart];
}
bool CChainSegment::IsPartInCollision(size_t n) const
{
	if(n>=m_vCollisions.size())
		return false;
	return m_vCollisions[n];
}
int CChainSegment::GetHighlightedPartIndex() const
{
	return m_nHighlightedPart;
}