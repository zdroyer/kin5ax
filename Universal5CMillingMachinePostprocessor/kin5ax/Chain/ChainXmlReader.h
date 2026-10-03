#pragma once

#include "Chain.h"

//Reads a kinematic chain from its XML (.kc) file; writing is CChain::toXML.
//This header is free of any XML library: pugixml (vendored in 3rdParty/pugixml) is included by
//ChainXmlReader.cpp alone. No DTD validation: the elements, attributes and numbers are checked by
//the reader itself, then CChain::CheckStructure checks the terminal segments.
//CChain and CChainSegment do not depend on any of it.
namespace kin5ax {

class CChainXmlReader
{
public:
	//throws CGeneratorException with the file name and the line of every problem
	static CChain ReadFile(const std::string &sFilePath);
};

//the former name of the portable reader, until the application stops choosing a reader
typedef CChainXmlReader CChainXmlReaderPugixml;

} // namespace kin5ax
