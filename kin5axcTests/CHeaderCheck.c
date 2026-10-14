/* The public header, compiled by the C compiler: if anything in kin5ax.h were C++ only, this file
 * would not build. It also calls the simplest functions through the C compiler, so that the linkage of
 * the shared library is exercised from C and not only from C++. */

#include "kin5ax.h"

/* 0 when everything the C compiler can check here is in order, else the number of the check that failed */
int k5c_header_check_c(void)
{
	int major = -1, minor = -1, patch = -1;
	k5_settings oSettings;
	k5_link oLink;
	k5_location oLocation;
	k5_gcode_axes oAxes;
	k5_chain chain = NULL;
	k5_tool tool = NULL;
	k5_path path = NULL;

	if(k5_version_string()==NULL || k5_version_string()[0]==0)
		return 1;
	/* the library answers with its own version, which is what a host checks; the macros of the header
	 * it was compiled against have to say the same thing, or checking them would prove nothing */
	k5_version(&major,&minor,&patch);
	if(major!=K5_VERSION_MAJOR || minor!=K5_VERSION_MINOR || patch!=K5_VERSION_PATCH)
		return 2;
	if(k5_last_error()==NULL)
		return 3;
	/* the build metadata, reachable from C as well */
	if(k5_commit_hash()==NULL || k5_commit_hash()[0]==0)
		return 10;
	if(k5_build_string()==NULL || k5_build_string()[0]==0)
		return 11;

	/* the recommended settings, and the fields a C host writes into */
	k5_settings_default(&oSettings);
	if(oSettings.check_limits!=1 || oSettings.start_criterion!=K5_START_MIN_ABS)
		return 4;
	oSettings.interpolate = 0;
	oSettings.start_random_seed = 7u;

	/* the structures are complete types a C host can put on its stack, write into and read back */
	oLink.type = K5_LINK_REVOLUTE;
	oLink.name[0] = 0;
	if(oLink.type!=K5_LINK_REVOLUTE || oLink.name[0]!=0)
		return 7;
	oLocation.status = K5_SOLVED;
	oLocation.origin = K5_FROM_FILE;
	oLocation.q[K5_AXES-1] = 0.5;
	if(oLocation.status!=K5_SOLVED || oLocation.origin!=K5_FROM_FILE || oLocation.q[K5_AXES-1]!=0.5)
		return 8;
	oAxes.degrees = 1;
	oAxes.angle1_letter = 'A';
	if(oAxes.degrees!=1 || oAxes.angle1_letter!='A')
		return 9;

	/* freeing a handle that was never created is allowed, the way free(NULL) is */
	k5_chain_free(chain);
	k5_tool_free(tool);
	k5_path_free(path);

	/* and a null argument is an error with a message, not a crash */
	if(k5_chain_open(NULL,&chain)!=K5_ERROR_ARGUMENT)
		return 5;
	if(k5_last_error()[0]==0)
		return 6;
	return 0;
}
