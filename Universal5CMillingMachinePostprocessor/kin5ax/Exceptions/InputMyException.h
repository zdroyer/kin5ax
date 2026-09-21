#pragma once

#include "MyException.h"

class CInputMyException : public CMyException  
{

public:
	CInputMyException(const char * str = "InputMyException default message");
	virtual string GetNameOfException();
	CInputMyException(const char * message, const char * method);
};