#pragma once 
#include <string>

static string sNL("\n");
static string sRNL("\r\n");


//printf into a string of the needed length (no fixed buffer, no shared buffer)
string string_printf(const char *szFormat, ...);
string size_to_string(size_t n, bool bEndWithNewLineAndR=false);
string double_to_string(double d, bool bEndWithNewLineAndR=false);
//the text with the five XML markup characters (& < > " ') replaced by their entities: safe inside an attribute value
string xml_escape(const string &s);