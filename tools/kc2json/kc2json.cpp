/**
 * @file kc2json.cpp
 * @brief Converts the retired XML chain file (.kc) into the JSON one (.kc.json).
 *
 * The XML form is no longer read by the library. This tool is the way out for a file that is still
 * in it: it reads with the last XML reader there was and writes the JSON the library now uses.
 *
 *     kc2json <file.kc> [more.kc ...]
 *
 * Each file is written beside its source as `<file>.kc.json`, and an existing one is not touched
 * unless `--force` is given. Nothing of the chain is lost in the direction XML to JSON: the JSON
 * form carries everything the XML one did.
 */
#include "stdafx.h"
#include "ChainXmlReader.h"
#include "ChainJson.h"
#include "GeneratorException.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace kin5ax;

namespace {

/// The name to write beside the source: "AC.kc" becomes "AC.kc.json".
std::string JsonNameOf(const std::string &sPath)
{
	return sPath + ".json";
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
		fprintf(stderr, "kc2json: the XML chain file (.kc) is retired; this writes the JSON one.\n"
						"usage: kc2json [--force] <file.kc> [more.kc ...]\n");
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
			const CChain oChain = CChainXmlReader::ReadFile(sSource);
			CChainJsonWriter::WriteFile(oChain, sTarget);
			printf("%s -> %s\n", sSource.c_str(), sTarget.c_str());
		} catch(CGeneratorException &e){
			fprintf(stderr, "%s: %s\n", sSource.c_str(), e.Message().c_str());
			nFailed++;
		}
	}
	return nFailed ? 1 : 0;
}
