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

//JSON text as dump() leaves it, with every array that holds nothing but numbers put back on
//one line: a vector of three is easier to read and to edit that way. No string is touched.
std::string json_numbers_on_one_line(const std::string &sJson);

} // namespace kin5ax
