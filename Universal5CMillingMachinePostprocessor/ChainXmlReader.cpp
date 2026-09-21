#include "StdAfx.h"
#include "ChainXmlReader.h"
#include "Atlconv.h"

static double getDoubleValue(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const string &sAttrName)
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


static string getStringValue(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const string &sAttrName)
{
	MSXML2::IXMLDOMNodePtr pAttr;
	pAttr = pMap->getNamedItem(_bstr_t(sAttrName.c_str()));
	VARIANT varValue;
	pAttr->get_nodeValue(&varValue);
//	HRESULT hr = VariantChangeTypeEx(&varValue,&varValue,1033,VARIANT_NOUSEROVERRIDE,VT_LPSTR);
	USES_CONVERSION;
	return OLE2T(varValue.bstrVal);
	
}
static bool doubleValue(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const string &sAttrName, double &d)
{
	MSXML2::IXMLDOMNodePtr pAttr = NULL;
	pAttr = pMap->getNamedItem(_bstr_t(sAttrName.c_str()));
	if(pAttr==NULL)
		return false;
	d = getDoubleValue(pMap,sAttrName);
	return true;
}

CChainSegment CChainXmlReader::ReadSegment(MSXML2::IXMLDOMNodePtr pNode)
{
	assert(pNode!=NULL);
	_bstr_t sNodeName = pNode->GetnodeName();
	if(wcscmp(sNodeName,_bstr_t("segment")) != 0)
		MY_THROW(CGeneratorException,"Invalid type of XML 'segment'node");
	MSXML2::IXMLDOMNamedNodeMapPtr pAttributesMap = pNode->Getattributes();
	//the type and the constant transformation are set below
	CChainSegment oSegment(CHAIN_SEGMENT_TOOL,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0));
	oSegment.SetName(getStringValue(pAttributesMap,"name"));
	oSegment.SetType(getStringValue(pAttributesMap,"type"));
	oSegment.m_dActualParameterValue = 0.0;
	if(doubleValue(pAttributesMap,"parameter_value_minimum",oSegment.m_dParameterValueMinimum) 
		&& doubleValue(pAttributesMap,"parameter_value_maximum",oSegment.m_dParameterValueMaximum)){
	
		oSegment.m_bParameterIsConstrained = true;
		if(oSegment.m_type==CHAIN_SEGMENT_REVOLUTE_LINK){
			oSegment.m_dParameterValueMaximum = degrees_to_radians(oSegment.m_dParameterValueMaximum);
			oSegment.m_dParameterValueMinimum = degrees_to_radians(oSegment.m_dParameterValueMinimum);
		}
	}
	else 
		oSegment.m_bParameterIsConstrained = false;
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
		MY_THROW(CGeneratorException,"Cannot obtain initial transformation of segment");
	pAttributesMap = pTranslationNode->Getattributes();
	oSegment.m_r3vConstTranslation.SetX(getDoubleValue(pAttributesMap,"x"));
	oSegment.m_r3vConstTranslation.SetY(getDoubleValue(pAttributesMap,"y"));
	oSegment.m_r3vConstTranslation.SetZ(getDoubleValue(pAttributesMap,"z"));

	pAttributesMap = pRotationNode->Getattributes();
	oSegment.m_r3vConstRevoluteAxis.SetX(getDoubleValue(pAttributesMap,"x"));
	oSegment.m_r3vConstRevoluteAxis.SetY(getDoubleValue(pAttributesMap,"y"));
	oSegment.m_r3vConstRevoluteAxis.SetZ(getDoubleValue(pAttributesMap,"z"));
	oSegment.m_dConstRevoluteAngle = getDoubleValue(pAttributesMap,"alpha");
	if(is_zero(oSegment.m_r3vConstRevoluteAxis.GetLengthSquare()))
		oSegment.m_r3vConstRevoluteAxis.SetX(1.0);

	vector<CR3Box> vParts;
	for(size_t i=0; i<vpBoxesNodes.size();i++){
		pAttributesMap = vpBoxesNodes[i]->Getattributes();
		double x1 = getDoubleValue(pAttributesMap,"x1");
		double x2 = getDoubleValue(pAttributesMap,"x2");
		double y1 = getDoubleValue(pAttributesMap,"y1");
		double y2 = getDoubleValue(pAttributesMap,"y2");
		double z1 = getDoubleValue(pAttributesMap,"z1");
		double z2 = getDoubleValue(pAttributesMap,"z2");
		vParts.push_back(CR3Box(x1,x2,y1,y2,z1,z2));
	}		
	oSegment.SetPartVector(vParts);
	return oSegment;
}

CChain CChainXmlReader::ReadChain(MSXML2::IXMLDOMElementPtr pDocElement, const string &sFileName)
{
	assert(pDocElement!=NULL);
	_bstr_t sNodeName = pDocElement->GetnodeName();
	_bstr_t sKinematicChain(CHAIN_XML_ROOT_NODE_TYPE_NAME);
	int n = wcscmp(sNodeName,sKinematicChain );
	if(n!=0)
		MY_THROW(CGeneratorException,"Invalid type of XML root node");
	
	//the default segments of a new chain are replaced below
	CChain oChain;
	MSXML2::IXMLDOMNodeListPtr pResultList = pDocElement->getElementsByTagName(_bstr_t("tool_part"));
	long i = pResultList->length;	
	MSXML2::IXMLDOMElementPtr pToolPartNode  = pResultList->item[0];

	MSXML2::IXMLDOMNodeListPtr pNodes = pToolPartNode->GetchildNodes();
	pNodes->reset();
	vector<CChainSegment> segmentVector;
	for(MSXML2::IXMLDOMNodePtr pChildNode = pNodes->nextNode() ; pChildNode != NULL; pChildNode = pNodes->nextNode())
			segmentVector.push_back(ReadSegment(pChildNode));
	oChain.m_vToolPart = segmentVector;

	pResultList = pDocElement->getElementsByTagName(_bstr_t("material_holder_part"));
	MSXML2::IXMLDOMElementPtr pMaterialHolderPartNode  = pResultList->item[0];
	pNodes = pMaterialHolderPartNode->GetchildNodes();
	segmentVector.erase(segmentVector.begin(),segmentVector.end());
	pNodes->reset();
	for(MSXML2::IXMLDOMNodePtr pChildNode = pNodes->nextNode() ; pChildNode != NULL; pChildNode = pNodes->nextNode())
		segmentVector.push_back(ReadSegment(pChildNode));
	oChain.m_vMaterialHolderPart = segmentVector;
	oChain.SetFileName(sFileName);
	//zero parameters for all regular links (SetParameters accepts only a chain ended with the tool and the material holder)
	size_t nLinks = oChain.GetNumberOfRegularLinks();
	if(nLinks+2 == oChain.m_vToolPart.size()+oChain.m_vMaterialHolderPart.size())
		oChain.SetParameters(CChainConfiguration((int)nLinks));
	return oChain;
}
