// The C interface of kin5ax, used the way a host uses it: handles, out parameters and statuses, with
// nothing of the C++ library in sight. The round trip of the solver is checked through the interface
// itself - forward kinematics to a cutter location, the solver back to the joint values.
#include "gtest/gtest.h"
#include "kin5ax.h"

#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <algorithm>

extern "C" int k5c_header_check_c(void);		//CHeaderCheck.c

namespace {

const char *szCHAIN = "AC_TiltRotaryTable.kc";	//a trunnion table: A tilts, C turns the workpiece

std::string ChainPath(const char *szFile = szCHAIN)
{
	return std::string(KIN5AXC_TEST_CHAINS_DIR) + "/" + szFile;
}

std::string TempPath(const char *szName)
{
	return (std::filesystem::temp_directory_path() / szName).string();
}

//a chain of the test set, opened or the test failed
k5_chain OpenChain(const char *szFile = szCHAIN)
{
	k5_chain chain = NULL;
	const k5_status n = k5_chain_open(ChainPath(szFile).c_str(),&chain);
	EXPECT_EQ(K5_OK,n)<<k5_last_error();
	EXPECT_TRUE(chain!=NULL);
	return chain;
}

//the default tool of the application: a 16 mm ball nose with a shank and a holder
k5_tool CreateTool()
{
	k5_tool tool = NULL;
	const k5_status n = k5_tool_create(K5_TOOL_BALL_NOSE,0.016,0.016,0.064,0.030,0.050,0.030,&tool);
	EXPECT_EQ(K5_OK,n)<<k5_last_error();
	return tool;
}

//Joint values inside the limits of the machine, with the rotary axes away from zero so that the tool
//axis is oblique and the orientation is not singular.
std::vector<double> QInsideLimits(k5_chain chain)
{
	std::vector<double> q((size_t)K5_AXES,0.0);
	size_t nRotary = 0;
	for(size_t i=0;i<(size_t)K5_AXES;i++){
		k5_link oLink;
		EXPECT_EQ(K5_OK,k5_chain_link(chain,i,&oLink))<<k5_last_error();
		double d = 0.0;
		if(oLink.type==K5_LINK_REVOLUTE){
			d = (nRotary==0) ? 0.3 : 0.5;		//radians, a tilt and a turn
			nRotary++;
		}
		if(oLink.constrained && (d<oLink.minimum || d>oLink.maximum))
			d = 0.5*(oLink.minimum+oLink.maximum);
		q[i] = d;
	}
	EXPECT_EQ((size_t)2,nRotary)<<"a five-axis machine has two rotary joints";
	return q;
}

//The same five joint values, as the machine sees them: a rotary axis is the same axis a full turn on,
//and the solver hands back one representative of each class of equivalents.
bool SameConfiguration(k5_chain chain, const double *a, const double *b)
{
	for(size_t i=0;i<(size_t)K5_AXES;i++){
		k5_link oLink;
		if(k5_chain_link(chain,i,&oLink)!=K5_OK)
			return false;
		double d = fabs(a[i]-b[i]);
		if(oLink.type==K5_LINK_REVOLUTE){
			const double d2Pi = 8.0*atan(1.0);
			d = fmod(d,d2Pi);
			d = std::min(d,d2Pi-d);
		}
		if(d>1E-9)
			return false;
	}
	return true;
}

//what a cutter location file can hold of a number: six decimals
double SixDecimals(double d)
{
	return floor(d*1E6+(d<0.0 ? -0.5 : 0.5))/1E6;
}

//writes three cutter locations dStep apart, all with the same tool axis
void WriteCLFile(const std::string &sPath, const double *p, const double *v, double dStep, size_t nCount)
{
	FILE *pFile = fopen(sPath.c_str(),"wt");
	ASSERT_TRUE(pFile!=NULL)<<"cannot write "<<sPath;
	for(size_t i=0;i<nCount;i++)
		fprintf(pFile,"GOTO/%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n"
			,p[0]+(double)i*dStep,p[1],p[2],v[0],v[1],v[2]);
	fclose(pFile);
}

std::string FirstLineOf(const std::string &sPath)
{
	FILE *pFile = fopen(sPath.c_str(),"rt");
	if(!pFile)
		return "";
	char szLine[512] = {0};
	if(!fgets(szLine,sizeof(szLine),pFile))
		szLine[0] = 0;
	fclose(pFile);
	return std::string(szLine);
}

} // namespace

// ---------------------------------------------------------------- the interface itself

TEST(Abi, TheHeaderIsCAndTheLibraryAnswersFromC)
{
	EXPECT_EQ(0,k5c_header_check_c())<<"the check that failed, see CHeaderCheck.c";
}

TEST(Abi, TheVersionAndTheLastError)
{
	EXPECT_STRNE("",k5_version_string());
	int nMajor=-1, nMinor=-1, nPatch=-1;
	k5_version(&nMajor,&nMinor,&nPatch);
	EXPECT_EQ(1,nMajor)<<"this is version 1 of the interface";
	EXPECT_LE(0,nMinor);
	EXPECT_LE(0,nPatch);
	EXPECT_EQ(std::string(k5_version_string()).substr(0,1),std::to_string(nMajor));

	//a call that succeeds leaves no message behind, a call that fails leaves one
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	size_t n = 0;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_link_count(NULL,&n));
	EXPECT_STRNE("",k5_last_error());
	k5_chain chain = OpenChain();
	EXPECT_EQ(K5_OK,k5_chain_link_count(chain,&n));
	EXPECT_STREQ("",k5_last_error())<<"a call that works says nothing";
	k5_chain_free(chain);
}

TEST(Abi, AFileThatIsNotAChainIsAnErrorWithAMessage)
{
	k5_chain chain = (k5_chain)0x1;
	EXPECT_EQ(K5_ERROR_FILE,k5_chain_open(ChainPath("there_is_no_such_machine.kc").c_str(),&chain));
	EXPECT_STRNE("",k5_last_error());
	EXPECT_TRUE(chain==NULL)<<"a failed open leaves no handle behind";
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_open(NULL,&chain));
	k5_chain_free(NULL);			//allowed, like free(NULL)
}

TEST(Abi, TheJointsOfAMachine)
{
	k5_chain chain = OpenChain();
	int bIs5C = 0;
	ASSERT_EQ(K5_OK,k5_chain_is_5c(chain,&bIs5C));
	EXPECT_EQ(1,bIs5C);
	size_t nLinks = 0;
	ASSERT_EQ(K5_OK,k5_chain_link_count(chain,&nLinks));
	ASSERT_EQ((size_t)K5_AXES,nLinks);

	char szName[K5_NAME_MAX] = {0};
	ASSERT_EQ(K5_OK,k5_chain_name(chain,szName,sizeof(szName)));
	EXPECT_STRNE("",szName)<<"the shipped models carry a name";

	size_t nPrismatic = 0, nRevolute = 0;
	for(size_t i=0;i<nLinks;i++){
		k5_link oLink;
		ASSERT_EQ(K5_OK,k5_chain_link(chain,i,&oLink))<<"joint "<<i;
		EXPECT_STRNE("",oLink.name)<<"joint "<<i<<" has a name in the file";
		if(oLink.type==K5_LINK_REVOLUTE){
			nRevolute++;
		} else {
			nPrismatic++;
		}
		if(oLink.constrained){
			EXPECT_LE(oLink.minimum,oLink.maximum)<<"joint "<<i;
		}
		EXPECT_EQ(0.0,oLink.value)<<"a chain just read stands at zero";
	}
	EXPECT_EQ((size_t)3,nPrismatic);
	EXPECT_EQ((size_t)2,nRevolute);
	k5_link oLink;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_link(chain,nLinks,&oLink))<<"past the last joint";

	//the first rotary axis is a different one from each end
	size_t nFromTool = 0, nFromPart = 0;
	ASSERT_EQ(K5_OK,k5_chain_first_rotary_axis(chain,K5_PART_TOOL,&nFromTool));
	ASSERT_EQ(K5_OK,k5_chain_first_rotary_axis(chain,K5_PART_MATERIAL_HOLDER,&nFromPart));
	EXPECT_NE(nFromTool,nFromPart);
	EXPECT_LT(nFromTool,(size_t)K5_AXES);
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_first_rotary_axis(chain,42,&nFromTool));
	k5_chain_free(chain);
}

TEST(Abi, TheFramesOfTheSegments)
{
	k5_chain chain = OpenChain();
	size_t nSegments = 0;
	ASSERT_EQ(K5_OK,k5_chain_frames(chain,K5_PART_TOOL,NULL,0,&nSegments))<<k5_last_error();
	ASSERT_GT(nSegments,(size_t)0)<<"the tool branch has segments";

	std::vector<double> v(nSegments*32,0.0);
	EXPECT_EQ(K5_ERROR_BUFFER_TOO_SMALL,k5_chain_frames(chain,K5_PART_TOOL,&v[0],v.size()-1,&nSegments));
	ASSERT_EQ(K5_OK,k5_chain_frames(chain,K5_PART_TOOL,&v[0],v.size(),&nSegments))<<k5_last_error();

	//the second frame of the last segment is the tool mount, in the layout OpenGL uses: three unit basis
	//vectors, the origin of the frame at 12..14 and 0,0,0,1 in the fourth column
	const double *m = &v[(nSegments-1)*32+16];
	for(int nBasis=0;nBasis<3;nBasis++){
		double dLength = 0.0;
		for(int k=0;k<3;k++)
			dLength += m[nBasis*4+k]*m[nBasis*4+k];
		EXPECT_NEAR(1.0,dLength,1E-12)<<"basis vector "<<nBasis<<" is a unit vector";
	}
	EXPECT_NEAR(0.0,m[3],1E-12);
	EXPECT_NEAR(0.0,m[7],1E-12);
	EXPECT_NEAR(0.0,m[11],1E-12);
	EXPECT_NEAR(1.0,m[15],1E-12);
	//the tool mount of this machine sits above the base, so the origin of that frame is not the origin
	EXPECT_GT(fabs(m[12])+fabs(m[13])+fabs(m[14]),1E-6)<<"the tool mount is somewhere";

	//and the frame the boxes of that segment live in differs from it only by the motion of its joint,
	//which is zero here: a chain just read stands at zero
	const double *mJoint = &v[(nSegments-1)*32];
	for(int k=0;k<16;k++)
		EXPECT_NEAR(mJoint[k],m[k],1E-12)<<"element "<<k<<": the terminal segment has no joint to move";
	k5_chain_free(chain);
}

TEST(Abi, ATool)
{
	k5_tool tool = CreateTool();
	ASSERT_TRUE(tool!=NULL);
	double dAssembly = 0.0, dMount = 0.0;
	ASSERT_EQ(K5_OK,k5_tool_assembly_length(tool,&dAssembly));
	ASSERT_EQ(K5_OK,k5_tool_mount_to_point_distance(tool,&dMount));
	EXPECT_NEAR(0.064+0.030+0.050,dAssembly,1E-12);
	EXPECT_NEAR(dAssembly-0.016,dMount,1E-12)<<"the ball centre is the programmed point";
	k5_tool_free(tool);

	//a flat end mill is programmed at its tip, so the whole assembly hangs below the mount
	k5_tool oFlat = NULL;
	ASSERT_EQ(K5_OK,k5_tool_create(K5_TOOL_FLAT_END,0.008,0.0,0.040,0.0,0.050,0.030,&oFlat));
	ASSERT_EQ(K5_OK,k5_tool_assembly_length(oFlat,&dAssembly));
	ASSERT_EQ(K5_OK,k5_tool_mount_to_point_distance(oFlat,&dMount));
	EXPECT_NEAR(dAssembly,dMount,1E-12);
	k5_tool_free(oFlat);

	//dimensions that do not make a tool come back as an argument error that says what is wrong
	k5_tool oBad = (k5_tool)0x1;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_create(K5_TOOL_BALL_NOSE,-0.016,0.016,0.064,0.0,0.0,0.0,&oBad));
	EXPECT_NE(std::string::npos,std::string(k5_last_error()).find("negative"))<<k5_last_error();
	EXPECT_TRUE(oBad==NULL);
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_create(K5_TOOL_BALL_NOSE,0.016,0.008,0.064,0.0,0.0,0.0,&oBad))
		<<"a ball narrower than the shank";
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_create(42,0.016,0.016,0.064,0.0,0.0,0.0,&oBad))<<"no such shape";
	k5_tool_free(NULL);
}

// ---------------------------------------------------------------- the solver through the interface

TEST(Abi, TheSolverFindsTheConfigurationTheMachineWasPutIn)
{
	k5_chain chain = OpenChain();
	k5_tool tool = CreateTool();
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	//the subject here is the interface, not the collision boxes of the model: a configuration that
	//reaches the location but has the holder in the table is a solution the solver is right to drop
	oSettings.check_collisions = 0;

	//forward: five joint values to a cutter location
	const std::vector<double> q = QInsideLimits(chain);
	double p[3] = {0,0,0}, v[3] = {0,0,0};
	ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,tool,&q[0],p,v))<<k5_last_error();
	EXPECT_NEAR(1.0,sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]),1E-12)<<"the tool axis is a unit vector";

	//backward: the same location to every configuration that reaches it
	size_t nCount = 0;
	int nStatus = -1;
	ASSERT_EQ(K5_OK,k5_solve_location(chain,tool,&oSettings,p,v,NULL,0,&nCount,&nStatus))<<k5_last_error();
	ASSERT_GT(nCount,(size_t)0)<<"the location the machine was standing at has to be solvable";
	EXPECT_EQ(K5_SOLVED,nStatus);

	std::vector<double> vQ(nCount*(size_t)K5_AXES,0.0);
	EXPECT_EQ(K5_ERROR_BUFFER_TOO_SMALL
		,k5_solve_location(chain,tool,&oSettings,p,v,&vQ[0],nCount-1,&nCount,&nStatus));
	ASSERT_EQ(K5_OK,k5_solve_location(chain,tool,&oSettings,p,v,&vQ[0],nCount,&nCount,&nStatus));

	//one of them is the configuration the machine was put in, and every one of them reaches the location
	bool bFoundTheOriginal = false;
	for(size_t s=0;s<nCount;s++){
		const double *pSolution = &vQ[s*(size_t)K5_AXES];
		if(SameConfiguration(chain,pSolution,&q[0]))
			bFoundTheOriginal = true;

		double p2[3] = {0,0,0}, v2[3] = {0,0,0};
		ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,tool,pSolution,p2,v2));
		for(int k=0;k<3;k++){
			EXPECT_NEAR(p[k],p2[k],1E-9)<<"solution "<<s<<", coordinate "<<k;
			EXPECT_NEAR(v[k],v2[k],1E-9)<<"solution "<<s<<", axis "<<k;
		}
	}
	EXPECT_TRUE(bFoundTheOriginal)<<"the solver found "<<nCount<<" configurations and none of them is the one asked for";

	k5_tool_free(tool);
	k5_chain_free(chain);
}

TEST(Abi, ALocationThatCannotBeReachedIsNoError)
{
	k5_chain chain = OpenChain();
	k5_tool tool = CreateTool();
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	const double p[3] = {1000.0,0.0,0.0};		//a kilometre away from any machine
	const double v[3] = {0.3,0.4,0.866025};
	size_t nCount = 1;
	int nStatus = -1;
	EXPECT_EQ(K5_OK,k5_solve_location(chain,tool,&oSettings,p,v,NULL,0,&nCount,&nStatus))<<k5_last_error();
	EXPECT_EQ((size_t)0,nCount);
	EXPECT_NE(K5_SOLVED,nStatus)<<"and the status says at which stage it was lost";
	EXPECT_STREQ("",k5_last_error())<<"an unreachable location is an answer, not a failure";

	//the collision check needs a tool, and says so instead of crashing
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_solve_location(chain,NULL,&oSettings,p,v,NULL,0,&nCount,&nStatus));
	oSettings.check_collisions = 0;
	EXPECT_EQ(K5_OK,k5_solve_location(chain,NULL,&oSettings,p,v,NULL,0,&nCount,&nStatus))<<k5_last_error();
	k5_tool_free(tool);
	k5_chain_free(chain);
}

// ---------------------------------------------------------------- a whole file

TEST(Abi, APathIsSolvedAndWrittenAsGCode)
{
	k5_chain chain = OpenChain();
	k5_tool tool = CreateTool();
	k5_settings oSettings;
	k5_settings_default(&oSettings);

	//three cutter locations the machine can reach, taken from a configuration it can stand in
	const std::vector<double> q = QInsideLimits(chain);
	double p[3] = {0,0,0}, v[3] = {0,0,0};
	ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,tool,&q[0],p,v));
	const std::string sCL = TempPath("kin5axcTests_path.cld");
	WriteCLFile(sCL,p,v,0.025,3);		//0.025 apart, the interpolation limit is 0.01: three steps each

	k5_path path = NULL;
	ASSERT_EQ(K5_OK,k5_postprocess_file(chain,tool,&oSettings,sCL.c_str(),&path))<<k5_last_error();
	ASSERT_TRUE(path!=NULL);

	size_t nSize = 0;
	ASSERT_EQ(K5_OK,k5_path_size(path,&nSize));
	EXPECT_EQ((size_t)7,nSize)<<"the first location, then two steps and a location, twice";
	size_t nFromFile = 0, nInterpolated = 0, nSolved = 0;
	ASSERT_EQ(K5_OK,k5_path_count_of_origin(path,K5_FROM_FILE,&nFromFile));
	ASSERT_EQ(K5_OK,k5_path_count_of_origin(path,K5_INTERPOLATED,&nInterpolated));
	ASSERT_EQ(K5_OK,k5_path_count_of_status(path,K5_SOLVED,&nSolved));
	EXPECT_EQ((size_t)3,nFromFile);
	EXPECT_EQ((size_t)4,nInterpolated);
	EXPECT_EQ(nSize,nSolved)<<"every location of this path is solvable";
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_count_of_origin(path,42,&nFromFile));

	//the records: the line of the file, the cutter location they belong to, and the solution
	size_t nFileSeen = 0;
	for(size_t i=0;i<nSize;i++){
		k5_location oLocation;
		ASSERT_EQ(K5_OK,k5_path_location(path,i,&oLocation))<<"record "<<i;
		EXPECT_EQ(1,oLocation.has_solution)<<"record "<<i;
		EXPECT_GT(oLocation.file_line,(size_t)0)<<"record "<<i<<" knows its line of the file";
		if(oLocation.origin==K5_FROM_FILE)
			nFileSeen++;
		EXPECT_EQ(nFileSeen,oLocation.file_location)<<"record "<<i;
	}
	EXPECT_EQ((size_t)3,nFileSeen);
	k5_location oIgnored;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_location(path,nSize,&oIgnored));

	//the solutions are exact
	double dPositionError = -1.0, dOrientationError = -1.0;
	ASSERT_EQ(K5_OK,k5_path_errors(path,&dPositionError,&dOrientationError));
	EXPECT_LT(dPositionError,1E-9);
	EXPECT_LT(dOrientationError,1E-9);

	//where the machine is left standing
	double qLast[K5_AXES] = {0,0,0,0,0};
	ASSERT_EQ(K5_OK,k5_path_last_configuration(path,qLast));
	k5_location oLast;
	ASSERT_EQ(K5_OK,k5_path_location(path,nSize-1,&oLast));
	for(size_t i=0;i<(size_t)K5_AXES;i++)
		EXPECT_NEAR(oLast.q[i],qLast[i],1E-12)<<"axis "<<i;

	//and the G-code
	k5_gcode_axes oAxes;
	ASSERT_EQ(K5_OK,k5_chain_default_gcode_axes(chain,&oAxes))<<k5_last_error();
	EXPECT_EQ(1,oAxes.degrees);
	EXPECT_EQ('A',oAxes.angle1_letter);
	const std::string sGCode = TempPath("kin5axcTests_path.nc");
	ASSERT_EQ(K5_OK,k5_path_write_gcode(path,&oAxes,sGCode.c_str()))<<k5_last_error();
	const std::string sFirst = FirstLineOf(sGCode);
	EXPECT_EQ((size_t)0,sFirst.find("N10 G1 X"))<<sFirst;
	EXPECT_NE(std::string::npos,sFirst.find(" A"))<<sFirst;
	EXPECT_NE(std::string::npos,sFirst.find(" C"))<<sFirst;

	//a letter the control does not have is refused, and so is an axis that is not one of the five
	oAxes.angle1_letter = 0;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_write_gcode(path,&oAxes,sGCode.c_str()));
	oAxes.angle1_letter = 'A';
	oAxes.x = (size_t)K5_AXES;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_write_gcode(path,&oAxes,sGCode.c_str()));

	remove(sGCode.c_str());
	remove(sCL.c_str());
	k5_path_free(path);
	k5_path_free(NULL);
	k5_tool_free(tool);
	k5_chain_free(chain);
}

TEST(Abi, TheInterpolationAndTheStartOfTheTrajectoryAreSettings)
{
	k5_chain chain = OpenChain();
	k5_tool tool = CreateTool();
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	const std::vector<double> q = QInsideLimits(chain);
	double p[3] = {0,0,0}, v[3] = {0,0,0};
	ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,tool,&q[0],p,v));
	const std::string sCL = TempPath("kin5axcTests_modes.cld");
	WriteCLFile(sCL,p,v,0.025,3);

	//nothing invented between the locations of the file
	oSettings.interpolate = 0;
	k5_path path = NULL;
	ASSERT_EQ(K5_OK,k5_postprocess_file(chain,tool,&oSettings,sCL.c_str(),&path))<<k5_last_error();
	size_t nSize = 0, nFromFile = 0;
	ASSERT_EQ(K5_OK,k5_path_size(path,&nSize));
	ASSERT_EQ(K5_OK,k5_path_count_of_origin(path,K5_FROM_FILE,&nFromFile));
	EXPECT_EQ((size_t)3,nSize);
	EXPECT_EQ(nSize,nFromFile);
	k5_path_free(path);

	//the start of the trajectory: the rotary axis nearest the tool, as close to zero as it gets
	size_t nAxis = 0;
	ASSERT_EQ(K5_OK,k5_chain_first_rotary_axis(chain,K5_PART_TOOL,&nAxis));
	oSettings.start_criterion = K5_START_MIN_ABS;
	path = NULL;
	ASSERT_EQ(K5_OK,k5_postprocess_file(chain,tool,&oSettings,sCL.c_str(),&path))<<k5_last_error();
	k5_location oFirstMinAbs;
	ASSERT_EQ(K5_OK,k5_path_location(path,0,&oFirstMinAbs));
	k5_path_free(path);

	//the same path with the largest value of that axis instead: a different start, unless the location
	//has only one solution
	oSettings.start_criterion = K5_START_MAX;
	path = NULL;
	ASSERT_EQ(K5_OK,k5_postprocess_file(chain,tool,&oSettings,sCL.c_str(),&path))<<k5_last_error();
	k5_location oFirstMax;
	ASSERT_EQ(K5_OK,k5_path_location(path,0,&oFirstMax));
	EXPECT_LE(fabs(oFirstMinAbs.q[nAxis]),fabs(oFirstMax.q[nAxis])+1E-12)
		<<"the closest to zero is not further from it than the largest";
	EXPECT_LE(oFirstMinAbs.q[nAxis],oFirstMax.q[nAxis]+1E-12);
	k5_path_free(path);

	//a criterion that does not exist is an error
	oSettings.start_criterion = 42;
	path = NULL;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_postprocess_file(chain,tool,&oSettings,sCL.c_str(),&path));
	EXPECT_TRUE(path==NULL);

	remove(sCL.c_str());
	k5_tool_free(tool);
	k5_chain_free(chain);
}

TEST(Abi, ANullArgumentIsAnErrorEverywhere)
{
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	const double p[3] = {0,0,0}, v[3] = {0,0,1};
	size_t n = 0;
	int nStatus = 0;
	k5_link oLink;
	k5_location oLocation;
	k5_gcode_axes oAxes;

	int bFlag = 0;
	double dLength = 0.0;
	k5_settings_default(NULL);		//allowed, and does nothing
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_is_5c(NULL,&bFlag));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_is_5c(NULL,NULL));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_link(NULL,0,&oLink));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_frames(NULL,K5_PART_TOOL,NULL,0,&n));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_tool_point(NULL,NULL,NULL,NULL,NULL));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_default_gcode_axes(NULL,&oAxes));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_assembly_length(NULL,&dLength));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_solve_location(NULL,NULL,&oSettings,p,v,NULL,0,&n,&nStatus));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_postprocess_file(NULL,NULL,&oSettings,"nowhere.cld",NULL));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_size(NULL,&n));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_location(NULL,0,&oLocation));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_errors(NULL,NULL,NULL));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_last_configuration(NULL,oLocation.q));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_write_gcode(NULL,&oAxes,"nowhere.nc"));
	EXPECT_STRNE("",k5_last_error());
}

// ---------------------------------------------------------------- a path handed over, not read

TEST(Abi, APathInMemoryGivesWhatTheSamePathInAFileGives)
{
	k5_chain chain = OpenChain();
	k5_tool tool = CreateTool();
	k5_settings oSettings;
	k5_settings_default(&oSettings);

	//Three locations the machine can reach, as a host would have them: two plain arrays. A cutter
	//location file carries six decimals, so the numbers are rounded to that before either run sees
	//them - otherwise the two runs are not given the same path and there is nothing to compare.
	const std::vector<double> q = QInsideLimits(chain);
	double p[3] = {0,0,0}, v[3] = {0,0,0};
	ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,tool,&q[0],p,v));
	for(int k=0;k<3;k++){
		p[k] = SixDecimals(p[k]);
		v[k] = SixDecimals(v[k]);
	}
	const size_t nCount = 3;
	const double dStep = 0.025;
	std::vector<double> vPositions, vAxes;
	for(size_t i=0;i<nCount;i++){
		vPositions.push_back(p[0]+(double)i*dStep);
		vPositions.push_back(p[1]);
		vPositions.push_back(p[2]);
		vAxes.push_back(v[0]);
		vAxes.push_back(v[1]);
		vAxes.push_back(v[2]);
	}
	//and the same three in a file
	const std::string sCL = TempPath("kin5axcTests_inmemory.cld");
	WriteCLFile(sCL,p,v,dStep,nCount);

	k5_path pathFile = NULL, pathMemory = NULL;
	ASSERT_EQ(K5_OK,k5_postprocess_file(chain,tool,&oSettings,sCL.c_str(),&pathFile))<<k5_last_error();
	ASSERT_EQ(K5_OK,k5_postprocess_locations(chain,tool,&oSettings,&vPositions[0],&vAxes[0],nCount,&pathMemory))
		<<k5_last_error();

	size_t nFromFile = 0, nFromMemory = 0;
	ASSERT_EQ(K5_OK,k5_path_size(pathFile,&nFromFile));
	ASSERT_EQ(K5_OK,k5_path_size(pathMemory,&nFromMemory));
	EXPECT_EQ((size_t)7,nFromFile)<<"the first location, then two steps and a location, twice";
	ASSERT_EQ(nFromFile,nFromMemory);
	for(size_t i=0;i<nFromFile;i++){
		k5_location a, b;
		ASSERT_EQ(K5_OK,k5_path_location(pathFile,i,&a));
		ASSERT_EQ(K5_OK,k5_path_location(pathMemory,i,&b));
		EXPECT_EQ(a.status,b.status)<<"record "<<i;
		EXPECT_EQ(a.origin,b.origin)<<"record "<<i;
		EXPECT_EQ(a.file_location,b.file_location)<<"record "<<i;
		EXPECT_EQ(a.file_line,b.file_line)<<"record "<<i<<": a location handed over is numbered the way"
			" the line of a file is";
		EXPECT_EQ(a.has_solution,b.has_solution)<<"record "<<i;
		if(a.has_solution){
			for(size_t k=0;k<(size_t)K5_AXES;k++)
				EXPECT_NEAR(a.q[k],b.q[k],1E-12)<<"record "<<i<<", axis "<<k;
		}
	}
	remove(sCL.c_str());
	k5_path_free(pathFile);
	k5_path_free(pathMemory);
	k5_tool_free(tool);
	k5_chain_free(chain);
}

TEST(Abi, APathHandedOverIsCheckedBeforeItIsSolved)
{
	k5_chain chain = OpenChain();
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	oSettings.check_collisions = 0;				//no tool needed, so the arrays are the only subject
	const double vPositions[6] = {0.1,0.2,0.03, 0.12,0.2,0.03};
	const double vGoodAxes[6] = {0.3,0.4,0.866025, 0.3,0.4,0.866025};
	const double vZeroAxis[6] = {0.3,0.4,0.866025, 0.0,0.0,0.0};
	k5_path path = (k5_path)0x1;

	//an axis of no length is no direction, and the message says which location it was
	EXPECT_EQ(K5_ERROR_ARGUMENT
		,k5_postprocess_locations(chain,NULL,&oSettings,vPositions,vZeroAxis,2,&path));
	EXPECT_NE(std::string::npos,std::string(k5_last_error()).find("location 2"))<<k5_last_error();
	EXPECT_TRUE(path==NULL);

	//no locations at all is a path of none, not a failure
	ASSERT_EQ(K5_OK,k5_postprocess_locations(chain,NULL,&oSettings,NULL,NULL,0,&path))<<k5_last_error();
	size_t nSize = 1;
	ASSERT_EQ(K5_OK,k5_path_size(path,&nSize));
	EXPECT_EQ((size_t)0,nSize);
	double qLast[K5_AXES] = {0,0,0,0,0};
	EXPECT_EQ(K5_ERROR_SOLVER,k5_path_last_configuration(path,qLast))<<"an empty path leaves the machine nowhere";
	k5_path_free(path);

	//the locations themselves cannot be missing when there are some to solve
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_postprocess_locations(chain,NULL,&oSettings,NULL,vGoodAxes,2,&path));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_postprocess_locations(chain,NULL,&oSettings,vPositions,NULL,2,&path));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_postprocess_locations(NULL,NULL,&oSettings,vPositions,vGoodAxes,2,&path));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_postprocess_locations(chain,NULL,NULL,vPositions,vGoodAxes,2,&path));
	k5_chain_free(chain);
}
