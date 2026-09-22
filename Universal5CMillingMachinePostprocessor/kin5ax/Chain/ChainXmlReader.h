#pragma once

#include "Chain.h"

//Reads a kinematic chain from its XML (.kc) file.
//This header is free of any XML library: the implementation (ChainXmlReader.cpp) is the only place
//in kin5ax that uses one - MSXML 6 on Windows, with the inline DTD of the .kc files validated.
//CChain and CChainSegment do not depend on it; writing is CChain::toXML.
class CChainXmlReader
{
public:
	//throws CGeneratorException: file not readable, XML error (with line and position), invalid chain
	static CChain ReadFile(const string &sFilePath);
};
