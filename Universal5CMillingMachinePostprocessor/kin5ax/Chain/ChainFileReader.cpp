#include "../stdafx.h"
#include "ChainFileReader.h"
#include "ChainXmlReader.h"
#include "ChainJson.h"

namespace kin5ax {

namespace {

//true when sText ends with sSuffix, letters compared without case
bool EndsWithNoCase(const std::string &sText, const std::string &sSuffix)
{
	if(sText.size()<sSuffix.size())
		return false;
	for(size_t i=0;i<sSuffix.size();i++)
		if(tolower((unsigned char)sText[sText.size()-sSuffix.size()+i])!=tolower((unsigned char)sSuffix[i]))
			return false;
	return true;
}

//the first character of the file that is neither blank nor part of a UTF-8 byte order mark; 0 when there is none
char FirstVisibleCharacter(const std::string &sFilePath)
{
	FILE *pFile = fopen(sFilePath.c_str(),"rb");
	if(!pFile)
		return 0;
	char szHead[4096];
	size_t n = fread(szHead,1,sizeof(szHead),pFile);
	fclose(pFile);
	size_t i = 0;
	if(n>=3 && (unsigned char)szHead[0]==0xEF && (unsigned char)szHead[1]==0xBB && (unsigned char)szHead[2]==0xBF)
		i = 3;
	for(;i<n;i++)
		if(!isspace((unsigned char)szHead[i]))
			return szHead[i];
	return 0;
}

} // namespace

CChainFileReader::FORMAT CChainFileReader::FormatOf(const std::string &sFilePath)
{
	if(EndsWithNoCase(sFilePath,".json"))
		return FORMAT_JSON;
	if(EndsWithNoCase(sFilePath,".kc"))
		return FORMAT_XML;
	return FirstVisibleCharacter(sFilePath)=='{' ? FORMAT_JSON : FORMAT_XML;
}

CChain CChainFileReader::ReadFile(const std::string &sFilePath)
{
	return FormatOf(sFilePath)==FORMAT_JSON
		? CChainJsonReader::ReadFile(sFilePath)
		: CChainXmlReader::ReadFile(sFilePath);
}

} // namespace kin5ax
