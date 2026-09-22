#pragma once
// Test data of kin5axTests: the kinematic chain (.kc) files of the test project
// (kin5axTests/TestChains, copied next to the test exe by the build) and temporary files.

#include "Chain.h"

//folder with the .kc files (with the trailing separator): TestChains next to the test exe
string GetTestChainsDir();

//full paths of all .kc files of GetTestChainsDir()
vector<string> GetChainFiles();

//file name without folder and extension, reduced to [A-Za-z0-9_] (GoogleTest parameter names)
string GetTestName(const string &sFilePath);

//a file name in the temporary folder of the system
string TempFilePath(const string &sName);

//reads a chain the same way as the application does (CChainXmlReader of kin5ax)
CChain LoadChain(const string &sFilePath);
