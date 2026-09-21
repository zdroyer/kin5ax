#pragma once

class CGeneratorException
{
public:
	CGeneratorException(void);
	virtual ~CGeneratorException(void);


	// Standardowy konstruktor jaki powinien byæ wykorzystywany przy wyrzucaniu wyj¹tku
	CGeneratorException(const string & message  // komunikat wyj¹tku
						,const string & fileName // nazwa pliku Ÿród³owego: __FILE__
						,const int lineNumber // numer lini w pliku: __LINE__
						,const string & methodName // nazwa metody wyrzucaj¹cej: __FUNCTION__ lub __FUNCSIG__
						);

	// Komunikat opisuj¹cy przyczyny wyst¹pienia wyj¹tku
	const string & Message(void) const ;

	/* Modu³ programu, z którego zosta³ wyrzucony wyj¹tek
	const string & GetThrowingModule(void) const;
	//*/

	// Miejsce (w kodzie) wyrzucenia wyj¹tku
	const string & GetThrowingPlace(void) const;

	// Wyœwietla wszystkie informacje o wyj¹tku w MessageBoxie. Metoda tylko do debugowania
	//void ShowMessageInBox(HWND hwnd = 0) const; //hwnd - okno-rodzic wyœwietlanego MessageBoxa


protected:
	// Komunikat b³êdu
	string m_sMessage;

	// Krotki opis typu wyjatku
	string m_sExceptionDescription;

	// Nazwa pliku, numer lini gdzie wyrzucany jest wyj¹tek
	string m_sThrowingPlace;

	// Nazwa metody która wyrzuca ten wy¹tek
	string m_sThrowingMethod;

};

#define MY_THROW(exception, text) throw exception(text,__FILE__,__LINE__,__FUNCTION__)


