#include "../stdafx.h"
#include "GeneratorException.h"

CGeneratorException::CGeneratorException(void)
{
}

CGeneratorException::~CGeneratorException(void)
{
}

	// Standardowy konstruktor jaki powinien byæ wykorzystywany przy wyrzucaniu wyj¹tku
CGeneratorException::CGeneratorException(const std::string & message // komunikat wyj¹tku
										  ,const std::string & fileName // nazwa pliku Ÿród³owego: __FILE__
										  ,const int lineNumber // numer lini w pliku: __LINE__
										  ,const std::string & methodName) // nazwa metody wyrzucaj¹cej: __FUNCTION__ 										  
{
	m_sMessage = std::string(message);
	char str[50];
	::sprintf(str,", Line: %d\0:",lineNumber);
	m_sThrowingPlace = std::string("File: ") + fileName + std::string(str);
	m_sThrowingMethod = std::string("Function: ")+methodName;
	//m_sThrowingModule = string("Ten komunikat nie powinien siê nigdy pokazaæ \nnie wyrzucamy wyj¹tku klasy CGeneratorException \nprosze poprawiæ kod.");	

//	LOG_ADDSTRING("-----");
//	LOG_ADDSTRING("Ciœniêto wyj¹tkiem:");
//	LOG_ADDSTRING(m_sThrowingPlace.c_str());
//	LOG_ADDSTRING(m_sThrowingMethod.c_str());
//	LOG_ADDSTRING(m_sMessage.c_str());
//	LOG_ADDSTRING("-----");
}

// komunikat opisuj¹cy przyczyny wyst¹pienia wyj¹tku
const std::string & CGeneratorException::Message(void) const
{
	return m_sMessage;
}

// Modu³ programu, z którego zosta³ wyrzucony wyj¹tek
/*
const string &CGeneratorException::GetThrowingModule(void) const 
{
	return m_sThrowingModule;
}
//*/

// Miejsca, sk¹d wyszed³ wyj¹tek
const std::string & CGeneratorException::GetThrowingPlace(void) const
{
	return m_sThrowingPlace;
}

// Wyœwietla wszystkie informacje o wyj¹tku w MessageBoxie
/*
void CGeneratorException::ShowMessageInBox(HWND hwnd) const
{
	string str("Modu³ programu: ");
	str.append(m_sThrowingModule);
	str.append("\n");
	str.append(m_sThrowingPlace);
	str.append("\n");
	str.append(m_sThrowingMethod);
	str.append("\n");
	str.append(string("Komunikat: ") + m_sMessage);
	::MessageBox(hwnd,str.c_str(),"SkyCraftException",MB_OK);	
}

//*/