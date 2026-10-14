#pragma once

//The version of the library, in one place. CMake reads these numbers out of this file for the
//version of the project and of the shared object; `k5_version` answers with them, so a host always
//learns the version of the library it is actually talking to rather than the one it was built
//against. The C interface repeats them in its own header, which cannot include this one, and a test
//of that interface holds the two together.
//
//Semantic versioning, with the usual meaning of a leading zero: while the major number is 0 the
//interface may still change between minor versions, and a host should check for the one it needs.

#define KIN5AX_VERSION_MAJOR 0
#define KIN5AX_VERSION_MINOR 1
#define KIN5AX_VERSION_PATCH 0
#define KIN5AX_VERSION_STRING "0.1.0"

namespace kin5ax {

//"0.1.0", the same as KIN5AX_VERSION_STRING: the version compiled into this library
const char *VersionString();
//the three numbers of it; any of the pointers may be NULL
void Version(int *pMajor, int *pMinor, int *pPatch);

} // namespace kin5ax
