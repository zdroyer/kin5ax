/**
 * @file kc2json.cpp
 * @brief Brings an old chain file up to date: the retired XML form, and version 1 of the JSON one.
 *
 * The library reads `.kc.json` in format version 2 and nothing else. This tool is the way out for
 * a file that is older than that, and it takes both kinds:
 *
 *     kc2json <file.kc> [more.kc ...]        the XML form, written beside it as <file>.kc.json
 *     kc2json --force <file.kc.json>         version 1 of the JSON form, rewritten as version 2
 *
 * An existing target is not touched unless `--force` is given - and for a version 1 JSON file the
 * target is the file itself, so that one always needs `--force`.
 *
 * Nothing of the chain is lost either way. Version 1 named the terminals `tool` and
 * `workpiece_holder`; they are `tool_spindle_nose` and `table` now, and the place they sit at has
 * not moved - what changed is that the tool offset is given to a solver instead of being derived
 * from the file.
 */
#include "stdafx.h"
#include "ChainXmlReader.h"
#include "ChainJson.h"
#include "GeneratorException.h"
#include "../../kin5ax/3rdParty/nlohmann/json.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace kin5ax;

namespace {

/// True when the name ends with that ending, whatever the case of either.
bool EndsWith(const std::string &sPath, const std::string &sEnding)
{
	if(sPath.size()<sEnding.size())
		return false;
	for(size_t i=0;i<sEnding.size();i++){
		const char c1 = (char)tolower((unsigned char)sPath[sPath.size()-sEnding.size()+i]);
		const char c2 = (char)tolower((unsigned char)sEnding[i]);
		if(c1!=c2)
			return false;
	}
	return true;
}

/// The name to write beside the source: "AC.kc" becomes "AC.kc.json", a JSON file keeps its own.
std::string JsonNameOf(const std::string &sPath)
{
	return EndsWith(sPath,".kc.json") ? sPath : sPath + ".json";
}

/**
 * Version 1 of the JSON form as version 2: the two terminals renamed and the version bumped.
 *
 * The members themselves are the same - the names, the frames, the holders, the boxes - and the
 * terminals sit where they sat. Version 1 derived the programmed point from the file, by hanging
 * the assembly below the terminal; version 2 leaves that to whoever solves, which is why the
 * terminal is called the nose of the spindle now.
 */
std::string UpgradedFromVersion1(const std::string &sJson, const std::string &sWhere)
{
	typedef nlohmann::ordered_json tJson;
	tJson root;
	try{
		root = tJson::parse(sJson);
	} catch(const std::exception &e){
		MY_THROW(CArgsMyException,std::string("not JSON at all: ")+e.what());
	}
	if(!root.is_object() || !root.contains("version") || !root["version"].is_number_integer())
		MY_THROW(CArgsMyException,"this is not a kinematic chain file: it has no integer version");
	const int nVersion = root["version"].get<int>();
	if(nVersion!=1)
		MY_THROW(CArgsMyException,"this is version "+size_to_string((size_t)nVersion)
			+" of the format, and only version 1 is converted here");
	//ordered_json keeps the order members were inserted in, so a renamed terminal is re-inserted
	//at the end of its branch rather than in place; nothing reads a chain file by position
	const char *szBranches[2] = {"tool_branch","workpiece_branch"};
	const char *szOld[2] = {"tool","workpiece_holder"};
	const char *szNew[2] = {"tool_spindle_nose","table"};
	for(int i=0;i<2;i++){
		if(!root.contains(szBranches[i]) || !root[szBranches[i]].is_object())
			continue;
		tJson &branch = root[szBranches[i]];
		if(!branch.contains(szOld[i]))
			continue;
		branch[szNew[i]] = branch[szOld[i]];
		branch.erase(szOld[i]);
		//the name is free text and is kept, unless it is the one version 1 wrote by default: a
		//terminal called "Tool holder" is the nose the holder hangs from, which is not the same thing
		static const char *szOldName[2] = {"Tool holder","Material holder"};
		static const char *szNewName[2] = {"Spindle nose","Table"};
		tJson &terminal = branch[szNew[i]];
		if(terminal.is_object() && terminal.contains("name") && terminal["name"].is_string()
			&& terminal["name"].get<std::string>()==szOldName[i])
			terminal["name"] = szNewName[i];
	}
	root["version"] = 2;
	//read back through the library, so that what is written is a file the library accepts
	const CChain oChain = CChainJsonReader::Parse(root.dump(2),sWhere);
	return CChainJsonWriter::ToJson(oChain);
}

std::string ReadWholeFile(const std::string &sPath)
{
	std::ifstream oFile(sPath.c_str(), std::ios::binary);
	if(!oFile)
		MY_THROW(CArgsMyException,"cannot be opened for reading");
	std::ostringstream oText;
	oText << oFile.rdbuf();
	return oText.str();
}

void WriteWholeFile(const std::string &sPath, const std::string &sText)
{
	std::ofstream oFile(sPath.c_str(), std::ios::binary);
	if(!oFile)
		MY_THROW(CArgsMyException,"cannot be opened for writing");
	oFile << sText;
	if(!oFile)
		MY_THROW(CArgsMyException,"could not be written to the end");
}

bool Exists(const std::string &sPath)
{
	FILE *pFile = fopen(sPath.c_str(), "rb");
	if(!pFile)
		return false;
	fclose(pFile);
	return true;
}

} // namespace

int main(int argc, char **argv)
{
	std::vector<std::string> vFiles;
	bool bForce = false;
	for(int i = 1; i < argc; i++){
		const std::string sArgument(argv[i]);
		if(sArgument == "--force")
			bForce = true;
		else
			vFiles.push_back(sArgument);
	}
	if(vFiles.empty()){
		fprintf(stderr, "kc2json: brings an old chain file up to date - the retired XML form (.kc),\n"
						"and version 1 of the JSON one, which is rewritten in place.\n"
						"usage: kc2json [--force] <file.kc | file.kc.json> [more ...]\n");
		return 2;
	}

	int nFailed = 0;
	for(size_t i = 0; i < vFiles.size(); i++){
		const std::string sSource = vFiles[i];
		const std::string sTarget = JsonNameOf(sSource);
		if(!bForce && Exists(sTarget)){
			printf("%s: %s is there already, left alone\n", sSource.c_str(), sTarget.c_str());
			continue;
		}
		try{
			if(EndsWith(sSource,".kc.json")){
				//the file is its own target, so --force was needed to get this far
				WriteWholeFile(sTarget, UpgradedFromVersion1(ReadWholeFile(sSource), sSource));
				printf("%s: version 1 -> version 2\n", sSource.c_str());
			} else {
				const CChain oChain = CChainXmlReader::ReadFile(sSource);
				CChainJsonWriter::WriteFile(oChain, sTarget);
				printf("%s -> %s\n", sSource.c_str(), sTarget.c_str());
			}
		} catch(CGeneratorException &e){
			fprintf(stderr, "%s: %s\n", sSource.c_str(), e.Message().c_str());
			nFailed++;
		}
	}
	return nFailed ? 1 : 0;
}
