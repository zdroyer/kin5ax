#include "../stdafx.h"
#include "ChainFileReader.h"
#include "ChainJson.h"
#include "GeneratorException.h"

#include <fstream>
#include <algorithm>

namespace kin5ax {

namespace {

std::string LowerExtension(const std::string &sFilePath)
{
	const size_t nDot = sFilePath.find_last_of('.');
	if(nDot==std::string::npos)
		return std::string();
	std::string sExtension = sFilePath.substr(nDot);
	std::transform(sExtension.begin(),sExtension.end(),sExtension.begin()
		,[](unsigned char c){ return (char)tolower(c); });
	return sExtension;
}

} // namespace

bool CChainFileReader::IsRetiredXml(const std::string &sFilePath)
{
	const std::string sExtension = LowerExtension(sFilePath);
	if(sExtension==".json")
		return false;
	if(sExtension==".kc")
		return true;
	//no extension to go by: the first character that is not a space decides, as it always did
	std::ifstream oFile(sFilePath.c_str(),std::ios::binary);
	if(!oFile.is_open())
		return false;					//unreadable: let the JSON reader report the file itself
	char c = 0;
	while(oFile.get(c))
		if(!isspace((unsigned char)c))
			return c=='<';
	return false;
}

CChain CChainFileReader::ReadFile(const std::string &sFilePath)
{
	if(IsRetiredXml(sFilePath))
		MY_THROW(CGeneratorException,sFilePath+": this is a kinematic chain in the XML form (.kc), "
			"which this library no longer reads. Convert it with tools/kc2json and open the "
			".kc.json it writes.");
	return CChainJsonReader::ReadFile(sFilePath);
}

} // namespace kin5ax
