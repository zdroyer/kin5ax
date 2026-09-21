#include "../stdafx.h"
#include "TestData.h"
#include "../ChainXmlReader.h"

static const char cSEPARATOR = 92;	//backslash

static bool FileExists(const string &sPattern)
{
	WIN32_FIND_DATA oData;
	HANDLE hFind = FindFirstFile(sPattern.c_str(),&oData);
	if(hFind==INVALID_HANDLE_VALUE)
		return false;
	FindClose(hFind);
	return true;
}

static void AppendFiles(const string &sFolder, const string &sPattern, vector<string> &vFiles)
{
	WIN32_FIND_DATA oData;
	HANDLE hFind = FindFirstFile((sFolder+sPattern).c_str(),&oData);
	if(hFind==INVALID_HANDLE_VALUE)
		return;
	do{
		if(!(oData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			vFiles.push_back(sFolder+oData.cFileName);
	}while(FindNextFile(hFind,&oData));
	FindClose(hFind);
}

string GetSolutionDir()
{
	char szPath[MAX_PATH];
	GetModuleFileName(NULL,szPath,MAX_PATH);
	string sFolder(szPath);
	//the exe is in <solution>/x64/Debug or in <solution>/Debug
	for(int i=0;i<4;i++){
		size_t n = sFolder.find_last_of(cSEPARATOR);
		if(n==string::npos)
			break;
		sFolder = sFolder.substr(0,n);
		if(FileExists(sFolder+cSEPARATOR+"*.sln"))
			return sFolder+cSEPARATOR;
	}
	return "";
}

vector<string> GetChainFiles()
{
	vector<string> vFiles;
	string sSolutionDir = GetSolutionDir();
	AppendFiles(sSolutionDir,"*.kc",vFiles);
	AppendFiles(sSolutionDir+"TestChains"+cSEPARATOR,"*.kc",vFiles);
	sort(vFiles.begin(),vFiles.end());
	return vFiles;
}

string GetTestName(const string &sFilePath)
{
	size_t nBegin = sFilePath.find_last_of(cSEPARATOR);
	nBegin = (nBegin==string::npos) ? 0 : nBegin+1;
	size_t nEnd = sFilePath.find_last_of(46);	//dot
	string sName = sFilePath.substr(nBegin,(nEnd==string::npos || nEnd<nBegin) ? string::npos : nEnd-nBegin);
	for(size_t i=0;i<sName.size();i++)
		if(!isalnum((unsigned char)sName[i]))
			sName[i]=95;	//underscore
	return sName;
}

CChain LoadChain(const string &sFilePath)
{
	//the same settings as CUniversal5CMillingMachinePostprocessorDoc::OnOpenDocument
	MSXML2::IXMLDOMDocument2Ptr pDOMDoc;
	if(FAILED(pDOMDoc.CreateInstance(__uuidof(DOMDocument60))))
		MY_THROW(CGeneratorException,"Cannot create MSXML 6 document");
	pDOMDoc->put_async(VARIANT_FALSE);
	pDOMDoc->put_validateOnParse(VARIANT_TRUE);
	pDOMDoc->setProperty(_bstr_t("ProhibitDTD"), _variant_t(false));
	if(VARIANT_FALSE == pDOMDoc->load(_variant_t(_bstr_t(sFilePath.c_str()))))
		MY_THROW(CGeneratorException,"XML parse error: "+string((char*)pDOMDoc->GetparseError()->Getreason()));
	return CChainXmlReader::ReadChain(pDOMDoc->GetdocumentElement(),sFilePath);
}
