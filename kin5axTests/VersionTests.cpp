// The version of the library: the numbers of kin5ax/Version.h, which CMake reads for the version of
// the project and of the shared object, and which the C interface answers a host with. A string and
// three numbers that disagree would be worse than none at all, so they are held together here.
#include "stdafx.h"
#include "gtest/gtest.h"
#include "Version.h"
#include <cstdio>
#include <string>

using namespace kin5ax;

TEST(Version, TheStringIsTheThreeNumbers)
{
	int nMajor = -1, nMinor = -1, nPatch = -1;
	Version(&nMajor,&nMinor,&nPatch);
	EXPECT_EQ(KIN5AX_VERSION_MAJOR,nMajor);
	EXPECT_EQ(KIN5AX_VERSION_MINOR,nMinor);
	EXPECT_EQ(KIN5AX_VERSION_PATCH,nPatch);

	char szExpected[32];
	snprintf(szExpected,sizeof(szExpected),"%d.%d.%d",nMajor,nMinor,nPatch);
	EXPECT_EQ(std::string(szExpected),std::string(VersionString()));
	EXPECT_EQ(std::string(KIN5AX_VERSION_STRING),std::string(VersionString()));
}

TEST(Version, AnyOfThePointersMayBeLeftOut)
{
	int nOnly = -1;
	Version(&nOnly,NULL,NULL);
	EXPECT_EQ(KIN5AX_VERSION_MAJOR,nOnly);
	Version(NULL,&nOnly,NULL);
	EXPECT_EQ(KIN5AX_VERSION_MINOR,nOnly);
	Version(NULL,NULL,&nOnly);
	EXPECT_EQ(KIN5AX_VERSION_PATCH,nOnly);
	Version(NULL,NULL,NULL);		//and all of them, which asks nothing and must not crash
}
