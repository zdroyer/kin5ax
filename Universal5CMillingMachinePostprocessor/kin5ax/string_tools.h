#pragma once 
#include <string>

namespace kin5ax {

static std::string sNL("\n");
static std::string sRNL("\r\n");


//printf into a string of the needed length (no fixed buffer, no shared buffer)
std::string string_printf(const char *szFormat, ...);
std::string size_to_string(size_t n, bool bEndWithNewLineAndR=false);
std::string double_to_string(double d, bool bEndWithNewLineAndR=false);
//the text with the five XML markup characters (& < > " ') replaced by their entities: safe inside an attribute value
std::string xml_escape(const std::string &s);

} // namespace kin5ax
