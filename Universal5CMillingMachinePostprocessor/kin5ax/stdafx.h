// stdafx.h of the kin5ax library: C++ standard library only (no Windows, no MFC, no OpenGL)

#pragma once

#pragma warning(disable:4786)
#pragma warning(disable:4996)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>

//no using-directive: the library names its standard types explicitly (std::)
