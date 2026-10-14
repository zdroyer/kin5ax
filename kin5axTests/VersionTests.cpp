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

//A binary of this library is shipped on its own, inside the FreeCAD addon and in a release archive,
//where nothing else says which source it was built from. The commit is compiled in so that it does.
TEST(Version, TheCommitItWasBuiltFromIsCompiledIn)
{
	const std::string sCommit = CommitHash();
	ASSERT_FALSE(sCommit.empty())<<"a build always says something, even if only that it cannot tell";

	//in a checkout it is the abbreviated hash, possibly marked as not being that commit exactly;
	//built from an unpacked archive, or with no git to ask, it says so instead of inventing one
	if(sCommit!="unknown"){
		const std::string sHash = sCommit.substr(0,sCommit.find('-'));
		EXPECT_GE(sHash.size(),(size_t)7)<<sCommit<<": an abbreviation short enough to be ambiguous";
		EXPECT_LE(sHash.size(),(size_t)40)<<sCommit;
		for(size_t i=0;i<sHash.size();i++)
			EXPECT_NE(std::string::npos,std::string("0123456789abcdef").find(sHash[i]))
				<<sCommit<<" is not a hexadecimal hash";
		if(sCommit.size()!=sHash.size())
			EXPECT_EQ("-dirty",sCommit.substr(sHash.size()))<<"the only mark there is";
	}

	//and the two together, the way semantic versioning writes build metadata
	EXPECT_EQ(std::string(VersionString())+"+"+sCommit,std::string(BuildString()));
	EXPECT_EQ(std::string(KIN5AX_VERSION_STRING)+"+"+sCommit,std::string(BuildString()));
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
