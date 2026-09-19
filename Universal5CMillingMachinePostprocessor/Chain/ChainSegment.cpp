#include "../StdAfx.h"
#include "chainsegment.h"
#include "../gltools.h"
#include "../string_tools.h"
#include "rotation.h"
#include "translation.h"
#include "r3box.h"
#include "Atlconv.h"

extern CR3Vector r3vOZ;
extern CApplicationConfiguration oConfiguration;

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
		,m_bDrawBigFrame(false)
		,m_nHighlightedPart(-1)
{
	SetType(type);
}

string CChainSegment::GetTypeName() const
{

	return m_sTypeName;
}



double getDoubleValue(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const string &sAttrName)
{
	MSXML2::IXMLDOMNodePtr pAttr = NULL;
	pAttr = pMap->getNamedItem(_bstr_t(sAttrName.c_str()));
	if(pAttr==NULL)
		MY_THROW(CGeneratorException,"There is no chain segment attribute: "+sAttrName);
	VARIANT varValue;
	pAttr->get_nodeValue(&varValue);
	HRESULT hr = VariantChangeTypeEx(&varValue,&varValue,1033,VARIANT_NOUSEROVERRIDE,VT_R8);
	if(FAILED(hr))
		MY_THROW(CGeneratorException,"Cannot obtain chain segment attribute: "+sAttrName);
	return varValue.dblVal;
}


string getStringValue(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const string &sAttrName)
{
	MSXML2::IXMLDOMNodePtr pAttr;
	pAttr = pMap->getNamedItem(_bstr_t(sAttrName.c_str()));
	VARIANT varValue;
	pAttr->get_nodeValue(&varValue);
//	HRESULT hr = VariantChangeTypeEx(&varValue,&varValue,1033,VARIANT_NOUSEROVERRIDE,VT_LPSTR);
	USES_CONVERSION;
	return OLE2T(varValue.bstrVal);
	
}
bool doubleValue(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const string &sAttrName, double &d)
{
	MSXML2::IXMLDOMNodePtr pAttr = NULL;
	pAttr = pMap->getNamedItem(_bstr_t(sAttrName.c_str()));
	if(pAttr==NULL)
		return false;
	d = getDoubleValue(pMap,sAttrName);
	return true;
}




CChainSegment::CChainSegment(MSXML2::IXMLDOMNodePtr & pNode)
:m_bDrawBigFrame(false)
	{	
	assert(pNode!=NULL);
	_bstr_t sNodeName = pNode->GetnodeName();
	if(wcscmp(sNodeName,_bstr_t("segment")) != 0)
		MY_THROW(CGeneratorException,"Invalid type of XML 'segment'node");
	MSXML2::IXMLDOMNamedNodeMapPtr pAttributesMap = pNode->Getattributes();
	SetName(getStringValue(pAttributesMap,"name"));
	SetType(getStringValue(pAttributesMap,"type"));
	m_dActualParameterValue = 0.0;
	if(doubleValue(pAttributesMap,"parameter_value_minimum",m_dParameterValueMinimum) 
		&& doubleValue(pAttributesMap,"parameter_value_maximum",m_dParameterValueMaximum)){
	
		m_bParameterIsConstrained = true;
		if(m_type==CHAIN_SEGMENT_REVOLUTE_LINK){
			m_dParameterValueMaximum = degrees_to_radians(m_dParameterValueMaximum);
			m_dParameterValueMinimum = degrees_to_radians(m_dParameterValueMinimum);
		}
	}
	else 
		m_bParameterIsConstrained = false;
	MSXML2::IXMLDOMNodePtr pTranslationNode = NULL, pRotationNode = NULL;
	vector<MSXML2::IXMLDOMNodePtr> vpBoxesNodes;
	MSXML2::IXMLDOMNodePtr pTempNode = pNode->firstChild;
	while(pTempNode!=NULL){
		if(wcscmp(pTempNode->GetnodeName(),_bstr_t("initial_translation")) == 0){
			if(pTranslationNode==0)
				pTranslationNode = pTempNode;

		}
		if(wcscmp(pTempNode->GetnodeName(),_bstr_t("initial_rotation")) == 0) {
			if(pRotationNode==0)
				pRotationNode = pTempNode;
		}
		if(wcscmp(pTempNode->GetnodeName(),_bstr_t("box")) == 0) {
			vpBoxesNodes.push_back(pTempNode);
		}

		pTempNode = pTempNode->nextSibling;
	}
	if(pTranslationNode == NULL || pRotationNode == NULL)
		MY_THROW(CGeneratorException,"Cannot obtain initial transforamtion of segment");
	pAttributesMap = pTranslationNode->Getattributes();
	m_r3vConstTranslation.SetX(getDoubleValue(pAttributesMap,"x"));
	m_r3vConstTranslation.SetY(getDoubleValue(pAttributesMap,"y"));
	m_r3vConstTranslation.SetZ(getDoubleValue(pAttributesMap,"z"));

	pAttributesMap = pRotationNode->Getattributes();
	m_r3vConstRevoluteAxis.SetX(getDoubleValue(pAttributesMap,"x"));
	m_r3vConstRevoluteAxis.SetY(getDoubleValue(pAttributesMap,"y"));
	m_r3vConstRevoluteAxis.SetZ(getDoubleValue(pAttributesMap,"z"));
	m_dConstRevoluteAngle = getDoubleValue(pAttributesMap,"alpha");
	if(is_zero(m_r3vConstRevoluteAxis.GetLengthSquare()))
		m_r3vConstRevoluteAxis.SetX(1.0);

	for(size_t i=0; i<vpBoxesNodes.size();i++){
		pAttributesMap = vpBoxesNodes[i]->Getattributes();
		double x1 = getDoubleValue(pAttributesMap,"x1");
		double x2 = getDoubleValue(pAttributesMap,"x2");
		double y1 = getDoubleValue(pAttributesMap,"y1");
		double y2 = getDoubleValue(pAttributesMap,"y2");
		double z1 = getDoubleValue(pAttributesMap,"z1");
		double z2 = getDoubleValue(pAttributesMap,"z2");
		m_vParts.push_back(CR3Box(x1,x2,y1,y2,z1,z2));
	}		
	m_nHighlightedPart=-1;
	m_vCollisions = vector<bool>(m_vParts.size(),false);
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

void CChainSegment::RenderParts()
{
	glColor3d(0.3,0.3,0.3);
	for(size_t i = 0 ; i<m_vParts.size();i++)
		if(i==m_nHighlightedPart){
			if(m_vCollisions[i]){
				glColor3d(1.0,0.0,0.0);
				m_vParts[i].Render3DSolid();
				glColor3d(0.3,0.3,0.3);
			} else {
				glColor3d(0.6,0.6,0.6);	
				m_vParts[i].Render3DSolid();
				glColor3d(0.3,0.3,0.3);
			}
		} else {
			if(m_vCollisions[i]){
				glColor3d(0.5,0.0,0.0);
				m_vParts[i].Render3DSolid();
				glColor3d(0.3,0.3,0.3);
			} else {
				m_vParts[i].Render3DSolid();
			}
		}


	

}
void CChainSegment::Render3D(bool bDrawFrame)
{
	m_bDrawBigFrame = bDrawFrame;
	DrawArm(m_r3vConstTranslation);
	ApplyConstantTransformation();
	if(oConfiguration.m_bShowParts)
		RenderParts();
	glEnable(GL_LIGHTING);
	switch(m_type){
		case CHAIN_SEGMENT_REVOLUTE_LINK:
			RenderRevolute();
			break;
		case CHAIN_SEGMENT_PRISMATIC_LINK:
			RenderPrismatic();
			break;
		case CHAIN_SEGMENT_MATERIAL_HOLDER:
			RenderMaterialHolder();
			break;
		case CHAIN_SEGMENT_TOOL:
			RenderToolHolder();
			break;
		default:
			return;
	}
}

void CChainSegment:: RenderRevolute()
{
	if(m_bDrawBigFrame)
		CGLTools::DrawAxes(oConfiguration.m_dUnit,oConfiguration.m_dUnit);
	else{
		if(oConfiguration.m_bShowLinksFrames)
			CGLTools::DrawAxes(0.3,0.3*oConfiguration.m_dUnit);
	}
	if(oConfiguration.m_bShowLinks){
		static double *pdU = &oConfiguration.m_dUnit;
		static GLUquadricObj *pQObj = gluNewQuadric();
		glColor3d(0.0,0.5,0.5);
		glNormal3d(0.0,0.0,1.0);
		glTranslated(0.0,0.0,-0.001*(*pdU));
		gluDisk(pQObj,0.01*(*pdU),0.1*(*pdU),40,6);
		glTranslated(0.0,0.0,0.002*(*pdU));
		glNormal3d(0.0,0.0,-1.0);

		gluDisk(pQObj,0.01*(*pdU),0.1*(*pdU),40,6);
		glTranslated(0.0,0.0,-0.001*(*pdU));

		glColor3d(0.3,0.3,0.3);
		glTranslated(0.0,0.0,-0.002*(*pdU));
		glNormal3d(0.0,0.0,1.0);

		gluPartialDisk(pQObj,0.03*(*pdU),0.09*(*pdU),20,4,0.0,radians_to_degrees(-m_dActualParameterValue));
		glTranslated(0.0,0.0,0.004*(*pdU));
		glNormal3d(0.0,0.0,-1.0);

		gluPartialDisk(pQObj,0.03*(*pdU),0.09*(*pdU),20,4,0.0,radians_to_degrees(-m_dActualParameterValue));
		glTranslated(0.0,0.0,-0.002*(*pdU));
	}	
	glRotated(radians_to_degrees( m_dActualParameterValue),0.0,0.0,1.0);
}
void CChainSegment::RenderPrismatic()
{
	if(m_bDrawBigFrame)
		CGLTools::DrawAxes(oConfiguration.m_dUnit,oConfiguration.m_dUnit);
	else{
		if(oConfiguration.m_bShowLinksFrames)
			CGLTools::DrawAxes(0.3,0.3*oConfiguration.m_dUnit);
	}
	if(oConfiguration.m_bShowLinks){
		static double *pdU = &oConfiguration.m_dUnit;
		glColor3d(0.0,0.5,0.5);
		double dArmThicknessDiv2 = 0.05*(*pdU); 
		glDisable(GL_LIGHTING);
		glBegin(GL_LINE_LOOP);
			glVertex3d(-dArmThicknessDiv2,-dArmThicknessDiv2,0);
			glVertex3d(-dArmThicknessDiv2,dArmThicknessDiv2,0);
			glVertex3d(dArmThicknessDiv2,dArmThicknessDiv2,0);
			glVertex3d(dArmThicknessDiv2,-dArmThicknessDiv2,0);
		glEnd();
		glEnable(GL_LIGHTING);
		OZBox(dArmThicknessDiv2,dArmThicknessDiv2,m_dActualParameterValue);
	}
	glTranslated(0.0,0.0,m_dActualParameterValue);
	

}
void CChainSegment::RenderToolHolder()
{
	if(m_bDrawBigFrame || oConfiguration.m_bShowToolFrame)
		CGLTools::DrawAxes(oConfiguration.m_dUnit,oConfiguration.m_dUnit);
	else
		if(oConfiguration.m_bShowLinksFrames)
			CGLTools::DrawAxes(0.3,0.3*oConfiguration.m_dUnit);
	
	return;
	static double *pdU = &oConfiguration.m_dUnit;
	static GLUquadricObj *pQObj = gluNewQuadric();
	//glTranslated(0.0,0.0,-m_dActualParameterValue-m_dParameterValueOffset);
	//gluSphere(pQObj,0.01,10,10);
	glColor3d(1.0,0.0,1.0);
	//static GLUquadricObj *pQObj2 = gluNewQuadric();
	glTranslated(0.0,0.0,-0.2*(*pdU));
	gluCylinder(pQObj,0.02*(*pdU),0.02*(*pdU),0.2*(*pdU),20,10);
	glTranslated(0.0,0.0,0.2*(*pdU));

}
void CChainSegment::RenderMaterialHolder()
{
	if(m_bDrawBigFrame || oConfiguration.m_bShowMaterialFrame){
		CGLTools::DrawAxes(oConfiguration.m_dUnit,oConfiguration.m_dUnit);
	} else {
		if(oConfiguration.m_bShowLinksFrames)
			CGLTools::DrawAxes(0.3,0.3*oConfiguration.m_dUnit);
	}
	glColor3d(0.15,0.15,0.3);
	if(!oConfiguration.m_bShowMaterialHolder)
		return;
	static double *pdU = &oConfiguration.m_dUnit;
	glBegin(GL_POLYGON);
		glNormal3d(0.0,0.0,1.0*(*pdU));
		glVertex3d(-0.5*(*pdU),-0.5*(*pdU),0.0);
		glVertex3d(-0.5*(*pdU),0.5*(*pdU),0.0);
		glVertex3d(0.5*(*pdU),0.5*(*pdU),0.0);
		glNormal3d(0.0,0.0,1.0);
		glVertex3d(0.5*(*pdU),-0.5*(*pdU),0.0);
	glEnd();
	
}
void CChainSegment::DrawArm(const CR3Vector &r3vEndOfArm)
{
	if(oConfiguration.m_bShowLinks){
		glColor3d(1.0,1.0,0.0);
		glBegin(GL_LINES);
			glVertex3dv(CR3Vector(0.0).toGL3dv());
			glVertex3d(r3vEndOfArm.GetX(),r3vEndOfArm.GetY(),r3vEndOfArm.GetZ());
		glEnd();
	}

}
void CChainSegment::ApplyConstantTransformation()
{
glTranslated(m_r3vConstTranslation.GetX()
							,m_r3vConstTranslation.GetY()
							,m_r3vConstTranslation.GetZ());

glRotated((m_dConstRevoluteAngle)
					,m_r3vConstRevoluteAxis.GetX()
					,m_r3vConstRevoluteAxis.GetY()
					,m_r3vConstRevoluteAxis.GetZ());
	
}

void CChainSegment::OZBox(double dXDiv2, double dYDiv2, double dZ)
{
	CR3Box box = CR3Box::BoxFromOZBox(dXDiv2*2.0,dYDiv2*2.0,dZ);
	box.Render3DSolid();
	static double d = 1E-5;
	box.ExpandXYZ(d,d,d);
	box.Render3DWire();	
	return;
	if(is_zero(dZ)){
		static double d = 1E-5;
		glShadeModel(GL_FLAT);
		glBegin(GL_QUADS);
			glNormal3d(0.0,0.0,-1.0);						
			glVertex3d(-dXDiv2,-dYDiv2,-d);
			glVertex3d(dXDiv2,-dYDiv2,-d);
			glVertex3d(dXDiv2,dYDiv2,-d);
			glVertex3d(-dXDiv2,dYDiv2,-d);
			glNormal3d(0.0,0.0,1.0);						
			glVertex3d(-dXDiv2,-dYDiv2,d);
			glVertex3d(dXDiv2,-dYDiv2,d);
			glVertex3d(dXDiv2,dYDiv2,d);
			glVertex3d(-dXDiv2,dYDiv2,d);
		glEnd();
		glShadeModel(GL_SMOOTH);
		return;
	}

	double dSign = 1.0;
	if(dZ<0.0)
		dSign=-1.0;
	glShadeModel(GL_FLAT);
	glBegin(GL_TRIANGLE_FAN);
		glNormal3d(-dSign,0.0,0.0);						
		glVertex3d(-dXDiv2,-dYDiv2,0.0);
		glVertex3d(-dXDiv2,-dYDiv2,dZ);
		glVertex3d(-dXDiv2,dYDiv2,dZ);
		glVertex3d(-dXDiv2,dYDiv2,0.0);
        glNormal3d(0.0,0.0,-dSign);
		glVertex3d(dXDiv2,dYDiv2,0.0);
		glVertex3d(dXDiv2,-dYDiv2,0.0);
		glNormal3d(0.0,-dSign,0.0);
		glVertex3d(dXDiv2,-dYDiv2,dZ);
		glVertex3d(-dXDiv2,-dYDiv2,dZ);			
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glNormal3d(1.0,0.0,0.0);						
		glVertex3d(dXDiv2,dYDiv2,dZ);
		glVertex3d(dXDiv2,dYDiv2,0.0);
		glVertex3d(dXDiv2,-dYDiv2,0.0);
		glVertex3d(dXDiv2,-dYDiv2,dZ);
        glNormal3d(0.0,0.0,dSign);
		glVertex3d(-dXDiv2,-dYDiv2,dZ);
		glVertex3d(-dXDiv2,dYDiv2,dZ);
		glNormal3d(0.0,1.0,0.0);
		glVertex3d(-dXDiv2,dYDiv2,0.0);
		glVertex3d(dXDiv2,dYDiv2,0.0);			
	glEnd();
	glShadeModel(GL_SMOOTH);
	return;
	
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
CR3Box *CChainSegment::GetAvtivePartPointer()
{
	if(m_nHighlightedPart<0 || m_nHighlightedPart>=(int)m_vParts.size())
		return 0;
	return &m_vParts[m_nHighlightedPart];
}
int CChainSegment::GetHighlightedPartIndex() const
{
	return m_nHighlightedPart;
}