#include "stdafx.h"
#include "Version.h"

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

} // namespace kin5ax
