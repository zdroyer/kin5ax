#include "../stdafx.h"
#include "ChainXmlReader.h"

//the only Windows dependency of kin5ax: MSXML 6 through COM; nothing of it leaves this file
#define NOMINMAX
#include <comdef.h>
#import <msxml6.dll> exclude("ISequentialStream","_FILETIME")

//COM initialised for the lifetime of the object; a thread already initialised in another mode is left as it is
namespace kin5ax {

class CComInitialiser
{
	bool m_bUninitialise;
public:
	CComInitialiser()
	{
		HRESULT hr = CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
		m_bUninitialise = SUCCEEDED(hr);	//S_OK or S_FALSE (already initialised in this mode)
	}
	~CComInitialiser()
	{
		if(m_bUninitialise)
			CoUninitialize();
	}
};

static std::string toString(const _bstr_t &s)
{
	const char *p = s;
	return p ? std::string(p) : std::string();
}

static MSXML2::IXMLDOMNodePtr getAttribute(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const std::string &sAttrName)
{
	MSXML2::IXMLDOMNodePtr pAttr = pMap->getNamedItem(_bstr_t(sAttrName.c_str()));
	if(pAttr==NULL)
		MY_THROW(CGeneratorException,"There is no chain segment attribute: "+sAttrName);
	return pAttr;
}

static double getDoubleValue(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const std::string &sAttrName)
{
	_variant_t varValue;
	getAttribute(pMap,sAttrName)->get_nodeValue(&varValue);
	//locale 1033 (en-US): the decimal separator of the file is the point
	HRESULT hr = VariantChangeTypeEx(&varValue,&varValue,1033,VARIANT_NOUSEROVERRIDE,VT_R8);
	if(FAILED(hr))
		MY_THROW(CGeneratorException,"Cannot obtain chain segment attribute: "+sAttrName);
	return varValue.dblVal;
}

static std::string getStringValue(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const std::string &sAttrName)
{
	_variant_t varValue;
	getAttribute(pMap,sAttrName)->get_nodeValue(&varValue);
	return toString(_bstr_t(varValue));
}

static bool doubleValue(MSXML2::IXMLDOMNamedNodeMapPtr &pMap, const std::string &sAttrName, double &d)
{
	MSXML2::IXMLDOMNodePtr pAttr = pMap->getNamedItem(_bstr_t(sAttrName.c_str()));
	if(pAttr==NULL)
		return false;
	d = getDoubleValue(pMap,sAttrName);
	return true;
}

static bool isNode(const MSXML2::IXMLDOMNodePtr &pNode, const char *szName)
{
	return wcscmp(pNode->GetnodeName(),_bstr_t(szName)) == 0;
}

static CChainSegment ReadSegment(MSXML2::IXMLDOMNodePtr pNode)
{
	assert(pNode!=NULL);
	if(!isNode(pNode,"segment"))
		MY_THROW(CGeneratorException,"Invalid type of XML 'segment' node");
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
	std::vector<MSXML2::IXMLDOMNodePtr> vpBoxesNodes;
	for(MSXML2::IXMLDOMNodePtr pTempNode = pNode->firstChild; pTempNode!=NULL; pTempNode = pTempNode->nextSibling){
		if(isNode(pTempNode,"initial_translation")){
			if(pTranslationNode==NULL)
				pTranslationNode = pTempNode;
		}
		if(isNode(pTempNode,"initial_rotation")){
			if(pRotationNode==NULL)
				pRotationNode = pTempNode;
		}
		if(isNode(pTempNode,"box"))
			vpBoxesNodes.push_back(pTempNode);
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

	std::vector<CR3Box> vParts;
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

//the segments of the <tool_part> or the <material_holder_part> element
static std::vector<CChainSegment> ReadPart(MSXML2::IXMLDOMElementPtr pDocElement, const char *szPartName)
{
	MSXML2::IXMLDOMNodeListPtr pResultList = pDocElement->getElementsByTagName(_bstr_t(szPartName));
	MSXML2::IXMLDOMElementPtr pPartNode = pResultList->item[0];
	if(pPartNode==NULL)
		MY_THROW(CGeneratorException,std::string("There is no '")+szPartName+"' element");
	MSXML2::IXMLDOMNodeListPtr pNodes = pPartNode->GetchildNodes();
	pNodes->reset();
	std::vector<CChainSegment> vSegments;
	for(MSXML2::IXMLDOMNodePtr pChildNode = pNodes->nextNode(); pChildNode != NULL; pChildNode = pNodes->nextNode())
		vSegments.push_back(ReadSegment(pChildNode));
	return vSegments;
}

static CChain ReadChain(MSXML2::IXMLDOMElementPtr pDocElement, const std::string &sFileName)
{
	assert(pDocElement!=NULL);
	if(!isNode(pDocElement,CHAIN_XML_ROOT_NODE_TYPE_NAME))
		MY_THROW(CGeneratorException,"Invalid type of XML root node");

	//the default segments of a new chain are replaced below
	CChain oChain;
	MSXML2::IXMLDOMNamedNodeMapPtr pRootAttributes = pDocElement->Getattributes();
	if(pRootAttributes->getNamedItem(_bstr_t("name"))!=NULL)
		oChain.SetName(getStringValue(pRootAttributes,"name"));
	oChain.m_vToolPart = ReadPart(pDocElement,"tool_part");
	oChain.m_vMaterialHolderPart = ReadPart(pDocElement,"material_holder_part");
	oChain.SetFileName(sFileName);
	try{
		oChain.CheckStructure();
	} catch (CGeneratorException &e){
		MY_THROW(CGeneratorException,sFileName+": "+e.Message());
	}
	//zero parameters for all regular links
	oChain.SetParameters(CChainConfiguration((int)oChain.GetNumberOfRegularLinks()));
	return oChain;
}

//all COM objects live inside this function, so they are released before CoUninitialize
static CChain LoadAndRead(const std::string &sFilePath)
{
	MSXML2::IXMLDOMDocument2Ptr pDOMDoc;
	if(FAILED(pDOMDoc.CreateInstance(__uuidof(MSXML2::DOMDocument60))))
		MY_THROW(CGeneratorException,"Cannot create the MSXML 6 document");
	pDOMDoc->put_async(VARIANT_FALSE);
	pDOMDoc->put_validateOnParse(VARIANT_TRUE);
	//MSXML 6 prohibits DTDs by default; .kc files carry an inline DTD
	pDOMDoc->setProperty(_bstr_t("ProhibitDTD"),_variant_t(false));
	if(VARIANT_FALSE == pDOMDoc->load(_variant_t(_bstr_t(sFilePath.c_str())))){
		MSXML2::IXMLDOMParseErrorPtr pParseError = pDOMDoc->GetparseError();
		char szWhere[64];
		sprintf(szWhere,"line %ld, position %ld: ",pParseError->line,pParseError->linepos);
		MY_THROW(CGeneratorException,"XML error in "+sFilePath+", "+szWhere+toString(pParseError->Getreason()));
	}
	return ReadChain(pDOMDoc->GetdocumentElement(),sFilePath);
}

CChain CChainXmlReader::ReadFile(const std::string &sFilePath)
{
	CComInitialiser oCom;
	try{
		return LoadAndRead(sFilePath);
	} catch (_com_error &e){
		MY_THROW(CGeneratorException,"COM error while reading "+sFilePath+": "+toString(_bstr_t(e.ErrorMessage())));
	}
}

} // namespace kin5ax
