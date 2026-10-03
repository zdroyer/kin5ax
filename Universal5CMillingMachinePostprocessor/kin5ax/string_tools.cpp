#include "stdafx.h"
#include "string_tools.h"
#include <charconv>

//static string sNL("\n");
//static string sRNL("\r\n");


namespace kin5ax {

std::string string_printf(const char *szFormat, ...)
{
	va_list args;
	va_start(args,szFormat);
	va_list args2;
	va_copy(args2,args);
	int nLength = vsnprintf(NULL,0,szFormat,args);
	va_end(args);
	std::string sResult;
	if(nLength>0){
		std::vector<char> v((size_t)nLength+1);
		vsnprintf(&v[0],v.size(),szFormat,args2);
		sResult.assign(&v[0],(size_t)nLength);
	}
	va_end(args2);
	return sResult;
}

std::string size_to_string(size_t n, bool bEndWithNewLineAndR){
	return string_printf(bEndWithNewLineAndR ? "%zu\r\n" : "%zu",n);
}
std::string double_to_string(double d, bool bEndWithNewLineAndR){
	return string_printf(bEndWithNewLineAndR ? "%.8G\r\n" : "%.8G",d);
}

std::string double_to_string_exact(double d)
{
	char szBuffer[64];
	std::to_chars_result oResult = std::to_chars(szBuffer,szBuffer+sizeof(szBuffer),d);
	return std::string(szBuffer,oResult.ptr);
}

std::string xml_escape(const std::string &s)
{
	std::string sResult;
	sResult.reserve(s.size());
	for(size_t i=0;i<s.size();i++){
		switch(s[i]){
			case '&': sResult+="&amp;"; break;
			case '<': sResult+="&lt;"; break;
			case '>': sResult+="&gt;"; break;
			case '"': sResult+="&quot;"; break;
			case '\'': sResult+="&apos;"; break;
			default: sResult+=s[i];
		}
	}
	return sResult;
}

//dump() puts every element of an array on its own line: a three-number vector is easier to read
//and to edit on one line. Only arrays that hold nothing but numbers are collapsed, so no string
//is ever touched (a string would bring a character that is not part of a number).
std::string json_numbers_on_one_line(const std::string &s)
{
	std::string sResult;
	sResult.reserve(s.size());
	for(size_t i=0;i<s.size();){
		char c = s[i];
		if(c=='"'){							//copy a string literal unchanged
			size_t j=i+1;
			while(j<s.size() && s[j]!='"'){
				if(s[j]=='\\')
					j++;
				j++;
			}
			j = std::min(j+1,s.size());
			sResult.append(s,i,j-i);
			i = j;
			continue;
		}
		if(c=='['){
			size_t j=i+1;
			bool bOnlyNumbers = true;
			while(j<s.size() && s[j]!=']'){
				char d = s[j];
				if(!(isdigit((unsigned char)d) || d=='-' || d=='+' || d=='.' || d=='e' || d=='E'
					|| d==',' || isspace((unsigned char)d))){
					bOnlyNumbers = false;
					break;
				}
				j++;
			}
			if(bOnlyNumbers && j<s.size()){
				sResult += '[';
				for(size_t k=i+1;k<j;k++){
					if(isspace((unsigned char)s[k]))
						continue;
					if(s[k]==',')
						sResult += ", ";
					else
						sResult += s[k];
				}
				sResult += ']';
				i = j+1;
				continue;
			}
		}
		sResult += c;
		i++;
	}
	return sResult;
}

} // namespace kin5ax
