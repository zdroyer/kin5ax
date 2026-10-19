#pragma once
// Test data of kin5axTests: the kinematic chain (.kc.json) files of the test project
// (kin5axTests/TestChains, copied next to the test exe by the build) and temporary files.

#include "Chain.h"

//folder with the chain files (with the trailing separator): TestChains next to the test exe
std::string GetTestChainsDir();

//full paths of all .kc.json files of GetTestChainsDir()
std::vector<std::string> GetChainFiles();

//file name without folder and extension, reduced to [A-Za-z0-9_] (GoogleTest parameter names)
std::string GetTestName(const std::string &sFilePath);

//a file name in the temporary folder of the system
std::string TempFilePath(const std::string &sName);

//reads a chain the same way as the application does (CChainJsonReader of kin5ax)
kin5ax::CChain LoadChain(const std::string &sFilePath);
