// stdafx.h of the kin5ax library: C++ standard library only (no Windows, no MFC, no OpenGL)

#pragma once

#ifdef _MSC_VER
#pragma warning(disable:4786)	//identifier truncated in the debug information
#pragma warning(disable:4996)	//the C library functions MSVC calls unsafe (fopen, strerror, ...)
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <locale.h>

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

//no using-directive: the library names its standard types explicitly (std::)
