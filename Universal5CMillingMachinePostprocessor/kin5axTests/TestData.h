#pragma once
// Test data of kin5axTests: the kinematic chain (.kc) files of the repository.

#include "Chain.h"

//folder of the solution file (with the trailing backslash); found by walking up from the test exe
string GetSolutionDir();

//full paths of all .kc files: the solution folder and its TestChains subfolder
vector<string> GetChainFiles();

//file name without folder and extension, reduced to [A-Za-z0-9_] (GoogleTest parameter names)
string GetTestName(const string &sFilePath);

//reads a chain the same way as the application does (MSXML 6, DTD validation on)
CChain LoadChain(const string &sFilePath);
