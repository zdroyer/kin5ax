// kin5axcTests: GoogleTest tests of the C interface of kin5ax. The target links the shared library and
// not the C++ one, so anything these tests reach is part of the interface.
#include "gtest/gtest.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

int main(int argc, char **argv)
{
#ifdef _MSC_VER
	//a failed assertion of the debug runtime must end the run with a message, not wait in a dialog box
	_CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR,_CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR,_CRTDBG_FILE_STDERR);
#endif
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
