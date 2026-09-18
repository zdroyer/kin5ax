#pragma once

#include "GeneratorException.h"

class CMemoryMyException : public CGeneratorException  
{
	public:
	// Standardowy konstruktor jaki powinien byæ wykorzystywany przy wyrzucaniu wyj¹tku
		CMemoryMyException(const string & message  // komunikat wyj¹tku
								,const string & fileName // nazwa pliku Ÿród³owego: __FILE__
								,const int lineNumber // numer lini w pliku: __LINE__
								,const string & methodName) // nazwa metody wyrzucaj¹cej: __FUNCTION__ 
								:CGeneratorException(message,fileName,lineNumber,methodName)
		{
			m_sExceptionDescription = string("Invalid function argument(s)");
		}
};
