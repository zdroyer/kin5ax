#include "stdafx.h"
#include "Version.h"

//The build system defines this from `git rev-parse`; a build with no git around, or from an
//unpacked archive, says "unknown" rather than claiming a commit it cannot know.
#ifndef KIN5AX_COMMIT
#define KIN5AX_COMMIT "unknown"
#endif

namespace kin5ax {

const char *VersionString()
{
	return KIN5AX_VERSION_STRING;
}

void Version(int *pMajor, int *pMinor, int *pPatch)
{
	if(pMajor)
		*pMajor = KIN5AX_VERSION_MAJOR;
	if(pMinor)
		*pMinor = KIN5AX_VERSION_MINOR;
	if(pPatch)
		*pPatch = KIN5AX_VERSION_PATCH;
}

const char *CommitHash()
{
	return KIN5AX_COMMIT;
}

const char *BuildString()
{
	//the two literals joined by the compiler, so there is no buffer and nothing to free
	return KIN5AX_VERSION_STRING "+" KIN5AX_COMMIT;
}

} // namespace kin5ax
