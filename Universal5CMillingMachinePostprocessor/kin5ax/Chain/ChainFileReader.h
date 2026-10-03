#pragma once

#include "Chain.h"

//Reads a kinematic chain from either of its files, .kc (XML, CChainXmlReader) or .kc.json (JSON,
//CChainJsonReader), and decides itself which one it has in front of it: by the extension when it
//is ".kc" or ".json" (any case), else by the first non-blank character of the file ('{' is JSON,
//anything else XML). This is what the application opens a chain with.
namespace kin5ax {

class CChainFileReader
{
public:
	enum FORMAT { FORMAT_XML, FORMAT_JSON };
	//the format of the file: by its extension, or by its content when the extension says nothing;
	//a file that cannot be read is FORMAT_XML (the XML reader then reports the file)
	static FORMAT FormatOf(const std::string &sFilePath);
	//throws CGeneratorException with the file name and the place of every problem
	static CChain ReadFile(const std::string &sFilePath);
};

} // namespace kin5ax
