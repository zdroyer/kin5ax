#pragma once 
#include "stdafx.h"
#include "string_tools.h"

//static string sNL("\n");
//static string sRNL("\r\n");


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
