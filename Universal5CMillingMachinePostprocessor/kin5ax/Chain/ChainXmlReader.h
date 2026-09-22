#pragma once

#include "Chain.h"

//Reads a kinematic chain from its XML (.kc) file; writing is CChain::toXML.
//This header is free of any XML library. Two readers with the same interface:
//  CChainXmlReader        MSXML 6 through COM (ChainXmlReader.cpp, Windows only), the inline DTD
//                         of the file validated by the parser
//  CChainXmlReaderPugixml pugixml (ChainXmlReaderPugixml.cpp, portable); no DTD: the elements,
//                         attributes and numbers are checked by the reader itself
//Both end with CChain::CheckStructure (the terminal segments). CChain and CChainSegment do not
//depend on any of it.
namespace kin5ax {

class CChainXmlReader
{
public:
	//throws CGeneratorException: file not readable, XML error (with line and position), invalid chain
	static CChain ReadFile(const std::string &sFilePath);
};

class CChainXmlReaderPugixml
{
public:
	//throws CGeneratorException with the file name and the line of every problem
	static CChain ReadFile(const std::string &sFilePath);
};

} // namespace kin5ax
