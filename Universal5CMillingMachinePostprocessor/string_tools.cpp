#pragma once 
#include "stdafx.h"
#include "string_tools.h"

//static string sNL("\n");
//static string sRNL("\r\n");


string size_to_string(size_t n, bool bEndWithNewLineAndR){
	static char s[25];
	s[24]=0;
	if(bEndWithNewLineAndR)
		sprintf(s,"%d\r\n",n);
	else
		sprintf(s,"%d",n);
	return string(s);
}
string double_to_string(double d, bool bEndWithNewLineAndR){
	static char s[255];
	s[254]=0;
	if(bEndWithNewLineAndR)
		sprintf(s,"%.8G\r\n",d);
	else
		sprintf(s,"%.8G",d);
	return string(s);
}
