#pragma once

#include "chain\chain.h"

//Reads a kinematic chain from its XML (.kc) representation with MSXML.
//CChain and CChainSegment themselves do not depend on MSXML/COM; writing is CChain::toXML.
class CChainXmlReader
{
public:
	static CChain ReadChain(MSXML2::IXMLDOMElementPtr pDocElement, const string &sFileName);
protected:
	static CChainSegment ReadSegment(MSXML2::IXMLDOMNodePtr pNode);
};
