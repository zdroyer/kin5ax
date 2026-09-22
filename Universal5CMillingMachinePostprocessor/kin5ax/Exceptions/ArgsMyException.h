#pragma once
#include "GeneratorException.h"

class CArgsMyException:
	public CGeneratorException 
{
	public:
	// Standardowy konstruktor jaki powinien byæ wykorzystywany przy wyrzucaniu wyj¹tku
		CArgsMyException(const std::string & message  // komunikat wyj¹tku
								,const std::string & fileName // nazwa pliku Ÿród³owego: __FILE__
								,const int lineNumber // numer lini w pliku: __LINE__
								,const std::string & methodName) // nazwa metody wyrzucaj¹cej: __FUNCTION__ 
								:CGeneratorException(message,fileName,lineNumber,methodName)
		{
			m_sExceptionDescription = std::string("Invalid function argument(s)");
		}
};
