#pragma once

#include <exception>
#include <string>

namespace kin5ax {

//The base of every exception of kin5ax, a std::exception for the hosts that know nothing else:
//what() is the message. MY_THROW adds the file, the line and the function that threw.
class CGeneratorException : public std::exception
{
public:
	CGeneratorException(void);
	virtual ~CGeneratorException(void);
	virtual const char *what() const noexcept;
	//the kind of the exception, set by the derived classes ("Division by zero", ...)
	const std::string & GetDescription(void) const;
	//"Function: " and the name of the throwing method
	const std::string & GetThrowingMethod(void) const;


	// Standardowy konstruktor jaki powinien byæ wykorzystywany przy wyrzucaniu wyj¹tku
	CGeneratorException(const std::string & message  // komunikat wyj¹tku
						,const std::string & fileName // nazwa pliku Ÿród³owego: __FILE__
						,const int lineNumber // numer lini w pliku: __LINE__
						,const std::string & methodName // nazwa metody wyrzucaj¹cej: __FUNCTION__ lub __FUNCSIG__
						);

	// Komunikat opisuj¹cy przyczyny wyst¹pienia wyj¹tku
	const std::string & Message(void) const ;

	/* Modu³ programu, z którego zosta³ wyrzucony wyj¹tek
	const string & GetThrowingModule(void) const;
	//*/

	// Miejsce (w kodzie) wyrzucenia wyj¹tku
	const std::string & GetThrowingPlace(void) const;

	// Wyœwietla wszystkie informacje o wyj¹tku w MessageBoxie. Metoda tylko do debugowania
	//void ShowMessageInBox(HWND hwnd = 0) const; //hwnd - okno-rodzic wyœwietlanego MessageBoxa


protected:
	// Komunikat b³êdu
	std::string m_sMessage;

	// Krotki opis typu wyjatku
	std::string m_sExceptionDescription;

	// Nazwa pliku, numer lini gdzie wyrzucany jest wyj¹tek
	std::string m_sThrowingPlace;

	// Nazwa metody która wyrzuca ten wy¹tek
	std::string m_sThrowingMethod;

};

#define MY_THROW(exception, text) throw exception(text,__FILE__,__LINE__,__FUNCTION__)



} // namespace kin5ax
