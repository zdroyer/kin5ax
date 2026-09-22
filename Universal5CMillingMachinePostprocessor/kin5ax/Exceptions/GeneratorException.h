#pragma once

class CGeneratorException
{
public:
	CGeneratorException(void);
	virtual ~CGeneratorException(void);


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


