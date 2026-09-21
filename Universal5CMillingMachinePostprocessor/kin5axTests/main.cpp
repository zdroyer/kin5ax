// kin5axTests: GoogleTest tests of the kin5ax library.
// Run the exe directly (exit code 1 = a test failed) or through the Visual Studio Test Explorer.
#include "../stdafx.h"
#include "gtest/gtest.h"

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	CoInitialize(NULL);		//the chains are read with MSXML
	int nResult = RUN_ALL_TESTS();
	CoUninitialize();
	return nResult;
}
