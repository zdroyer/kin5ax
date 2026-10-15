// The .fcm bridge through the C interface: kin5axfcm.h, a header of its own over the same shared
// library. These link kin5axc and nothing else, like the rest of this suite, so what they exercise
// is what a host gets.
//
// The conversion itself is tested against the C++ side in kin5axTests/ChainFcmTests.cpp. What is
// tested here is the boundary: that a FreeCAD machine file becomes a chain like any other, that the
// chain says millimetres because that is what an .fcm is, and above all that the remarks come
// across - a bridge that converts silently is worse than one that refuses, because what it drops is
// the difference between the machine in the file and the machine about to be solved for.
#include "gtest/gtest.h"
#include "kin5ax.h"
#include "kin5axfcm.h"

#include <string>
#include <vector>
#include <cstdio>
#include <filesystem>

namespace {

std::string TempPath(const char *szName)
{
	return (std::filesystem::temp_directory_path() / szName).string();
}

//a BC table machine, the shape of the template of the C++ suite: X, Y, Z in the head, B and C under
//the workpiece. Lengths in millimetres, as an .fcm is by FreeCAD's definition.
const char *szFCM = R"FCM({
  "freecad_version": "1.1",
  "machine": {
    "name": "Test BC table",
    "manufacturer": "kin5axcTests",
    "units": "metric",
    "kinematics": { "base_frame": { "origin": [0, 0, 0], "orientation_quaternion": [0, 0, 0, 1] } },
    "axes": {
      "X": { "type": "linear", "role": "head_linear", "parent": null, "sequence": 0,
             "joint": [[0, 0, 0], [1, 0, 0]], "limits": { "min": -350, "max": 350 } },
      "Y": { "type": "linear", "role": "head_linear", "parent": "X", "sequence": 1,
             "joint": [[0, 0, 0], [0, 1, 0]], "limits": { "min": -250, "max": 250 } },
      "Z": { "type": "linear", "role": "head_linear", "parent": "Y", "sequence": 2,
             "joint": [[0, 0, 0], [0, 0, 1]], "limits": { "min": -400, "max": 0 } },
      "B": { "type": "rotary", "role": "table_rotary", "parent": null, "sequence": 0,
             "joint": [[0, 0, -80], [0, 1, 0]], "limits": { "min": -110, "max": 110 } },
      "C": { "type": "rotary", "role": "table_rotary", "parent": "B", "sequence": 1,
             "joint": [[0, 0, -80], [0, 0, 1]], "limits": { "min": -360, "max": 360 } }
    },
    "toolheads": []
  },
  "version": 1
}
)FCM";

std::string WriteTemp(const char *szName, const std::string &sContent)
{
	const std::string sPath = TempPath(szName);
	FILE *pFile = fopen(sPath.c_str(),"wb");
	EXPECT_TRUE(pFile!=NULL)<<sPath;
	if(pFile){
		fwrite(sContent.data(),1,sContent.size(),pFile);
		fclose(pFile);
	}
	return sPath;
}

//every remark of a chain, as a host would read them
std::vector<std::string> Remarks(k5_chain chain)
{
	std::vector<std::string> v;
	size_t n = 0;
	EXPECT_EQ(K5_OK,k5_fcm_remark_count(chain,&n));
	for(size_t i=0;i<n;i++){
		char sz[K5_REMARK_MAX] = {0};
		EXPECT_EQ(K5_OK,k5_fcm_remark(chain,i,sz,sizeof(sz)))<<"remark "<<i;
		v.push_back(sz);
	}
	return v;
}

bool AnyContains(const std::vector<std::string> &v, const char *szText)
{
	for(size_t i=0;i<v.size();i++)
		if(v[i].find(szText)!=std::string::npos)
			return true;
	return false;
}

} // namespace

TEST(Fcm, TheDefaultOptionsAreBothFlagsOffAndNoStrings)
{
	k5_fcm_options o;
	o.negate_table_linear_axes = 1;
	o.negate_table_rotary_axes = 1;
	o.machine_name = "something";
	k5_fcm_options_default(&o);
	EXPECT_EQ(0,o.negate_table_linear_axes);
	EXPECT_EQ(0,o.negate_table_rotary_axes);
	EXPECT_TRUE(o.machine_name==NULL);
	EXPECT_TRUE(o.template_file_path==NULL);
	k5_fcm_options_default(NULL);		//allowed, and does nothing
}

TEST(Fcm, AMachineFileBecomesAChainInMillimetres)
{
	const std::string sPath = WriteTemp("kin5axcTests_bridge.fcm",szFCM);
	k5_chain chain = NULL;
	ASSERT_EQ(K5_OK,k5_chain_open_fcm(sPath.c_str(),NULL,&chain))<<k5_last_error();
	ASSERT_TRUE(chain!=NULL);

	//it is a chain like any other: the calls of kin5ax.h work on it
	int bIs5C = 0;
	ASSERT_EQ(K5_OK,k5_chain_is_5c(chain,&bIs5C));
	EXPECT_EQ(1,bIs5C)<<"three linear and two rotary joints";
	size_t nLinks = 0;
	ASSERT_EQ(K5_OK,k5_chain_link_count(chain,&nLinks));
	EXPECT_EQ((size_t)K5_AXES,nLinks);

	//and it is in millimetres, because that is what an .fcm is
	int nUnit = -1;
	ASSERT_EQ(K5_OK,k5_chain_unit(chain,&nUnit));
	EXPECT_EQ(K5_UNIT_MM,nUnit);
	double dInMillimetres = 0.0;
	ASSERT_EQ(K5_OK,k5_unit_in_millimetres(nUnit,&dInMillimetres));
	EXPECT_DOUBLE_EQ(1.0,dInMillimetres);

	//the limits of the file, as they were written, with no thousand anywhere
	k5_link oLink;
	ASSERT_EQ(K5_OK,k5_chain_link(chain,0,&oLink));
	bool bFound = false;
	for(size_t i=0;i<nLinks && !bFound;i++){
		ASSERT_EQ(K5_OK,k5_chain_link(chain,i,&oLink));
		if(std::string(oLink.name)=="X"){
			bFound = true;
			EXPECT_EQ(1,oLink.constrained);
			EXPECT_NEAR(-350.0,oLink.minimum,1E-9)<<"the millimetres of the file";
			EXPECT_NEAR(350.0,oLink.maximum,1E-9);
		}
	}
	EXPECT_TRUE(bFound)<<"the axis named X of the file";

	k5_chain_free(chain);
	remove(sPath.c_str());
}

TEST(Fcm, WhatTheConversionCouldNotCarryIsReadBackOffTheChain)
{
	//a file with a tool head and an axis without a role: two things a kinematic chain has no place
	//for, and the bridge says so rather than dropping them where nobody looks
	std::string sText = szFCM;
	const std::string sOld = "\"toolheads\": []";
	const size_t n = sText.find(sOld);
	ASSERT_NE(std::string::npos,n);
	sText = sText.substr(0,n)+"\"toolheads\": [{\"name\": \"spindle\"}]"+sText.substr(n+sOld.size());

	const std::string sPath = WriteTemp("kin5axcTests_remarks.fcm",sText);
	k5_chain chain = NULL;
	ASSERT_EQ(K5_OK,k5_chain_open_fcm(sPath.c_str(),NULL,&chain))<<k5_last_error();

	const std::vector<std::string> v = Remarks(chain);
	ASSERT_FALSE(v.empty())<<"a tool head was dropped and nothing said so";
	EXPECT_TRUE(AnyContains(v,"tool head"))<<"the remarks: "<<v.size();
	for(size_t i=0;i<v.size();i++)
		EXPECT_FALSE(v[i].empty())<<"remark "<<i;

	//a buffer shorter than the remark is cut, not overrun, and an index past the end is refused
	char szShort[12] = {0};
	ASSERT_EQ(K5_OK,k5_fcm_remark(chain,0,szShort,sizeof(szShort)));
	EXPECT_EQ((size_t)11,std::string(szShort).size());
	char szAny[K5_REMARK_MAX];
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_fcm_remark(chain,v.size(),szAny,sizeof(szAny)));

	k5_chain_free(chain);
	remove(sPath.c_str());
}

TEST(Fcm, AChainThatCarriedAcrossWholeHasNoRemarks)
{
	const std::string sPath = WriteTemp("kin5axcTests_exact.fcm",szFCM);
	k5_chain chain = NULL;
	ASSERT_EQ(K5_OK,k5_chain_open_fcm(sPath.c_str(),NULL,&chain))<<k5_last_error();
	size_t nRemarks = 1;
	ASSERT_EQ(K5_OK,k5_fcm_remark_count(chain,&nRemarks));
	EXPECT_EQ((size_t)0,nRemarks)<<"this file holds nothing a kinematic chain cannot say";
	k5_chain_free(chain);
	remove(sPath.c_str());

	//and a chain that has been through no conversion at all has none either
	k5_chain kc = NULL;
	ASSERT_EQ(K5_OK,k5_chain_open((std::string(KIN5AXC_TEST_CHAINS_DIR)+"/AC_TiltRotaryTable.kc").c_str(),&kc));
	nRemarks = 1;
	ASSERT_EQ(K5_OK,k5_fcm_remark_count(kc,&nRemarks));
	EXPECT_EQ((size_t)0,nRemarks);
	k5_chain_free(kc);
}

TEST(Fcm, AChainOfTheLibraryIsWrittenAsAMachineFileAndSaysWhatItLeftBehind)
{
	//A .kc of the test set written out as a FreeCAD machine and read back. Two things are shown
	//here, and the second is the reason this bridge reports at all.
	//
	//The machine survives as a machine: five joints, still a 5C chain, now in millimetres, because
	//an .fcm is millimetres and the trunnion table was drawn in metres.
	//
	//But it is not the same chain, and cannot be. An .fcm has no collision boxes, and it puts the
	//tool frame on the base frame at the home position, so a tool terminal that stood somewhere
	//else does not come back. Both are said in the remarks rather than discovered later by a user
	//whose collision check finds nothing and whose tool is in the wrong place.
	k5_chain kc = NULL;
	ASSERT_EQ(K5_OK,k5_chain_open((std::string(KIN5AXC_TEST_CHAINS_DIR)+"/AC_TiltRotaryTable.kc").c_str(),&kc));
	int nUnit = -1;
	ASSERT_EQ(K5_OK,k5_chain_unit(kc,&nUnit));
	ASSERT_EQ(K5_UNIT_M,nUnit)<<"the models shipped are metres";

	const std::string sPath = TempPath("kin5axcTests_written.fcm");
	ASSERT_EQ(K5_OK,k5_chain_write_fcm(kc,NULL,sPath.c_str()))<<k5_last_error();
	const std::vector<std::string> vWritten = Remarks(kc);
	EXPECT_TRUE(AnyContains(vWritten,"collision boxes"))<<"writing dropped them silently";
	EXPECT_TRUE(AnyContains(vWritten,"tool reference point"))<<"the tool terminal moved silently";

	k5_chain back = NULL;
	ASSERT_EQ(K5_OK,k5_chain_open_fcm(sPath.c_str(),NULL,&back))<<k5_last_error();
	ASSERT_EQ(K5_OK,k5_chain_unit(back,&nUnit));
	EXPECT_EQ(K5_UNIT_MM,nUnit)<<"what comes back is an .fcm, which is millimetres";
	int bIs5C = 0;
	ASSERT_EQ(K5_OK,k5_chain_is_5c(back,&bIs5C));
	EXPECT_EQ(1,bIs5C)<<"still a machine the solver can work with";
	size_t nLinks = 0;
	ASSERT_EQ(K5_OK,k5_chain_link_count(back,&nLinks));
	EXPECT_EQ((size_t)K5_AXES,nLinks);
	//the linear travel of the file is the travel of the chain, a thousand times over
	for(size_t i=0;i<nLinks;i++){
		k5_link a, b;
		ASSERT_EQ(K5_OK,k5_chain_link(kc,i,&a));
		ASSERT_EQ(K5_OK,k5_chain_link(back,i,&b));
		EXPECT_EQ(a.type,b.type)<<"joint "<<i;
		if(a.type==K5_LINK_PRISMATIC && a.constrained && b.constrained)
			EXPECT_NEAR(a.minimum*1000.0,b.minimum,1E-6)<<"joint "<<i<<" travels the same distance";
	}

	//reading it back replaced the remarks of writing it: they belong to the last conversion
	const std::vector<std::string> vRead = Remarks(back);
	EXPECT_FALSE(AnyContains(vRead,"collision boxes"))<<"those were remarks of the write";

	k5_chain_free(back);
	k5_chain_free(kc);
	remove(sPath.c_str());
}

TEST(Fcm, TheOptionsReachTheConversion)
{
	//negating a table rotary axis is the one convention an .fcm cannot state, so it is given here;
	//the machine that comes of it turns the other way, which is what the flag is for
	const std::string sPath = WriteTemp("kin5axcTests_negate.fcm",szFCM);
	k5_fcm_options o;
	k5_fcm_options_default(&o);
	o.negate_table_rotary_axes = 1;
	o.machine_name = "a name the file does not carry";

	k5_chain plain = NULL, negated = NULL;
	ASSERT_EQ(K5_OK,k5_chain_open_fcm(sPath.c_str(),NULL,&plain))<<k5_last_error();
	ASSERT_EQ(K5_OK,k5_chain_open_fcm(sPath.c_str(),&o,&negated))<<k5_last_error();

	//Every rotary joint turned, not just the first. The first of this machine is C, whose axis is
	//the line the tool point stands on at the home position, so turning it either way leaves that
	//point exactly where it was - a negation that is real and invisible at that one spot.
	double q[K5_AXES] = {0,0,0,0,0};
	for(size_t i=0;i<(size_t)K5_AXES;i++){
		k5_link oLink;
		ASSERT_EQ(K5_OK,k5_chain_link(plain,i,&oLink));
		if(oLink.type==K5_LINK_REVOLUTE)
			q[i] = 0.5;
	}
	double a[3] = {0,0,0}, b[3] = {0,0,0}, vIgnored[3] = {0,0,0};
	ASSERT_EQ(K5_OK,k5_chain_tool_point(plain,NULL,q,a,vIgnored));
	ASSERT_EQ(K5_OK,k5_chain_tool_point(negated,NULL,q,b,vIgnored));
	const double dApart = (a[0]-b[0])*(a[0]-b[0])+(a[1]-b[1])*(a[1]-b[1])+(a[2]-b[2])*(a[2]-b[2]);
	EXPECT_GT(dApart,1E-9)<<"the flag did not reach the conversion";

	char szName[K5_NAME_MAX] = {0};
	ASSERT_EQ(K5_OK,k5_chain_name(negated,szName,sizeof(szName)));
	EXPECT_STREQ("a name the file does not carry",szName);

	k5_chain_free(plain);
	k5_chain_free(negated);
	remove(sPath.c_str());
}

TEST(Fcm, BadArgumentsAreRefusedAndNothingIsHandedBack)
{
	k5_chain chain = (k5_chain)1;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_open_fcm(NULL,NULL,&chain));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_open_fcm("nowhere.fcm",NULL,NULL));
	EXPECT_EQ(K5_ERROR_FILE,k5_chain_open_fcm(TempPath("kin5axcTests_no_such.fcm").c_str(),NULL,&chain));
	EXPECT_TRUE(chain==NULL)<<"a failed open hands back nothing to free";
	EXPECT_STRNE("",k5_last_error());

	size_t n = 1;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_fcm_remark_count(NULL,&n));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_fcm_remark_count((k5_chain)NULL,NULL));
	char sz[K5_REMARK_MAX];
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_fcm_remark(NULL,0,sz,sizeof(sz)));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_write_fcm(NULL,NULL,"nowhere.fcm"));
}
