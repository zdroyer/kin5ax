#pragma once 
#include <string>

static string sNL("\n");
static string sRNL("\r\n");


string size_to_string(size_t n, bool bEndWithNewLineAndR=false);
string double_to_string(double d, bool bEndWithNewLineAndR=false);