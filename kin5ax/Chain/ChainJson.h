#pragma once

#include "Chain.h"

//Reads and writes the JSON form of a kinematic chain (*.kc.json), described by kin5ax/kc.schema.json.
//This header is free of any JSON library: nlohmann/json is included by ChainJson.cpp alone.
//
//The JSON form is not a transcription of the XML one:
//  - the terminals are members of their branch ("tool_spindle_nose", "table"), not entries of the
//    segment list, and carry no type and no limits;
//  - a joint is "rotary" or "linear" (REVOLUTE_LINK / PRISMATIC_LINK of the model);
//  - a branch may have no joint at all ("links": []);
//  - the material holder part of the model is the workpiece branch of the file.
//Angles are degrees in the file as in the XML; the limits of a rotary joint too.
namespace kin5ax {

class CChainJsonReader
{
public:
	//throws CGeneratorException naming the file and the member at fault
	static CChain ReadFile(const std::string &sFilePath);
	//sWhere appears in the messages: a file name, or empty for text held in memory
	static CChain Parse(const std::string &sJson, const std::string &sWhere = std::string());
};

class CChainJsonWriter
{
public:
	//indented with two spaces, one line per vector, ends with a newline
	static std::string ToJson(const CChain &oChain);
	//throws CGeneratorException when the file cannot be written
	static void WriteFile(const CChain &oChain, const std::string &sFilePath);
};

} // namespace kin5ax
