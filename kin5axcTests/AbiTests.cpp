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
	const k5_status n = k5_tool_create(K5_TOOL_BALL_NOSE,0.016,0.016,0.064,0.030,0.050,0.030,0.0,&tool);
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
	EXPECT_LE(0,nMajor);
	EXPECT_LE(0,nMinor);
	EXPECT_LE(0,nPatch);
	//the library answers with its own version, and the header this test was built against repeats
	//those numbers by hand - the two have to agree, or a host checking the header checks nothing
	EXPECT_EQ(K5_VERSION_MAJOR,nMajor)<<"kin5ax.h and kin5ax/Version.h disagree about the major";
	EXPECT_EQ(K5_VERSION_MINOR,nMinor)<<"kin5ax.h and kin5ax/Version.h disagree about the minor";
	EXPECT_EQ(K5_VERSION_PATCH,nPatch)<<"kin5ax.h and kin5ax/Version.h disagree about the patch";
	EXPECT_EQ(std::string(K5_VERSION_STRING),std::string(k5_version_string()));
	char szExpected[32];
	snprintf(szExpected,sizeof(szExpected),"%d.%d.%d",nMajor,nMinor,nPatch);
	EXPECT_EQ(std::string(szExpected),std::string(k5_version_string()))<<"the string is the numbers";

	//the commit is build metadata: it travels with a shipped binary so that it can be traced back,
	//and takes no part in the version a host checks for compatibility
	ASSERT_STRNE("",k5_commit_hash());
	EXPECT_EQ(std::string(k5_version_string())+"+"+k5_commit_hash(),std::string(k5_build_string()));
	EXPECT_EQ(std::string::npos,std::string(k5_version_string()).find('+'))
		<<"the version a host compares has no metadata in it";

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

	//the unit every length of it is in, and what a host multiplies by: the shipped models are metres
	int nUnit = -1;
	ASSERT_EQ(K5_OK,k5_chain_unit(chain,&nUnit));
	EXPECT_EQ(K5_UNIT_M,nUnit);
	double dMillimetres = 0.0;
	ASSERT_EQ(K5_OK,k5_unit_in_millimetres(nUnit,&dMillimetres));
	EXPECT_DOUBLE_EQ(1000.0,dMillimetres);
	ASSERT_EQ(K5_OK,k5_unit_in_millimetres(K5_UNIT_MM,&dMillimetres));
	EXPECT_DOUBLE_EQ(1.0,dMillimetres);
	ASSERT_EQ(K5_OK,k5_unit_in_millimetres(K5_UNIT_IN,&dMillimetres));
	EXPECT_DOUBLE_EQ(25.4,dMillimetres);
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_unit_in_millimetres(42,&dMillimetres))<<"not a unit";
	//that a file saying m comes back as K5_UNIT_M is the check that the two enumerations agree: these
	//tests see the shared library alone, never the C++ headers, and that is the point of them

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

TEST(Abi, TheBoxesOfTheSegments)
{
	//a model with boxes: the holder of its tool and the holder of its material each carry one
	k5_chain chain = OpenChain("Reiden_RX10_A_SwivelHead_C_RotaryTable.kc.json");
	size_t nTotal = 0;
	const int nParts[2] = {K5_PART_TOOL,K5_PART_MATERIAL_HOLDER};
	for(int p=0;p<2;p++){
		size_t nSegments = 0;
		ASSERT_EQ(K5_OK,k5_chain_frames(chain,nParts[p],NULL,0,&nSegments))<<k5_last_error();
		for(size_t i=0;i<nSegments;i++){
			size_t nBoxes = 0;
			ASSERT_EQ(K5_OK,k5_chain_segment_boxes(chain,nParts[p],i,NULL,0,&nBoxes))<<k5_last_error();
			std::vector<double> v(6*nBoxes+1,0.0);
			size_t nAgain = 0;
			ASSERT_EQ(K5_OK,k5_chain_segment_boxes(chain,nParts[p],i,&v[0],6*nBoxes,&nAgain))<<k5_last_error();
			EXPECT_EQ(nBoxes,nAgain);
			for(size_t b=0;b<nBoxes;b++)
				for(int k=0;k<3;k++)
					EXPECT_LE(v[6*b+k],v[6*b+3+k])<<"the minimum corner first";
			nTotal += nBoxes;
		}
	}
	EXPECT_EQ((size_t)2,nTotal)<<"one box on each terminal of this model";

	//the last segment of the tool branch is its holder, which has the box: too small a buffer says so
	size_t nSegments = 0, nBoxes = 0;
	ASSERT_EQ(K5_OK,k5_chain_frames(chain,K5_PART_TOOL,NULL,0,&nSegments));
	double d[1] = {0.0};
	EXPECT_EQ(K5_ERROR_BUFFER_TOO_SMALL,k5_chain_segment_boxes(chain,K5_PART_TOOL,nSegments-1,d,1,&nBoxes));
	EXPECT_EQ((size_t)1,nBoxes)<<"and the count is given anyway";
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_segment_boxes(chain,K5_PART_TOOL,nSegments,NULL,0,&nBoxes));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_segment_boxes(chain,42,0,NULL,0,&nBoxes));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_segment_boxes(NULL,K5_PART_TOOL,0,NULL,0,&nBoxes));
	k5_chain_free(chain);
}

TEST(Abi, TheConfigurationCanBeSetAndTheFramesFollow)
{
	k5_chain chain = OpenChain();
	const std::vector<double> q = QInsideLimits(chain);
	ASSERT_EQ(K5_OK,k5_chain_set_configuration(chain,&q[0]))<<k5_last_error();
	for(size_t i=0;i<(size_t)K5_AXES;i++){
		k5_link oLink;
		ASSERT_EQ(K5_OK,k5_chain_link(chain,i,&oLink));
		EXPECT_NEAR(q[i],oLink.value,1E-12)<<"joint "<<i<<" stands where it was put";
	}
	size_t n = 0;
	ASSERT_EQ(K5_OK,k5_chain_frames(chain,K5_PART_TOOL,NULL,0,&n));
	std::vector<double> vBefore(32*n,0.0), vAfter(32*n,0.0);
	ASSERT_EQ(K5_OK,k5_chain_frames(chain,K5_PART_TOOL,&vBefore[0],vBefore.size(),&n));
	std::vector<double> q2 = q;
	for(size_t i=0;i<q2.size();i++)
		q2[i] += 0.01;
	ASSERT_EQ(K5_OK,k5_chain_set_configuration(chain,&q2[0]))<<k5_last_error();
	ASSERT_EQ(K5_OK,k5_chain_frames(chain,K5_PART_TOOL,&vAfter[0],vAfter.size(),&n));
	EXPECT_NE(vBefore,vAfter)<<"the segments moved with their joints";
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_set_configuration(chain,NULL));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_set_configuration(NULL,&q[0]));
	k5_chain_free(chain);
}

// ---------------------------------------------------------------- the cutting simulation

TEST(Abi, ABlockOfMaterialAndWhatIsLeftOfIt)
{
	k5_material material = NULL;
	//a block of 0.1 x 0.1 x 0.05 in voxels of 0.005: 21 x 21 x 11 of them, more or less
	ASSERT_EQ(K5_OK,k5_material_create(0.1,0.1,0.05,0.005,-0.05,-0.05,0.0,&material))<<k5_last_error();
	size_t nx=0, ny=0, nz=0;
	ASSERT_EQ(K5_OK,k5_material_dimensions(material,&nx,&ny,&nz));
	EXPECT_GE(nx,(size_t)20);
	EXPECT_GE(nz,(size_t)10);
	double dTotal=0.0, dRemoved=1.0;
	ASSERT_EQ(K5_OK,k5_material_volumes(material,&dTotal,&dRemoved));
	EXPECT_GT(dTotal,0.0);
	EXPECT_EQ(0.0,dRemoved)<<"nothing is cut yet";
	//the surface of a whole block is its six faces: every voxel on the outside, none inside
	size_t nSurface = 0;
	ASSERT_EQ(K5_OK,k5_material_surface(material,NULL,0,&nSurface));
	EXPECT_GT(nSurface,(size_t)0);
	EXPECT_LT(nSurface,nx*ny*nz)<<"the inside is not surface";
	std::vector<double> v(3*nSurface,0.0);
	size_t nAgain = 0;
	ASSERT_EQ(K5_OK,k5_material_surface(material,&v[0],v.size(),&nAgain));
	EXPECT_EQ(nSurface,nAgain);
	for(size_t i=0;i<nSurface;i++){
		EXPECT_GE(v[3*i],-0.05-1E-9);
		EXPECT_LE(v[3*i],0.05+0.005+1E-9);
		EXPECT_GE(v[3*i+2],-1E-9);
	}
	double d[1] = {0.0};
	EXPECT_EQ(K5_ERROR_BUFFER_TOO_SMALL,k5_material_surface(material,d,1,&nAgain));
	k5_material_free(material);

	k5_material bad = NULL;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_material_create(0.0,0.1,0.05,0.005,0.0,0.0,0.0,&bad));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_material_create(0.1,0.1,0.05,0.0,0.0,0.0,0.0,&bad));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_material_create(1.0,1.0,1.0,1E-6,0.0,0.0,0.0,&bad))<<"too many voxels";
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_material_create(0.1,0.1,0.05,0.005,0.0,0.0,0.0,NULL));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_material_volumes(NULL,&dTotal,&dRemoved));
	k5_material_free(NULL);		//allowed
}

TEST(Abi, TheSurfaceOfTheMaterialAsTriangles)
{
	//a block of 10 x 5 x 2 voxels: its outside is 2(10*5 + 5*2 + 10*2) = 160 faces, no more
	k5_material material = NULL;
	ASSERT_EQ(K5_OK,k5_material_create(0.10,0.05,0.02,0.01,0.0,0.0,0.0,&material))<<k5_last_error();
	size_t nx=0, ny=0, nz=0;
	ASSERT_EQ(K5_OK,k5_material_dimensions(material,&nx,&ny,&nz));
	const size_t nExpected = 2*(nx*ny+ny*nz+nx*nz);
	size_t nFaces = 0;
	ASSERT_EQ(K5_OK,k5_material_surface_triangles(material,NULL,0,&nFaces))<<k5_last_error();
	EXPECT_EQ(nExpected,nFaces);
	std::vector<double> v(18*nFaces,0.0);
	size_t nAgain = 0;
	ASSERT_EQ(K5_OK,k5_material_surface_triangles(material,&v[0],v.size(),&nAgain));
	EXPECT_EQ(nFaces,nAgain);
	//every corner on the block, and every triangle's normal pointing out of it
	const double dX = (double)nx*0.01, dY = (double)ny*0.01, dZ = (double)nz*0.01;
	for(size_t f=0;f<nFaces;f++){
		for(int t=0;t<2;t++){
			const double *a = &v[18*f+9*t], *b = a+3, *c = a+6;
			for(int k=0;k<3;k++){
				EXPECT_GE(a[k],-1E-12);
			}
			EXPECT_LE(a[0],dX+1E-12);
			EXPECT_LE(a[1],dY+1E-12);
			EXPECT_LE(a[2],dZ+1E-12);
			const double n[3] = {(b[1]-a[1])*(c[2]-a[2])-(b[2]-a[2])*(c[1]-a[1])
				,(b[2]-a[2])*(c[0]-a[0])-(b[0]-a[0])*(c[2]-a[2])
				,(b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])};
			const double m[3] = {(a[0]+b[0]+c[0])/3.0-dX/2.0,(a[1]+b[1]+c[1])/3.0-dY/2.0,(a[2]+b[2]+c[2])/3.0-dZ/2.0};
			EXPECT_GT(n[0]*m[0]+n[1]*m[1]+n[2]*m[2],0.0)<<"face "<<f<<" triangle "<<t<<" faces outwards";
		}
	}
	double d[1] = {0.0};
	EXPECT_EQ(K5_ERROR_BUFFER_TOO_SMALL,k5_material_surface_triangles(material,d,1,&nAgain));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_material_surface_triangles(NULL,NULL,0,&nAgain));
	k5_material_free(material);
}

TEST(Abi, OneStampOfTheToolWhereverItIsPut)
{
	//what a host does for a location its simulation will not cut: put the tool there and stamp it
	k5_tool tool = CreateTool();
	k5_material material = NULL;
	ASSERT_EQ(K5_OK,k5_material_create(0.12,0.06,0.03,0.002,-0.06,-0.03,-0.03,&material))<<k5_last_error();
	double dTotal=0.0, dRemoved=0.0;
	ASSERT_EQ(K5_OK,k5_material_volumes(material,&dTotal,&dRemoved));
	EXPECT_EQ(0.0,dRemoved);

	const double p[3] = {0.0,0.0,0.0};		//the ball centre on the top of the block
	const double v[3] = {0.0,0.0,1.0};
	int nGouged = 1;
	ASSERT_EQ(K5_OK,k5_material_cut(material,tool,p,v,&nGouged,NULL))<<k5_last_error();
	EXPECT_EQ(0,nGouged)<<"the shank is above the block";
	ASSERT_EQ(K5_OK,k5_material_volumes(material,&dTotal,&dRemoved));
	EXPECT_GT(dRemoved,0.0)<<"a bite out of it";
	const double dOne = dRemoved;

	//the same stamp again takes nothing more: a voxel removed is removed
	ASSERT_EQ(K5_OK,k5_material_cut(material,tool,p,v,NULL,NULL));
	ASSERT_EQ(K5_OK,k5_material_volumes(material,&dTotal,&dRemoved));
	EXPECT_EQ(dOne,dRemoved);

	//and one pushed into the block has its shank in material that is still there - a tool whose
	//cutting part is 20 mm, since the 64 mm one of CreateTool reaches out of a block 30 mm deep
	k5_tool stubby = NULL;
	ASSERT_EQ(K5_OK,k5_tool_create(K5_TOOL_BALL_NOSE,0.016,0.016,0.020,0.030,0.050,0.030,0.0,&stubby))
		<<k5_last_error();
	const double deep[3] = {0.04,0.0,-0.02};
	double gouge[3] = {0.0,0.0,0.0};
	ASSERT_EQ(K5_OK,k5_material_cut(material,stubby,deep,v,&nGouged,gouge))<<k5_last_error();
	EXPECT_EQ(1,nGouged);
	EXPECT_GT(gouge[2],-0.03)<<"and the place of it is in the block";
	k5_tool_free(stubby);

	const double zero[3] = {0.0,0.0,0.0};
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_material_cut(material,tool,p,zero,NULL,NULL))<<"no direction";
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_material_cut(NULL,tool,p,v,NULL,NULL));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_material_cut(material,NULL,p,v,NULL,NULL));
	k5_material_free(material);
	k5_tool_free(tool);
}

TEST(Abi, ThePathCutsTheMaterialWhereItGoes)
{
	k5_chain chain = OpenChain();
	k5_tool tool = CreateTool();
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	oSettings.check_collisions = 0;		//the block is not in the chain's model of itself

	//a path across the top of a block: the programmed point of the ball nose on the top face itself,
	//so the ball dips in and cuts a groove a radius deep. The tool stands up, the material frame being
	//the frame the path is in.
	const double dTop = 0.0, dRadius = 0.016;
	std::vector<double> vPositions, vAxes;
	const size_t nCount = 5;
	for(size_t i=0;i<nCount;i++){
		vPositions.push_back(-0.04+0.02*(double)i);
		vPositions.push_back(0.0);
		vPositions.push_back(dTop);
		vAxes.push_back(0.0);
		vAxes.push_back(0.0);
		vAxes.push_back(1.0);
	}
	k5_path path = NULL;
	ASSERT_EQ(K5_OK,k5_postprocess_locations(chain,tool,&oSettings,&vPositions[0],&vAxes[0],nCount,&path))
		<<k5_last_error();
	size_t nSolved = 0;
	ASSERT_EQ(K5_OK,k5_path_count_of_status(path,K5_SOLVED,&nSolved));
	ASSERT_GT(nSolved,(size_t)0)<<"the path has to be reachable for this to mean anything";

	k5_material material = NULL;
	ASSERT_EQ(K5_OK,k5_material_create(0.12,0.06,0.03,0.002,-0.06,-0.03,dTop-0.03,&material))<<k5_last_error();
	k5_simulation simulation = NULL;
	ASSERT_EQ(K5_OK,k5_simulation_create(chain,tool,path,material,&simulation))<<k5_last_error();

	size_t nIndex=99, nSteps=0;
	int nFinished = 1;
	ASSERT_EQ(K5_OK,k5_simulation_progress(simulation,&nIndex,&nSteps,&nFinished));
	EXPECT_EQ((size_t)0,nIndex);
	size_t nPathSize = 0;
	ASSERT_EQ(K5_OK,k5_path_size(path,&nPathSize));
	EXPECT_EQ(nPathSize,nSteps)<<"a step per location of the path";
	EXPECT_EQ(0,nFinished);

	//one step cuts something, and the material says so
	int nStepped = 0;
	ASSERT_EQ(K5_OK,k5_simulation_step(simulation,&nStepped))<<k5_last_error();
	EXPECT_EQ(1,nStepped);
	double dTotal=0.0, dRemoved=0.0;
	ASSERT_EQ(K5_OK,k5_material_volumes(material,&dTotal,&dRemoved));
	EXPECT_GT(dRemoved,0.0)<<"the ball took a bite";
	EXPECT_LT(dRemoved,dTotal);
	const double dAfterOne = dRemoved;

	//the rest of the way, and more is gone, but nothing below the reach of the ball
	ASSERT_EQ(K5_OK,k5_simulation_run(simulation));
	ASSERT_EQ(K5_OK,k5_simulation_progress(simulation,&nIndex,&nSteps,&nFinished));
	EXPECT_EQ(nSteps,nIndex);
	EXPECT_EQ(1,nFinished);
	ASSERT_EQ(K5_OK,k5_material_volumes(material,&dTotal,&dRemoved));
	EXPECT_GT(dRemoved,dAfterOne);
	size_t nSurface = 0;
	ASSERT_EQ(K5_OK,k5_material_surface(material,NULL,0,&nSurface));
	std::vector<double> v(3*nSurface,0.0);
	ASSERT_EQ(K5_OK,k5_material_surface(material,&v[0],v.size(),&nSurface));
	//the top of what is left: under the path it is the floor of the groove, a radius down; beside the
	//path, further than the ball reaches, it is still the top of the block
	double dUnderThePath = -1.0, dBesideIt = -1.0;
	for(size_t i=0;i<nSurface;i++){
		if(fabs(v[3*i])>0.03)
			continue;
		if(fabs(v[3*i+1])<0.001)
			dUnderThePath = std::max(dUnderThePath,v[3*i+2]);
		if(fabs(v[3*i+1]-0.025)<0.0015)
			dBesideIt = std::max(dBesideIt,v[3*i+2]);
	}
	EXPECT_LT(dUnderThePath,dTop-0.5*dRadius)<<"a groove under the path";
	EXPECT_GT(dUnderThePath,dTop-dRadius-0.004)<<"no deeper than the ball reaches, to a voxel";
	EXPECT_GT(dBesideIt,dTop-0.004)<<"and nothing taken off beside it";

	//no gouge: the shank and the holder stayed above the block
	size_t nGouges = 99;
	ASSERT_EQ(K5_OK,k5_simulation_gouges(simulation,&nGouges,NULL,NULL));
	EXPECT_EQ((size_t)0,nGouges);

	//a step past the end does nothing and says so; a reset starts again on the same, cut, material
	ASSERT_EQ(K5_OK,k5_simulation_step(simulation,&nStepped));
	EXPECT_EQ(0,nStepped);
	ASSERT_EQ(K5_OK,k5_simulation_reset(simulation));
	ASSERT_EQ(K5_OK,k5_simulation_progress(simulation,&nIndex,NULL,&nFinished));
	EXPECT_EQ((size_t)0,nIndex);
	ASSERT_EQ(K5_OK,k5_material_reset(material));
	ASSERT_EQ(K5_OK,k5_material_volumes(material,&dTotal,&dRemoved));
	EXPECT_EQ(0.0,dRemoved)<<"whole again";

	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_simulation_create(NULL,tool,path,material,&simulation));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_simulation_create(chain,tool,path,NULL,&simulation));
	k5_simulation_free(simulation);
	k5_simulation_free(NULL);		//allowed
	k5_material_free(material);
	k5_path_free(path);
	k5_tool_free(tool);
	k5_chain_free(chain);
}

TEST(Abi, ALeaningToolCutsWithNoMachineAtAll)
{
	//a ball on the top of a block, leaning thirty degrees, with no chain: the groove is still one radius
	//deep, since the lowest point of a ball does not care which way the shank points
	k5_tool tool = CreateTool();
	const double dRadius = 0.016;
	k5_material material = NULL;
	ASSERT_EQ(K5_OK,k5_material_create(0.12,0.06,0.03,0.002,-0.06,-0.03,-0.03,&material))<<k5_last_error();
	const double dLean = 30.0*atan(1.0)/45.0;
	const double p[6] = {-0.04,0.0,0.0, 0.04,0.0,0.0};
	const double v[6] = {sin(dLean),0.0,cos(dLean), sin(dLean),0.0,cos(dLean)};
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	oSettings.max_position_difference = 0.002;		//as dense as the voxels
	k5_simulation simulation = NULL;
	ASSERT_EQ(K5_OK,k5_simulation_create_from_locations(tool,&oSettings,p,v,2,material,&simulation))
		<<k5_last_error();
	size_t nIndex=0, nCount=0;
	int nFinished=1;
	ASSERT_EQ(K5_OK,k5_simulation_progress(simulation,&nIndex,&nCount,&nFinished));
	EXPECT_EQ((size_t)41,nCount)<<"80 mm in 2 mm steps, and the first location";
	EXPECT_EQ(0,nFinished);

	//the dense path can be read back: the first location as it was, the second one step on, invented
	double q[3]={0,0,0}, a[3]={0,0,0};
	size_t nSource=9;
	int nInvented=9;
	ASSERT_EQ(K5_OK,k5_simulation_location(simulation,1,q,a,&nSource,&nInvented))<<k5_last_error();
	EXPECT_NEAR(-0.038,q[0],1E-9);
	EXPECT_NEAR(sin(dLean),a[0],1E-9);
	EXPECT_EQ((size_t)0,nSource);
	EXPECT_EQ(1,nInvented);
	ASSERT_EQ(K5_OK,k5_simulation_location(simulation,40,q,a,&nSource,&nInvented));
	EXPECT_NEAR(0.04,q[0],1E-9);
	EXPECT_EQ((size_t)1,nSource);
	EXPECT_EQ(0,nInvented);
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_simulation_location(simulation,41,q,a,NULL,NULL));

	ASSERT_EQ(K5_OK,k5_simulation_run(simulation));
	ASSERT_EQ(K5_OK,k5_simulation_progress(simulation,&nIndex,&nCount,&nFinished));
	EXPECT_EQ(1,nFinished);
	size_t nSurface = 0;
	ASSERT_EQ(K5_OK,k5_material_surface(material,NULL,0,&nSurface));
	std::vector<double> s(3*nSurface,0.0);
	ASSERT_EQ(K5_OK,k5_material_surface(material,&s[0],s.size(),&nSurface));
	double dUnderThePath = -1.0, dBesideIt = -1.0;
	for(size_t i=0;i<nSurface;i++){
		if(fabs(s[3*i])>0.03)
			continue;
		if(fabs(s[3*i+1])<0.001)
			dUnderThePath = std::max(dUnderThePath,s[3*i+2]);
		if(fabs(s[3*i+1]-0.025)<0.0015)
			dBesideIt = std::max(dBesideIt,s[3*i+2]);
	}
	EXPECT_LT(dUnderThePath,-0.5*dRadius)<<"a groove under the path";
	EXPECT_GT(dUnderThePath,-dRadius-0.004)<<"one radius deep, lean or no lean";
	EXPECT_GT(dBesideIt,-0.004)<<"nothing taken off beside it";
	size_t nGouges = 9;
	ASSERT_EQ(K5_OK,k5_simulation_gouges(simulation,&nGouges,NULL,NULL));
	EXPECT_EQ((size_t)0,nGouges)<<"the shank leans away above the block";
	k5_simulation_free(simulation);

	//an axis of no length is refused with the number of its location
	const double zero[6] = {0,0,1, 0,0,0};
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_simulation_create_from_locations(tool,&oSettings,p,zero,2,material,&simulation));
	EXPECT_TRUE(strstr(k5_last_error(),"2")!=NULL);
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_simulation_create_from_locations(NULL,&oSettings,p,v,2,material,&simulation));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_simulation_create_from_locations(tool,NULL,p,v,2,material,&simulation));
	//and an empty path is a simulation of nothing, which is allowed
	ASSERT_EQ(K5_OK,k5_simulation_create_from_locations(tool,&oSettings,NULL,NULL,0,material,&simulation))<<k5_last_error();
	ASSERT_EQ(K5_OK,k5_simulation_progress(simulation,&nIndex,&nCount,&nFinished));
	EXPECT_EQ((size_t)0,nCount);
	EXPECT_EQ(1,nFinished);
	k5_simulation_free(simulation);
	k5_material_free(material);
	k5_tool_free(tool);
}

TEST(Abi, TheLocationsOfASimulationOnAChainAreItsToolPoints)
{
	k5_chain chain = OpenChain();
	k5_tool tool = CreateTool();
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	oSettings.check_collisions = 0;
	const std::vector<double> q = QInsideLimits(chain);
	double p[3]={0,0,0}, v[3]={0,0,0};
	ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,tool,&q[0],p,v));
	k5_path path = NULL;
	ASSERT_EQ(K5_OK,k5_postprocess_locations(chain,tool,&oSettings,p,v,1,&path))<<k5_last_error();
	k5_material material = NULL;
	ASSERT_EQ(K5_OK,k5_material_create(0.1,0.1,0.05,0.005,-0.05,-0.05,-0.2,&material));
	k5_simulation simulation = NULL;
	ASSERT_EQ(K5_OK,k5_simulation_create(chain,tool,path,material,&simulation))<<k5_last_error();
	double q2[3]={0,0,0}, a[3]={0,0,0};
	size_t nSource=9;
	int nInvented=9;
	ASSERT_EQ(K5_OK,k5_simulation_location(simulation,0,q2,a,&nSource,&nInvented))<<k5_last_error();
	for(int k=0;k<3;k++){
		EXPECT_NEAR(p[k],q2[k],1E-9);
		EXPECT_NEAR(v[k],a[k],1E-9);
	}
	EXPECT_EQ((size_t)0,nSource);
	EXPECT_EQ(0,nInvented);
	k5_simulation_free(simulation);
	k5_material_free(material);
	k5_path_free(path);
	k5_tool_free(tool);
	k5_chain_free(chain);
}

TEST(Abi, AChainSaysWhichOfItsSolidsMetAtThePoseItStandsIn)
{
	k5_chain chain = OpenChain("3CWithSphericalWrist_withBaseBox.kc");
	k5_tool tool = CreateTool();
	std::vector<double> q((size_t)K5_AXES,0.0);
	ASSERT_EQ(K5_OK,k5_chain_set_configuration(chain,&q[0]));
	int nCollides = 1;
	ASSERT_EQ(K5_OK,k5_chain_check_collisions(chain,tool,&nCollides))<<k5_last_error();
	EXPECT_EQ(0,nCollides)<<"standing up, nothing meets";
	size_t nPairs = 1;
	ASSERT_EQ(K5_OK,k5_chain_collision_count(chain,&nPairs));
	EXPECT_EQ((size_t)0,nPairs);

	//every rotary joint turned a long way: this wrist meets itself
	for(size_t i=0;i<(size_t)K5_AXES;i++){
		k5_link oLink;
		ASSERT_EQ(K5_OK,k5_chain_link(chain,i,&oLink));
		if(oLink.type==K5_LINK_REVOLUTE)
			q[i] = 2.0;
	}
	ASSERT_EQ(K5_OK,k5_chain_set_configuration(chain,&q[0]));
	ASSERT_EQ(K5_OK,k5_chain_check_collisions(chain,tool,&nCollides))<<k5_last_error();
	ASSERT_EQ(1,nCollides)<<"turned that far, something meets";
	ASSERT_EQ(K5_OK,k5_chain_collision_count(chain,&nPairs));
	ASSERT_GT(nPairs,(size_t)0);
	for(size_t i=0;i<nPairs;i++){
		k5_collision oPair;
		ASSERT_EQ(K5_OK,k5_chain_collision(chain,i,&oPair))<<k5_last_error();
		EXPECT_STRNE("",oPair.first);
		EXPECT_STRNE("",oPair.second);
		EXPECT_EQ((size_t)1,oPair.locations)<<"one check, one count";
	}
	k5_collision oPair;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_collision(chain,nPairs,&oPair));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_check_collisions(chain,NULL,&nCollides));

	//and the names of the solids are the ones the pairs are made of
	char szName[K5_NAME_MAX] = {0};
	size_t nSegments = 0;
	ASSERT_EQ(K5_OK,k5_chain_frames(chain,K5_PART_TOOL,NULL,0,&nSegments));
	bool bFound = false;
	for(size_t i=0;i<nSegments && !bFound;i++){
		size_t nBoxes = 0;
		ASSERT_EQ(K5_OK,k5_chain_segment_boxes(chain,K5_PART_TOOL,i,NULL,0,&nBoxes));
		for(size_t b=0;b<nBoxes;b++){
			ASSERT_EQ(K5_OK,k5_chain_solid_name(chain,K5_PART_TOOL,i,b,szName,sizeof(szName)))
				<<k5_last_error();
			EXPECT_STRNE("",szName);
			for(size_t p=0;p<nPairs;p++){
				k5_collision o;
				ASSERT_EQ(K5_OK,k5_chain_collision(chain,p,&o));
				if(!strcmp(o.first,szName) || !strcmp(o.second,szName))
					bFound = true;
			}
		}
	}
	EXPECT_TRUE(bFound)<<"a solid of the tool branch is named in one of the pairs";
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_solid_name(chain,K5_PART_TOOL,nSegments,0,szName,sizeof(szName)))
		<<"a segment that is not there is a bad argument, as it is everywhere else";
	k5_tool_free(tool);
	k5_chain_free(chain);
}

TEST(Abi, APathSaysWhichSolidsMetAndAtHowManyLocations)
{
	//the model of the spherical wrist cannot tilt its tool far without meeting itself: a path that
	//asks it to is where a pair and a count are worth more than "it collides"
	k5_chain chain = OpenChain("3CWithSphericalWrist_withBaseBox.kc");
	k5_tool tool = CreateTool();
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	oSettings.interpolate = 0;

	//a line of locations whose tool axis leans further and further over
	std::vector<double> vPositions, vAxes;
	const size_t nCount = 12;
	for(size_t i=0;i<nCount;i++){
		const double dLean = (double)i*(4.0*atan(1.0)/180.0)*8.0;	//0, 8, 16 ... degrees
		vPositions.push_back(0.02*(double)i-0.1);
		vPositions.push_back(0.0);
		vPositions.push_back(0.1);
		vAxes.push_back(sin(dLean));
		vAxes.push_back(0.0);
		vAxes.push_back(cos(dLean));
	}
	k5_path path = NULL;
	ASSERT_EQ(K5_OK,k5_postprocess_locations(chain,tool,&oSettings,&vPositions[0],&vAxes[0],nCount,&path))
		<<k5_last_error();
	size_t nColliding = 0;
	ASSERT_EQ(K5_OK,k5_path_count_of_status(path,K5_COLLISION,&nColliding));
	ASSERT_GT(nColliding,(size_t)0)<<"the far end of that path has to collide for this to say anything";

	size_t nPairs = 0;
	ASSERT_EQ(K5_OK,k5_path_collision_count(path,&nPairs))<<k5_last_error();
	ASSERT_GT(nPairs,(size_t)0)<<"and the path says which solids met";
	size_t nWorst = 0;
	for(size_t i=0;i<nPairs;i++){
		k5_collision oPair;
		ASSERT_EQ(K5_OK,k5_path_collision(path,i,&oPair))<<k5_last_error();
		EXPECT_STRNE("",oPair.first);
		EXPECT_STRNE("",oPair.second);
		EXPECT_STRNE(oPair.first,oPair.second);
		EXPECT_GT(oPair.locations,(size_t)0);
		EXPECT_LE(oPair.locations,nColliding)<<"no pair can cost more locations than collided";
		EXPECT_TRUE(oPair.kind==K5_COLLIDING_PARTS || oPair.kind==K5_COLLIDING_TOOL
			|| oPair.kind==K5_COLLIDING_TOOL_SHANK);
		nWorst = std::max(nWorst,oPair.locations);
	}
	EXPECT_EQ(nColliding,nWorst)<<"the pair that rejected every colliding location costs all of them";

	k5_collision oPair;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_collision(path,nPairs,&oPair));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_collision(NULL,0,&oPair));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_path_collision_count(NULL,&nPairs));
	k5_path_free(path);

	//with the check off nothing is rejected and nothing is reported
	oSettings.check_collisions = 0;
	ASSERT_EQ(K5_OK,k5_postprocess_locations(chain,tool,&oSettings,&vPositions[0],&vAxes[0],nCount,&path));
	ASSERT_EQ(K5_OK,k5_path_collision_count(path,&nPairs));
	EXPECT_EQ((size_t)0,nPairs);
	k5_path_free(path);
	k5_tool_free(tool);
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
	ASSERT_EQ(K5_OK,k5_tool_create(K5_TOOL_FLAT_END,0.008,0.0,0.040,0.0,0.050,0.030,0.0,&oFlat));
	ASSERT_EQ(K5_OK,k5_tool_assembly_length(oFlat,&dAssembly));
	ASSERT_EQ(K5_OK,k5_tool_mount_to_point_distance(oFlat,&dMount));
	EXPECT_NEAR(dAssembly,dMount,1E-12);
	k5_tool_free(oFlat);

	//dimensions that do not make a tool come back as an argument error that says what is wrong
	k5_tool oBad = (k5_tool)0x1;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_create(K5_TOOL_BALL_NOSE,-0.016,0.016,0.064,0.0,0.0,0.0,0.0,&oBad));
	EXPECT_NE(std::string::npos,std::string(k5_last_error()).find("negative"))<<k5_last_error();
	EXPECT_TRUE(oBad==NULL);
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_create(K5_TOOL_BALL_NOSE,0.016,0.008,0.064,0.0,0.0,0.0,0.0,&oBad))
		<<"a ball narrower than the shank";
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_create(42,0.016,0.016,0.064,0.0,0.0,0.0,0.0,&oBad))<<"no such shape";

	//The gauge length, which is what ties a tool to a machine: the end of a tool branch is where a
	//holder is clamped, the same place for every tool, so how far the tip reaches below it belongs
	//to what is in the spindle. Two tools of identical dimensions, one pulled further out of its
	//holder, put their tips 0.1 apart on the same machine.
	k5_tool oShort = NULL, oLong = NULL;
	ASSERT_EQ(K5_OK,k5_tool_create(K5_TOOL_BALL_NOSE,0.016,0.016,0.064,0.030,0.050,0.030,0.150,&oShort));
	ASSERT_EQ(K5_OK,k5_tool_create(K5_TOOL_BALL_NOSE,0.016,0.016,0.064,0.030,0.050,0.030,0.250,&oLong));
	double dGauge = 0.0, dToPoint = 0.0;
	ASSERT_EQ(K5_OK,k5_tool_gauge_length(oShort,&dGauge));
	EXPECT_DOUBLE_EQ(0.150,dGauge);
	ASSERT_EQ(K5_OK,k5_tool_mount_to_point_distance(oShort,&dToPoint));
	EXPECT_DOUBLE_EQ(0.150-0.016,dToPoint)<<"the ball centre, not the tip";
	ASSERT_EQ(K5_OK,k5_tool_gauge_length(oLong,&dGauge));
	EXPECT_DOUBLE_EQ(0.250,dGauge);

	//0 is the tool standing right up against its holder, which is what every caller meant before
	//this length existed
	k5_tool oDerived = NULL;
	ASSERT_EQ(K5_OK,k5_tool_create(K5_TOOL_BALL_NOSE,0.016,0.016,0.064,0.030,0.050,0.030,0.0,&oDerived));
	ASSERT_EQ(K5_OK,k5_tool_gauge_length(oDerived,&dGauge));
	EXPECT_DOUBLE_EQ(0.064+0.030+0.050,dGauge);
	dAssembly = 0.0;
	ASSERT_EQ(K5_OK,k5_tool_assembly_length(oDerived,&dAssembly));
	EXPECT_DOUBLE_EQ(dGauge,dAssembly)<<"the older name of the same measurement";

	//and on a machine the two tools reach exactly their difference apart
	k5_chain chain = OpenChain();
	const std::vector<double> q = QInsideLimits(chain);
	double pShort[3] = {0,0,0}, pLong[3] = {0,0,0}, v[3] = {0,0,0};
	ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,oShort,&q[0],pShort,v));
	ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,oLong,&q[0],pLong,v));
	double dApart = 0.0;
	for(int i=0;i<3;i++)
		dApart += (pShort[i]-pLong[i])*(pShort[i]-pLong[i]);
	EXPECT_NEAR(0.100,sqrt(dApart),1E-12)<<"the longer tool reaches its extra length further";

	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_gauge_length(NULL,&dGauge));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_gauge_length(oShort,NULL));
	k5_chain_free(chain);
	k5_tool_free(oShort);
	k5_tool_free(oLong);
	k5_tool_free(oDerived);

	k5_tool_free(NULL);
}

// ---------------------------------------------------------------- the workpiece holder

namespace {

//the frame of a vice 0.08 tall whose jaws are turned a quarter turn about OZ, in the layout of
//k5_chain_frames: the rows are the images of the axes, the last row the origin
void ViceFrame(double *m)
{
	for(int i=0;i<16;i++)
		m[i] = 0.0;
	m[0] = 0.0;  m[1] = 1.0;			//OX of the part points along the OY of the table
	m[4] = -1.0; m[5] = 0.0;
	m[10] = 1.0;
	m[14] = 0.08;						//and the jaws hold it 0.08 above the table
	m[15] = 1.0;
}

} // namespace

TEST(Abi, TheWorkpieceHolderIsTheIdentityUntilOneIsGiven)
{
	k5_chain chain = OpenChain();
	ASSERT_TRUE(chain!=NULL);
	double m[16];
	for(int i=0;i<16;i++)
		m[i] = 42.0;
	ASSERT_EQ(K5_OK,k5_chain_workpiece_holder(chain,m));
	for(int r=0;r<4;r++)
		for(int c=0;c<4;c++)
			EXPECT_DOUBLE_EQ((r==c) ? 1.0 : 0.0,m[4*r+c])<<"row "<<r<<" column "<<c;
	//null is the identity: a host with nothing to say about the vice says nothing
	EXPECT_EQ(K5_OK,k5_chain_set_workpiece_holder(chain,NULL));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_set_workpiece_holder(NULL,m));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_workpiece_holder(chain,NULL));
	k5_chain_free(chain);
}

TEST(Abi, TheWorkpieceHolderComesBackAsItWasGiven)
{
	k5_chain chain = OpenChain();
	ASSERT_TRUE(chain!=NULL);
	double m[16], back[16];
	ViceFrame(m);
	ASSERT_EQ(K5_OK,k5_chain_set_workpiece_holder(chain,m))<<k5_last_error();
	ASSERT_EQ(K5_OK,k5_chain_workpiece_holder(chain,back));
	for(int i=0;i<16;i++)
		EXPECT_DOUBLE_EQ(m[i],back[i])<<"element "<<i;
	k5_chain_free(chain);
}

TEST(Abi, AFrameThatIsNotRigidIsNoWayOfHoldingAnything)
{
	k5_chain chain = OpenChain();
	ASSERT_TRUE(chain!=NULL);
	double m[16];
	ViceFrame(m);
	m[0] = 2.0;							//a scale: twice as big in one direction
	m[1] = 0.0;
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_set_workpiece_holder(chain,m));
	EXPECT_NE(std::string::npos,std::string(k5_last_error()).find("rigid"))<<k5_last_error();
	//and the chain still stands where it did
	double back[16];
	ASSERT_EQ(K5_OK,k5_chain_workpiece_holder(chain,back));
	EXPECT_DOUBLE_EQ(1.0,back[0]);
	k5_chain_free(chain);
}

TEST(Abi, ThePartOnAViceIsSolvedInItsOwnFrame)
{
	//The whole point of the holder: a cutter location is given in the frame of the part, and where
	//the part is held is the machine's business. The forward kinematics answers in the same frame,
	//so a target solved with a vice on the table comes back to the same point of the part - with
	//the machine standing somewhere else than it would have without one.
	k5_chain chain = OpenChain();
	ASSERT_TRUE(chain!=NULL);
	k5_tool tool = CreateTool();
	ASSERT_TRUE(tool!=NULL);
	const std::vector<double> q = QInsideLimits(chain);
	double target[3] = {0,0,0}, axis[3] = {0,0,0};
	ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,tool,&q[0],target,axis));

	double m[16];
	ViceFrame(m);
	ASSERT_EQ(K5_OK,k5_chain_set_workpiece_holder(chain,m));
	k5_settings oSettings;
	k5_settings_default(&oSettings);
	oSettings.check_collisions = 0;		//the holder meets the head boxes of the models
	std::vector<double> qOut((size_t)K5_AXES*8,0.0);
	size_t nCount = 0;
	int nStatus = 0;
	ASSERT_EQ(K5_OK,k5_solve_location(chain,tool,&oSettings,target,axis,&qOut[0],8,&nCount,&nStatus))
		<<k5_last_error();
	ASSERT_GT(nCount,(size_t)0)<<"the location is reachable with the part on a vice too";
	for(size_t s=0;s<nCount;s++){
		double p[3] = {0,0,0}, v[3] = {0,0,0};
		ASSERT_EQ(K5_OK,k5_chain_tool_point(chain,tool,&qOut[s*K5_AXES],p,v));
		for(int i=0;i<3;i++){
			EXPECT_NEAR(target[i],p[i],1E-9)<<"solution "<<s<<" axis "<<i;
			EXPECT_NEAR(axis[i],v[i],1E-9)<<"solution "<<s<<" axis "<<i;
		}
		//and the machine is not where it was: the part came up and turned, so the joints did
		double dApart = 0.0;
		for(size_t i=0;i<(size_t)K5_AXES;i++)
			dApart += fabs(qOut[s*K5_AXES+i]-q[i]);
		EXPECT_GT(dApart,1E-6)<<"solution "<<s<<" stands where the bare table stood";
	}
	k5_tool_free(tool);
	k5_chain_free(chain);
}

// ---------------------------------------------------------------- tool holders

TEST(Abi, TheHolderAChainSaysItsMachineCarries)
{
	//AC_TiltRotaryTable declares one cylinder on its tool terminal and names no model
	k5_chain chain = OpenChain();
	ASSERT_TRUE(chain!=NULL);
	char szModel[K5_NAME_MAX] = "not asked";
	ASSERT_EQ(K5_OK,k5_chain_default_holder_model(chain,szModel,sizeof(szModel)));
	EXPECT_EQ(std::string(),std::string(szModel))<<"the file names no holder";
	size_t nSections = 99;
	ASSERT_EQ(K5_OK,k5_chain_default_holder(chain,NULL,0,&nSections));
	ASSERT_EQ((size_t)1,nSections)<<"one cylinder, as the file gives it";
	double v[2*K5_HOLDER_SECTIONS] = {0};
	ASSERT_EQ(K5_OK,k5_chain_default_holder(chain,v,sizeof(v)/sizeof(v[0]),&nSections));
	EXPECT_GT(v[0],0.0)<<"a length";
	EXPECT_GT(v[1],0.0)<<"and a radius";
	//a buffer that cannot hold the cylinders says so rather than writing past it
	EXPECT_EQ(K5_ERROR_BUFFER_TOO_SMALL,k5_chain_default_holder(chain,v,1,&nSections));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_default_holder(NULL,v,2,&nSections));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_default_holder(chain,v,2,NULL));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_default_holder_model(chain,NULL,sizeof(szModel)));
	k5_chain_free(chain);
}

TEST(Abi, AChainWithoutAHolderSaysSoWithoutFailing)
{
	//A machine file need not name a holder, and then there is nothing for a host to offer: no name,
	//no cylinder, and no error either. Every chain shipped with the library names one, so this test
	//makes a file that does not out of one that does.
	std::string sText;
	{
		FILE *pFile = fopen(ChainPath().c_str(),"rb");
		ASSERT_TRUE(pFile!=NULL);
		char szBuffer[4096];
		size_t n = 0;
		while((n=fread(szBuffer,1,sizeof(szBuffer),pFile))>0)
			sText.append(szBuffer,n);
		fclose(pFile);
	}
	const size_t nAt = sText.rfind("<tool_holder");
	ASSERT_NE(std::string::npos,nAt);
	const size_t nClose = sText.find("</tool_holder>",nAt);
	const size_t nEnd = (nClose!=std::string::npos) ? nClose+sizeof("</tool_holder>")-1
		: sText.find("/>",nAt)+2;
	sText.erase(nAt,nEnd-nAt);
	const std::string sPath = TempPath("kin5axcTests_no_holder.kc");
	{
		FILE *pFile = fopen(sPath.c_str(),"wb");
		ASSERT_TRUE(pFile!=NULL);
		fwrite(sText.c_str(),1,sText.size(),pFile);
		fclose(pFile);
	}
	k5_chain chain = NULL;
	ASSERT_EQ(K5_OK,k5_chain_open(sPath.c_str(),&chain))<<k5_last_error();
	remove(sPath.c_str());
	char szModel[K5_NAME_MAX] = "not asked";
	ASSERT_EQ(K5_OK,k5_chain_default_holder_model(chain,szModel,sizeof(szModel)));
	EXPECT_EQ(std::string(),std::string(szModel));
	size_t nSections = 99;
	ASSERT_EQ(K5_OK,k5_chain_default_holder(chain,NULL,0,&nSections));
	EXPECT_EQ((size_t)0,nSections);
	k5_chain_free(chain);
}

TEST(Abi, TheCatalogueOfTheHoldersAMachineUsuallyCarries)
{
	size_t nCount = 0;
	ASSERT_EQ(K5_OK,k5_tool_holder_catalogue_size(&nCount));
	ASSERT_GE(nCount,(size_t)5)<<"a list worth offering";
	for(size_t i=0;i<nCount;i++){
		char szModel[K5_NAME_MAX] = "";
		ASSERT_EQ(K5_OK,k5_tool_holder_catalogue_model(i,szModel,sizeof(szModel)))<<k5_last_error();
		SCOPED_TRACE(szModel);
		EXPECT_FALSE(std::string(szModel).empty())<<"every holder is named";
		size_t nSections = 0;
		double v[2*K5_HOLDER_SECTIONS] = {0};
		ASSERT_EQ(K5_OK,k5_tool_holder_catalogue(i,v,sizeof(v)/sizeof(v[0]),&nSections));
		ASSERT_GE(nSections,(size_t)1);
		ASSERT_LE(nSections,(size_t)K5_HOLDER_SECTIONS)<<"the header says how many there can be";
		double dLength = 0.0;
		for(size_t j=0;j<nSections;j++){
			EXPECT_GT(v[2*j],0.0);
			EXPECT_GT(v[2*j+1],0.0);
			dLength += v[2*j];
		}
		//metres, whatever unit a chain is in: a holder is a hand's length
		EXPECT_GT(dLength,0.02);
		EXPECT_LT(dLength,0.30);
	}
	//past the end is an error, not an empty holder: the index came from somewhere wrong
	char szModel[K5_NAME_MAX] = "";
	size_t nSections = 0;
	double v[2*K5_HOLDER_SECTIONS] = {0};
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_holder_catalogue_model(nCount,szModel,sizeof(szModel)));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_holder_catalogue(nCount,v,2*K5_HOLDER_SECTIONS,&nSections));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_holder_catalogue_size(NULL));
}

TEST(Abi, AToolTakesAHolderOfSeveralCylinders)
{
	//a tool measured on a presetter: its tip is 0.2 below the mount, whatever holder it sits in
	k5_tool tool = NULL;
	ASSERT_EQ(K5_OK,k5_tool_create(K5_TOOL_BALL_NOSE,0.008,0.008,0.040,0.030,0.050,0.030,0.200,&tool));
	const double v[6] = {0.065,0.0445, 0.025,0.027, 0.035,0.025};
	ASSERT_EQ(K5_OK,k5_tool_set_holder(tool,v,3))<<k5_last_error();
	double dGauge = 0.0;
	ASSERT_EQ(K5_OK,k5_tool_gauge_length(tool,&dGauge));
	EXPECT_DOUBLE_EQ(0.200,dGauge)<<"a measured tool does not move when its holder changes";

	//a tool that was never measured stands against its holder, so a longer holder lifts the tip
	k5_tool oDerived = NULL;
	ASSERT_EQ(K5_OK,k5_tool_create(K5_TOOL_BALL_NOSE,0.008,0.008,0.040,0.030,0.050,0.030,0.0,&oDerived));
	ASSERT_EQ(K5_OK,k5_tool_gauge_length(oDerived,&dGauge));
	ASSERT_DOUBLE_EQ(0.040+0.030+0.050,dGauge);
	ASSERT_EQ(K5_OK,k5_tool_set_holder(oDerived,v,3));
	ASSERT_EQ(K5_OK,k5_tool_gauge_length(oDerived,&dGauge));
	EXPECT_DOUBLE_EQ(0.040+0.030+0.125,dGauge);

	//no cylinder at all is a holder without a body
	ASSERT_EQ(K5_OK,k5_tool_set_holder(oDerived,NULL,0));
	ASSERT_EQ(K5_OK,k5_tool_gauge_length(oDerived,&dGauge));
	EXPECT_DOUBLE_EQ(0.040+0.030,dGauge);

	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_set_holder(tool,v,K5_HOLDER_SECTIONS+1));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_set_holder(tool,NULL,1));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_tool_set_holder(NULL,v,3));
	k5_tool_free(tool);
	k5_tool_free(oDerived);
}

TEST(Abi, AHolderOfTheCatalogueGoesStraightOntoATool)
{
	//what a host does with the list: pick a holder, put it on the tool, and the lengths agree
	size_t nSections = 0;
	double v[2*K5_HOLDER_SECTIONS] = {0};
	ASSERT_EQ(K5_OK,k5_tool_holder_catalogue(0,v,sizeof(v)/sizeof(v[0]),&nSections));
	k5_tool tool = NULL;
	ASSERT_EQ(K5_OK,k5_tool_create(K5_TOOL_FLAT_END,0.006,0.0,0.030,0.020,0.0,0.0,0.0,&tool));
	ASSERT_EQ(K5_OK,k5_tool_set_holder(tool,v,nSections));
	double dHolder = 0.0;
	for(size_t j=0;j<nSections;j++)
		dHolder += v[2*j];
	double dGauge = 0.0;
	ASSERT_EQ(K5_OK,k5_tool_gauge_length(tool,&dGauge));
	EXPECT_NEAR(0.030+0.020+dHolder,dGauge,1E-12);
	k5_tool_free(tool);
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

	//the records: the line of the file, the cutter location they belong to, the solution, and what
	//they were solved for - the file's own points a third of a step apart, with the axis of the file
	size_t nFileSeen = 0;
	for(size_t i=0;i<nSize;i++){
		k5_location oLocation;
		ASSERT_EQ(K5_OK,k5_path_location(path,i,&oLocation))<<"record "<<i;
		EXPECT_EQ(1,oLocation.has_solution)<<"record "<<i;
		EXPECT_GT(oLocation.file_line,(size_t)0)<<"record "<<i<<" knows its line of the file";
		if(oLocation.origin==K5_FROM_FILE)
			nFileSeen++;
		EXPECT_EQ(nFileSeen,oLocation.file_location)<<"record "<<i;
		EXPECT_NEAR(p[0]+(double)i*0.025/3.0,oLocation.position[0],1E-6)<<"record "<<i<<" is a third of a step on";
		EXPECT_NEAR(p[1],oLocation.position[1],1E-6)<<"record "<<i;
		EXPECT_NEAR(p[2],oLocation.position[2],1E-6)<<"record "<<i;
		for(size_t k=0;k<3;k++)
			EXPECT_NEAR(v[k],oLocation.axis[k],1E-5)<<"record "<<i<<" axis "<<k;
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
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_chain_unit(NULL,&bFlag));
	EXPECT_EQ(K5_ERROR_ARGUMENT,k5_unit_in_millimetres(K5_UNIT_M,NULL));
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
