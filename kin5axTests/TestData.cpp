#include "stdafx.h"
#include "TestData.h"
#include "ChainJson.h"
#include <filesystem>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

using namespace kin5ax;

namespace fs = std::filesystem;

//the folder of the test exe; the only platform-dependent query of the tests (the chain files are
//listed while the parameterised tests are instantiated, before main() and its argv[0])
static fs::path ExeDir()
{
#ifdef _WIN32
	char szPath[4096];
	DWORD nLength = GetModuleFileNameA(NULL,szPath,sizeof(szPath));
	return fs::path(std::string(szPath,nLength)).parent_path();
#else
	return fs::read_symlink("/proc/self/exe").parent_path();
#endif
}

std::string GetTestChainsDir()
{
	fs::path oDir = ExeDir()/"TestChains";
	return oDir.string()+(char)fs::path::preferred_separator;
}

//the files of GetTestChainsDir() with the given extension (".json"), sorted
static std::vector<std::string> GetFilesWithExtension(const std::string &sExtension)
{
	std::vector<std::string> vFiles;
	std::error_code oError;	//a missing folder gives an empty list, not an exception
	for(const fs::directory_entry &oEntry : fs::directory_iterator(GetTestChainsDir(),oError))
		if(oEntry.is_regular_file() && oEntry.path().extension()==sExtension)
			vFiles.push_back(oEntry.path().string());
	std::sort(vFiles.begin(),vFiles.end());
	return vFiles;
}

std::vector<std::string> GetChainFiles()
{
	return GetFilesWithExtension(".json");
}

std::string GetTestName(const std::string &sFilePath)
{
	std::string sName = fs::path(sFilePath).stem().string();
	for(size_t i=0;i<sName.size();i++)
		if(!isalnum((unsigned char)sName[i]))
			sName[i]='_';
	return sName;
}

std::string TempFilePath(const std::string &sName)
{
	return (fs::temp_directory_path()/sName).string();
}

CChain LoadChain(const std::string &sFilePath)
{
	return CChainJsonReader::ReadFile(sFilePath);
}
