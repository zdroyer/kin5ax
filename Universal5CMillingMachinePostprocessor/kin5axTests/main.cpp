// kin5axTests: GoogleTest tests of the kin5ax library.
// Run the exe directly (exit code 1 = a test failed) or through the Visual Studio Test Explorer.
#include "../stdafx.h"
#include "gtest/gtest.h"

int main(int argc, char **argv)
{
	//a failed assertion of the debug runtime must end the run with a message, not wait in a dialog box
	_CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR,_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR,_CRTDBG_FILE_STDERR);
	::testing::InitGoogleTest(&argc, argv);
	CoInitialize(NULL);		//the chains are read with MSXML
	int nResult = RUN_ALL_TESTS();
	CoUninitialize();
	return nResult;
}
