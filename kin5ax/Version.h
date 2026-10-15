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
#define KIN5AX_VERSION_MINOR 2
#define KIN5AX_VERSION_PATCH 0
#define KIN5AX_VERSION_STRING "0.2.0"

namespace kin5ax {

//"0.1.0", the same as KIN5AX_VERSION_STRING: the version compiled into this library
const char *VersionString();
//the three numbers of it; any of the pointers may be NULL
void Version(int *pMajor, int *pMinor, int *pPatch);

//The commit this library was built from, abbreviated: "09a933b4f1c2", with "-dirty" after it when
//the working tree held changes that are in no commit, and "unknown" when it was built without git
//or outside a checkout. A binary that is shipped - and kin5axc is shipped, inside the FreeCAD addon
//- can be traced back to the source it came from only if it carries this.
const char *CommitHash();
//"0.1.0+09a933b4f1c2": the version and the commit, in the form semantic versioning gives build
//metadata. Metadata is what it is: it says nothing about the interface and takes no part in the
//compatibility check, which is why it is not in VersionString.
const char *BuildString();

} // namespace kin5ax
