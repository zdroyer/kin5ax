// The C interface of kin5ax: handles instead of objects, a status instead of an exception, and plain
// doubles instead of the geometry classes. Every function of kin5ax.h is here and nowhere else.
#include "handles.h"		//brings the prelude of the kin5ax library, which its headers expect first
#include "kin5ax.h"

#include "MillingToolBall.h"
#include "MillingToolCylinder.h"
#include "5CPostprocessor.h"
#include "ArgsMyException.h"
#include "double_tools.h"
#include "string_tools.h"

#include <string>
#include <cstring>
#include <cstdio>
#include <vector>

using namespace kin5ax;

namespace {

//The message of the last call that failed, per thread: a host that uses two machines on two threads
//gets its own answer on each of them.
thread_local std::string g_sLastError;

k5_status Fail(k5_status nStatus, const std::string &sMessage)
{
	g_sLastError = sMessage;
	return nStatus;
}

//every call starts by forgetting the message of the one before it
void Clear()
{
	g_sLastError.clear();
}

//the boundary: nothing of C++ escapes through the header, not even an exception
#define K5_BEGIN	Clear(); try {
#define K5_END(nDefault)	} \
	catch(const CArgsMyException &e){ return Fail(K5_ERROR_ARGUMENT,e.Message()); } \
	catch(const CGeneratorException &e){ return Fail(nDefault,e.Message()); } \
	catch(const std::exception &e){ return Fail(K5_ERROR_UNKNOWN,e.what()); } \
	catch(...){ return Fail(K5_ERROR_UNKNOWN,"an exception of a kind this interface does not know"); }

#define K5_NEED(bCondition,szMessage)	if(!(bCondition)) return Fail(K5_ERROR_ARGUMENT,szMessage);

//the values of the enums are the interface, so they are mapped and never cast
int ToCStatus(SOLVE_STATUS nStatus)
{
	switch(nStatus){
		case SOLVE_STATUS_SOLVED:			return K5_SOLVED;
		case SOLVE_STATUS_NO_ORIENTATION:	return K5_NO_ORIENTATION;
		case SOLVE_STATUS_NO_POSITION:		return K5_NO_POSITION;
		case SOLVE_STATUS_LIMITS_EXCEEDED:	return K5_LIMITS_EXCEEDED;
		case SOLVE_STATUS_COLLISION:		return K5_COLLISION;
		default:							return K5_SINGULAR_UNSOLVED;
	}
}

bool FromCStatus(int nStatus, SOLVE_STATUS &nOut)
{
	switch(nStatus){
		case K5_SOLVED:				nOut = SOLVE_STATUS_SOLVED; return true;
		case K5_NO_ORIENTATION:		nOut = SOLVE_STATUS_NO_ORIENTATION; return true;
		case K5_NO_POSITION:		nOut = SOLVE_STATUS_NO_POSITION; return true;
		case K5_LIMITS_EXCEEDED:	nOut = SOLVE_STATUS_LIMITS_EXCEEDED; return true;
		case K5_COLLISION:			nOut = SOLVE_STATUS_COLLISION; return true;
		case K5_SINGULAR_UNSOLVED:	nOut = SOLVE_STATUS_SINGULAR_UNSOLVED; return true;
	}
	return false;
}

int ToCOrigin(LOCATION_ORIGIN nOrigin)
{
	switch(nOrigin){
		case LOCATION_FROM_FILE:	return K5_FROM_FILE;
		case LOCATION_INTERPOLATED:	return K5_INTERPOLATED;
		default:					return K5_SINGULAR_SECTION;
	}
}

bool FromCOrigin(int nOrigin, LOCATION_ORIGIN &nOut)
{
	switch(nOrigin){
		case K5_FROM_FILE:			nOut = LOCATION_FROM_FILE; return true;
		case K5_INTERPOLATED:		nOut = LOCATION_INTERPOLATED; return true;
		case K5_SINGULAR_SECTION:	nOut = LOCATION_SINGULAR_SECTION; return true;
	}
	return false;
}

bool FromCCriterion(int nCriterion, TRAJECTORY_START_CRITERION &nOut)
{
	switch(nCriterion){
		case K5_START_SHORTEST_PATH:	nOut = TRAJECTORY_START_SHORTEST_PATH; return true;
		case K5_START_MIN:				nOut = TRAJECTORY_START_MIN; return true;
		case K5_START_MAX:				nOut = TRAJECTORY_START_MAX; return true;
		case K5_START_MIN_ABS:			nOut = TRAJECTORY_START_MIN_ABS; return true;
		case K5_START_RANDOM:			nOut = TRAJECTORY_START_RANDOM; return true;
	}
	return false;
}

bool FromCPart(int nPart, CHAIN_PART &nOut)
{
	if(nPart==K5_PART_TOOL){
		nOut = CHAIN_PART_TOOL;
		return true;
	}
	if(nPart==K5_PART_MATERIAL_HOLDER){
		nOut = CHAIN_PART_MATERIAL_HOLDER;
		return true;
	}
	return false;
}

CPostprocessorSettings FromCSettings(const k5_settings &o)
{
	CPostprocessorSettings oSettings;
	oSettings.m_bCheckLimits = (o.check_limits!=0);
	oSettings.m_bCheckCollisions = (o.check_collisions!=0);
	oSettings.m_bInterpolateBetweenLocations = (o.interpolate!=0);
	oSettings.m_dMaxPositionDifference = o.max_position_difference;
	oSettings.m_dMaxOrientationDifferenceDegrees = o.max_orientation_difference_degrees;
	oSettings.m_dMaxOrientationDifferenceRadians = degrees_to_radians(o.max_orientation_difference_degrees);
	oSettings.m_dAngleDistanceCoefficientRadians = o.angle_distance_weight;
	oSettings.m_dDisplacementCoefficient = o.displacement_weight;
	return oSettings;
}

//the joint values of a configuration into q[K5_AXES], in the order the chain keeps them
void ToQ(const CChainConfiguration &oConfig, double *q)
{
	for(size_t i=0;i<(size_t)K5_AXES;i++)
		q[i] = (i<oConfig.GetParametersCount()) ? oConfig[i].GetValue() : 0.0;
}

//and back, with the unit of each joint taken from the chain
CChainConfiguration FromQ(const CChain &oChain, const double *q)
{
	const size_t n = oChain.GetNumberOfRegularLinks();
	CChainConfiguration oConfig((int)n);
	for(size_t i=0;i<n;i++){
		const bool bRevolute = oChain[(int)i].GetType()==CHAIN_SEGMENT_REVOLUTE_LINK;
		oConfig[i].SetParameter(q[i],bRevolute ? RADIAN : DISTANCE_UNIT);
	}
	return oConfig;
}

//A chain this interface can hand five joint values to and take five back. Everything that speaks in
//q[K5_AXES] needs it; the queries about the structure of a chain do not.
k5_status NeedFiveAxes(const CChain &oChain)
{
	if(!oChain.Is5CChain())
		return Fail(K5_ERROR_CHAIN,"This is not a five-axis chain: three linear and two rotary joints are needed");
	if(oChain.GetNumberOfRegularLinks()!=(size_t)K5_AXES)
		return Fail(K5_ERROR_CHAIN,"This chain does not have five joints");
	return K5_OK;
}

void CopyName(const std::string &sName, char *out, size_t nCapacity)
{
	if(!nCapacity)
		return;
	const size_t n = (sName.size()<nCapacity-1) ? sName.size() : nCapacity-1;
	memcpy(out,sName.c_str(),n);
	out[n] = 0;
}

//the start of a trajectory, as the settings of a run describe it
CTrajectoryStart StartOf(const CChain &oChain, const k5_settings &o)
{
	TRAJECTORY_START_CRITERION nCriterion = TRAJECTORY_START_SHORTEST_PATH;
	FromCCriterion(o.start_criterion,nCriterion);	//checked by the caller
	CTrajectoryStart oStart = oChain.GetTrajectoryStart(
		o.start_count_from_tool ? CHAIN_PART_TOOL : CHAIN_PART_MATERIAL_HOLDER,nCriterion);
	oStart.m_nRandomSeed = o.start_random_seed;
	return oStart;
}

//What the two batch entry points share, once their own arguments are in order: the checks of the
//chain, the tool and the settings, the run, and the trajectory through what came out of it.
k5_status RunPostprocessor(k5_chain_t *pChain, k5_tool_t *pTool, const k5_settings &oCSettings
	,CCutterLocationData &oLocations, k5_path *out)
{
	const k5_status nFive = NeedFiveAxes(pChain->m_oChain);
	if(nFive!=K5_OK)
		return nFive;
	TRAJECTORY_START_CRITERION nCriterion = TRAJECTORY_START_SHORTEST_PATH;
	if(!FromCCriterion(oCSettings.start_criterion,nCriterion))
		return Fail(K5_ERROR_ARGUMENT,"start_criterion is not one of k5_start_criterion");
	CMillingTool *pMillingTool = (pTool && pTool->m_pTool) ? pTool->m_pTool.get() : NULL;
	if(!pMillingTool && oCSettings.check_collisions)
		return Fail(K5_ERROR_ARGUMENT,"the collision check needs a tool");

	const CPostprocessorSettings oSettings = FromCSettings(oCSettings);
	std::unique_ptr<k5_path_t> pHandle(new k5_path_t());
	CCLDataPostprocessor oPostprocessor(oSettings);
	pHandle->m_oResult = oPostprocessor.Process(pChain->m_oChain,pMillingTool,oLocations
		,pHandle->m_oData,NULL,0,NULL);
	pHandle->m_oData.FindShortestPath(oSettings,NULL,StartOf(pChain->m_oChain,oCSettings));
	*out = pHandle.release();
	return K5_OK;
}

} // namespace

// ---------------------------------------------------------------- the library itself

extern "C" {

const char *k5_last_error(void)
{
	return g_sLastError.c_str();
}

const char *k5_version_string(void)
{
	return "1.0.0";
}

void k5_version(int *major, int *minor, int *patch)
{
	if(major) *major = 1;
	if(minor) *minor = 0;
	if(patch) *patch = 0;
}

void k5_settings_default(k5_settings *out)
{
	if(!out)
		return;
	const CPostprocessorSettings oDefault;
	out->check_limits = oDefault.m_bCheckLimits ? 1 : 0;
	out->check_collisions = oDefault.m_bCheckCollisions ? 1 : 0;
	out->interpolate = oDefault.m_bInterpolateBetweenLocations ? 1 : 0;
	out->max_position_difference = oDefault.m_dMaxPositionDifference;
	out->max_orientation_difference_degrees = oDefault.m_dMaxOrientationDifferenceDegrees;
	out->angle_distance_weight = oDefault.m_dAngleDistanceCoefficientRadians;
	out->displacement_weight = oDefault.m_dDisplacementCoefficient;
	out->start_criterion = K5_START_MIN_ABS;
	out->start_count_from_tool = 1;
	out->start_random_seed = 0;
}

// ---------------------------------------------------------------- a machine

k5_status k5_chain_open(const char *file_path, k5_chain *out)
{
	K5_BEGIN
	K5_NEED(file_path && out,"k5_chain_open needs a file name and somewhere to put the handle")
	*out = NULL;
	std::unique_ptr<k5_chain_t> pHandle(new k5_chain_t());
	pHandle->m_oChain = CChainFileReader::ReadFile(file_path);
	*out = pHandle.release();
	return K5_OK;
	K5_END(K5_ERROR_FILE)
}

void k5_chain_free(k5_chain chain)
{
	delete chain;
}

k5_status k5_chain_name(k5_chain chain, char *out, size_t capacity)
{
	K5_BEGIN
	K5_NEED(chain && out && capacity,"k5_chain_name needs a chain and a buffer")
	CopyName(chain->m_oChain.GetName(),out,capacity);
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_chain_is_5c(k5_chain chain, int *out)
{
	K5_BEGIN
	K5_NEED(chain && out,"k5_chain_is_5c needs a chain and somewhere to put the answer")
	*out = chain->m_oChain.Is5CChain() ? 1 : 0;
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_chain_link_count(k5_chain chain, size_t *out)
{
	K5_BEGIN
	K5_NEED(chain && out,"k5_chain_link_count needs a chain and somewhere to put the count")
	*out = chain->m_oChain.GetNumberOfRegularLinks();
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_chain_link(k5_chain chain, size_t index, k5_link *out)
{
	K5_BEGIN
	K5_NEED(chain && out,"k5_chain_link needs a chain and somewhere to put the answer")
	K5_NEED(index<chain->m_oChain.GetNumberOfRegularLinks(),"there is no joint of that index in this chain")
	const CChainSegment &oSegment = chain->m_oChain[(int)index];
	out->type = (oSegment.GetType()==CHAIN_SEGMENT_REVOLUTE_LINK) ? K5_LINK_REVOLUTE : K5_LINK_PRISMATIC;
	out->constrained = oSegment.IsParameterConstrained() ? 1 : 0;
	out->minimum = oSegment.m_dParameterValueMinimum;
	out->maximum = oSegment.m_dParameterValueMaximum;
	out->value = oSegment.m_dActualParameterValue;
	CopyName(oSegment.GetName(),out->name,(size_t)K5_NAME_MAX);
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_chain_first_rotary_axis(k5_chain chain, int part, size_t *out)
{
	K5_BEGIN
	K5_NEED(chain && out,"k5_chain_first_rotary_axis needs a chain and somewhere to put the index")
	CHAIN_PART nPart = CHAIN_PART_TOOL;
	K5_NEED(FromCPart(part,nPart),"part is neither K5_PART_TOOL nor K5_PART_MATERIAL_HOLDER")
	*out = chain->m_oChain.GetFirstRotaryParameterIndex(nPart);
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_chain_frames(k5_chain chain, int part, double *out, size_t capacity_in_doubles,
	size_t *count_out)
{
	K5_BEGIN
	K5_NEED(chain && count_out,"k5_chain_frames needs a chain and somewhere to put the count")
	CHAIN_PART nPart = CHAIN_PART_TOOL;
	K5_NEED(FromCPart(part,nPart),"part is neither K5_PART_TOOL nor K5_PART_MATERIAL_HOLDER")
	const std::vector<CSegmentFrame> vFrames = chain->m_oChain.GetActualFrames(nPart);
	*count_out = vFrames.size();
	if(!out)
		return K5_OK;				//the count was the question
	const size_t nNeeded = vFrames.size()*32;
	if(capacity_in_doubles<nNeeded)
		return Fail(K5_ERROR_BUFFER_TOO_SMALL,"32 doubles are needed for every segment of the branch");
	size_t n = 0;
	for(size_t i=0;i<vFrames.size();i++){
		const CTransformation *pBoth[2] = {&vFrames[i].m_trJoint,&vFrames[i].m_trOutput};
		for(int f=0;f<2;f++)
			for(int r=0;r<4;r++)
				for(int c=0;c<4;c++)
					out[n++] = pBoth[f]->GetAt(r,c);
	}
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_chain_segment_boxes(k5_chain chain, int part, size_t index, double *out,
	size_t capacity_in_doubles, size_t *count_out)
{
	K5_BEGIN
	K5_NEED(chain && count_out,"k5_chain_segment_boxes needs a chain and somewhere to put the count")
	CHAIN_PART nPart = CHAIN_PART_TOOL;
	K5_NEED(FromCPart(part,nPart),"part is neither K5_PART_TOOL nor K5_PART_MATERIAL_HOLDER")
	const std::vector<CChainSegment> &vPart = chain->m_oChain.GetPart(nPart);
	K5_NEED(index<vPart.size(),"there is no segment of that index in that branch of the chain")
	const std::vector<CR3Box> &vBoxes = vPart[index].GetPartVector();
	*count_out = vBoxes.size();
	if(!out)
		return K5_OK;				//the count was the question
	if(capacity_in_doubles<vBoxes.size()*6)
		return Fail(K5_ERROR_BUFFER_TOO_SMALL,"6 doubles are needed for every box of the segment");
	size_t n = 0;
	for(size_t i=0;i<vBoxes.size();i++){
		const CR3Point a = vBoxes[i].GetMinPoint(), b = vBoxes[i].GetMaxPoint();
		out[n++] = a.GetX();
		out[n++] = a.GetY();
		out[n++] = a.GetZ();
		out[n++] = b.GetX();
		out[n++] = b.GetY();
		out[n++] = b.GetZ();
	}
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_chain_set_configuration(k5_chain chain, const double *q)
{
	K5_BEGIN
	K5_NEED(chain && q,"k5_chain_set_configuration needs a chain and five joint values")
	const k5_status nFive = NeedFiveAxes(chain->m_oChain);
	if(nFive!=K5_OK)
		return nFive;
	chain->m_oChain.SetParameters(FromQ(chain->m_oChain,q));
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_chain_tool_point(k5_chain chain, k5_tool tool, const double *q,
	double *position, double *axis)
{
	K5_BEGIN
	K5_NEED(chain && q,"k5_chain_tool_point needs a chain and five joint values")
	const k5_status nFive = NeedFiveAxes(chain->m_oChain);
	if(nFive!=K5_OK)
		return nFive;
	if(tool && tool->m_pTool)
		chain->m_oChain.SetToolAssembly(*tool->m_pTool);
	chain->m_oChain.SetParameters(FromQ(chain->m_oChain,q));
	const CR3AnchoredVector cv = chain->m_oChain.GetActualToolPositionAndOrientation();
	if(position){
		position[0] = cv.m_p.GetX();
		position[1] = cv.m_p.GetY();
		position[2] = cv.m_p.GetZ();
	}
	if(axis){
		axis[0] = cv.m_v.GetX();
		axis[1] = cv.m_v.GetY();
		axis[2] = cv.m_v.GetZ();
	}
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

// ---------------------------------------------------------------- the cutting simulation

k5_status k5_material_create(double x_size, double y_size, double z_size, double voxel_size,
	double x_start, double y_start, double z_start, k5_material *out)
{
	K5_BEGIN
	K5_NEED(out,"k5_material_create needs somewhere to put the handle")
	K5_NEED(x_size>0.0 && y_size>0.0 && z_size>0.0,"a block of material has three sizes greater than zero")
	K5_NEED(voxel_size>0.0,"the side of a voxel is greater than zero")
	const double dVoxels = (x_size/voxel_size+1.0)*(y_size/voxel_size+1.0)*(z_size/voxel_size+1.0);
	K5_NEED(dVoxels<2.0E9,"that many voxels is more than a block can hold: make the voxel larger")
	std::unique_ptr<k5_material_t> p(new k5_material_t());
	p->m_oMaterial.GenerateBox(x_size,y_size,z_size,voxel_size,x_start,y_start,z_start);
	*out = p.release();
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

void k5_material_free(k5_material material)
{
	delete material;
}

k5_status k5_material_reset(k5_material material)
{
	K5_BEGIN
	K5_NEED(material,"k5_material_reset needs a material")
	material->m_oMaterial.ResetMaterial();
	return K5_OK;
	K5_END(K5_ERROR_UNKNOWN)
}

k5_status k5_material_dimensions(k5_material material, size_t *nx, size_t *ny, size_t *nz)
{
	K5_BEGIN
	K5_NEED(material,"k5_material_dimensions needs a material")
	size_t x=0, y=0, z=0;
	material->m_oMaterial.GetDimensions(x,y,z);
	if(nx) *nx = x;
	if(ny) *ny = y;
	if(nz) *nz = z;
	return K5_OK;
	K5_END(K5_ERROR_UNKNOWN)
}

k5_status k5_material_volumes(k5_material material, double *total, double *removed)
{
	K5_BEGIN
	K5_NEED(material,"k5_material_volumes needs a material")
	if(total) *total = material->m_oMaterial.GetTotalVolume();
	if(removed) *removed = material->m_oMaterial.GetRemovedVolume();
	return K5_OK;
	K5_END(K5_ERROR_UNKNOWN)
}

k5_status k5_material_surface(k5_material material, double *out, size_t capacity_in_doubles,
	size_t *count_out)
{
	K5_BEGIN
	K5_NEED(material && count_out,"k5_material_surface needs a material and somewhere to put the count")
	const std::vector<CR3Point> &vSurface = material->m_oMaterial.GetSurfaceVoxels();
	*count_out = vSurface.size();
	if(!out)
		return K5_OK;				//the count was the question
	if(capacity_in_doubles<vSurface.size()*3)
		return Fail(K5_ERROR_BUFFER_TOO_SMALL,"3 doubles are needed for every voxel of the surface");
	size_t n = 0;
	for(size_t i=0;i<vSurface.size();i++){
		out[n++] = vSurface[i].GetX();
		out[n++] = vSurface[i].GetY();
		out[n++] = vSurface[i].GetZ();
	}
	return K5_OK;
	K5_END(K5_ERROR_UNKNOWN)
}

k5_status k5_material_surface_triangles(k5_material material, double *out,
	size_t capacity_in_doubles, size_t *count_out)
{
	K5_BEGIN
	K5_NEED(material && count_out,"k5_material_surface_triangles needs a material and somewhere to put the count")
	std::vector<double> v;
	const size_t nFaces = material->m_oMaterial.GetSurfaceTriangles(v);
	*count_out = nFaces;
	if(!out)
		return K5_OK;				//the count was the question
	if(capacity_in_doubles<v.size())
		return Fail(K5_ERROR_BUFFER_TOO_SMALL,"18 doubles are needed for every face of the surface");
	for(size_t i=0;i<v.size();i++)
		out[i] = v[i];
	return K5_OK;
	K5_END(K5_ERROR_UNKNOWN)
}

k5_status k5_simulation_create(k5_chain chain, k5_tool tool, k5_path path, k5_material material,
	k5_simulation *out)
{
	K5_BEGIN
	K5_NEED(chain && tool && path && material && out,
		"k5_simulation_create needs a chain, a tool, a path, a material and somewhere to put the handle")
	K5_NEED(tool->m_pTool,"the tool handle holds no tool")
	const k5_status nFive = NeedFiveAxes(chain->m_oChain);
	if(nFive!=K5_OK)
		return nFive;
	chain->m_oChain.SetToolAssembly(*tool->m_pTool);
	*out = new k5_simulation_t(chain,tool,path,material);
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

k5_status k5_simulation_create_from_locations(k5_tool tool, const k5_settings *settings,
	const double *positions, const double *axes, size_t count, k5_material material, k5_simulation *out)
{
	K5_BEGIN
	K5_NEED(tool && settings && material && out,
		"k5_simulation_create_from_locations needs a tool, settings, a material and somewhere to put the handle")
	K5_NEED(tool->m_pTool,"the tool handle holds no tool")
	K5_NEED(count==0 || (positions && axes),"a path of locations needs its positions and its axes")
	std::vector<CR3AnchoredVector> vLocations;
	vLocations.reserve(count);
	for(size_t i=0;i<count;i++){
		const CR3Vector v(axes[3*i],axes[3*i+1],axes[3*i+2]);
		if(v.IsZero(1E-12))
			return Fail(K5_ERROR_ARGUMENT,"the tool axis of location "+size_to_string(i+1)+" has no length");
		vLocations.push_back(CR3AnchoredVector(v,CR3Point(positions[3*i],positions[3*i+1],positions[3*i+2])));
	}
	*out = new k5_simulation_t(tool,vLocations,FromCSettings(*settings),material);
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

void k5_simulation_free(k5_simulation simulation)
{
	delete simulation;
}

k5_status k5_simulation_location(k5_simulation simulation, size_t index, double *position,
	double *axis, size_t *source, int *invented)
{
	K5_BEGIN
	K5_NEED(simulation,"k5_simulation_location needs a simulation")
	K5_NEED(index<simulation->m_pSimulation->GetStepCount(),"there is no location of that index in this simulation")
	CR3AnchoredVector cv;
	size_t nSource = index;
	bool bInvented = false;
	if(simulation->m_pFree){
		const CDenseLocation &o = simulation->m_pFree->GetLocations()[index];
		cv = o.m_cv;
		nSource = o.m_nSource;
		bInvented = o.m_bInvented;
	}else{
		k5_path_t *path = simulation->m_pPath;
		K5_NEED(path->m_oData.HasSolution(index),"that location has no solution, so the tool is nowhere")
		CChain &oChain = simulation->m_pChain->m_oChain;
		oChain.SetParameters(path->m_oData[index]);
		cv = oChain.GetActualToolPositionAndOrientation();
		if(index<path->m_oResult.m_vLocations.size()){
			const CSolvedLocation &o = path->m_oResult.m_vLocations[index];
			nSource = o.m_nFileLocation ? o.m_nFileLocation-1 : 0;
			bInvented = (o.m_nOrigin!=LOCATION_FROM_FILE);
		}
	}
	if(position){
		position[0] = cv.m_p.GetX();
		position[1] = cv.m_p.GetY();
		position[2] = cv.m_p.GetZ();
	}
	if(axis){
		axis[0] = cv.m_v.GetX();
		axis[1] = cv.m_v.GetY();
		axis[2] = cv.m_v.GetZ();
	}
	if(source) *source = nSource;
	if(invented) *invented = bInvented ? 1 : 0;
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

k5_status k5_simulation_reset(k5_simulation simulation)
{
	K5_BEGIN
	K5_NEED(simulation,"k5_simulation_reset needs a simulation")
	simulation->m_pSimulation->Reset();
	return K5_OK;
	K5_END(K5_ERROR_UNKNOWN)
}

k5_status k5_simulation_step(k5_simulation simulation, int *stepped)
{
	K5_BEGIN
	K5_NEED(simulation,"k5_simulation_step needs a simulation")
	const bool b = simulation->m_pSimulation->Step();
	if(stepped) *stepped = b ? 1 : 0;
	return K5_OK;
	K5_END(K5_ERROR_SOLVER)
}

k5_status k5_simulation_run(k5_simulation simulation)
{
	K5_BEGIN
	K5_NEED(simulation,"k5_simulation_run needs a simulation")
	simulation->m_pSimulation->Run(NULL);
	return K5_OK;
	K5_END(K5_ERROR_SOLVER)
}

k5_status k5_simulation_progress(k5_simulation simulation, size_t *step_index, size_t *step_count,
	int *finished)
{
	K5_BEGIN
	K5_NEED(simulation,"k5_simulation_progress needs a simulation")
	if(step_index) *step_index = simulation->m_pSimulation->GetStepIndex();
	if(step_count) *step_count = simulation->m_pSimulation->GetStepCount();
	if(finished) *finished = simulation->m_pSimulation->IsFinished() ? 1 : 0;
	return K5_OK;
	K5_END(K5_ERROR_UNKNOWN)
}

k5_status k5_simulation_gouges(k5_simulation simulation, size_t *count, size_t *first_step,
	double *first_point)
{
	K5_BEGIN
	K5_NEED(simulation && count,"k5_simulation_gouges needs a simulation and somewhere to put the count")
	*count = simulation->m_pSimulation->GetGougeCount();
	if(first_step) *first_step = simulation->m_pSimulation->GetFirstGougeStep();
	if(first_point){
		const CR3Point &p = simulation->m_pSimulation->GetFirstGougePoint();
		first_point[0] = p.GetX();
		first_point[1] = p.GetY();
		first_point[2] = p.GetZ();
	}
	return K5_OK;
	K5_END(K5_ERROR_UNKNOWN)
}

k5_status k5_chain_default_gcode_axes(k5_chain chain, k5_gcode_axes *out)
{
	K5_BEGIN
	K5_NEED(chain && out,"k5_chain_default_gcode_axes needs a chain and somewhere to put the answer")
	const k5_status nFive = NeedFiveAxes(chain->m_oChain);
	if(nFive!=K5_OK)
		return nFive;
	size_t nX=0, nY=0, nZ=0, n1=0, n2=0;
	chain->m_oChain.GetTranslationsIndexes(nX,nY,nZ);
	chain->m_oChain.GetRotationsIndexes(n1,n2);
	out->x = nX;
	out->y = nY;
	out->z = nZ;
	out->angle1 = n1;
	out->angle2 = n2;
	//nothing in the chain file says what the control calls its rotary axes: a guess to be corrected
	out->angle1_letter = 'A';
	out->angle2_letter = 'C';
	out->degrees = 1;
	return K5_OK;
	K5_END(K5_ERROR_CHAIN)
}

// ---------------------------------------------------------------- a tool

k5_status k5_tool_create(int shape, double rz, double r, double cutting_height,
	double shank_length, double holder_length, double holder_radius, k5_tool *out)
{
	K5_BEGIN
	K5_NEED(out,"k5_tool_create needs somewhere to put the handle")
	*out = NULL;
	K5_NEED(shape==K5_TOOL_BALL_NOSE || shape==K5_TOOL_FLAT_END,"shape is neither K5_TOOL_BALL_NOSE nor K5_TOOL_FLAT_END")
	const CToolHolder oHolder(holder_length,holder_radius);
	//the programmed point follows the shape: the centre of the ball, or the tip
	std::unique_ptr<k5_tool_t> pHandle(new k5_tool_t());
	if(shape==K5_TOOL_BALL_NOSE)
		pHandle->m_pTool.reset(new CMillingToolBall(rz,r,cutting_height,r,shank_length,oHolder));
	else
		pHandle->m_pTool.reset(new CMillingToolCylinder(rz,0.0,cutting_height,0.0,shank_length,oHolder));
	*out = pHandle.release();
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

void k5_tool_free(k5_tool tool)
{
	delete tool;
}

k5_status k5_tool_assembly_length(k5_tool tool, double *out)
{
	K5_BEGIN
	K5_NEED(tool && tool->m_pTool && out,"k5_tool_assembly_length needs a tool and somewhere to put the length")
	*out = tool->m_pTool->GetAssemblyLength();
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

k5_status k5_tool_mount_to_point_distance(k5_tool tool, double *out)
{
	K5_BEGIN
	K5_NEED(tool && tool->m_pTool && out,"k5_tool_mount_to_point_distance needs a tool and somewhere to put the distance")
	*out = tool->m_pTool->GetMountToToolPointDistance();
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

// ---------------------------------------------------------------- one cutter location

k5_status k5_solve_location(k5_chain chain, k5_tool tool, const k5_settings *settings,
	const double *position, const double *axis,
	double *q_out, size_t capacity_in_configurations, size_t *count_out, int *status_out)
{
	K5_BEGIN
	K5_NEED(chain && settings && position && axis && count_out
		,"k5_solve_location needs a chain, settings, a location and somewhere to put the count")
	*count_out = 0;
	if(status_out)
		*status_out = K5_NO_ORIENTATION;
	const k5_status nFive = NeedFiveAxes(chain->m_oChain);
	if(nFive!=K5_OK)
		return nFive;
	CMillingTool *pTool = (tool && tool->m_pTool) ? tool->m_pTool.get() : NULL;
	K5_NEED(pTool || !settings->check_collisions,"the collision check needs a tool")
	if(pTool)
		chain->m_oChain.SetToolAssembly(*pTool);

	const CPostprocessorSettings oSettings = FromCSettings(*settings);
	C5CPostprocessor oPostprocessor(chain->m_oChain,oSettings);
	std::string sLog;
	CSolveReport oReport;
	const tChainConfigurationVector vSolutions = oPostprocessor.Solve(
		CR3Point(position[0],position[1],position[2])
		,CR3Vector(axis[0],axis[1],axis[2])
		,chain->m_oChain,pTool,sLog,oReport);
	*count_out = vSolutions.size();
	if(status_out)
		*status_out = ToCStatus(oReport.Status());
	if(!q_out)
		return K5_OK;				//the count was the question
	if(capacity_in_configurations<vSolutions.size())
		return Fail(K5_ERROR_BUFFER_TOO_SMALL,"there are more solutions than the buffer can hold");
	for(size_t i=0;i<vSolutions.size();i++)
		ToQ(vSolutions[i],q_out+i*(size_t)K5_AXES);
	return K5_OK;
	K5_END(K5_ERROR_SOLVER)
}

// ---------------------------------------------------------------- a whole path

k5_status k5_postprocess_file(k5_chain chain, k5_tool tool, const k5_settings *settings,
	const char *cl_file_path, k5_path *out)
{
	K5_BEGIN
	K5_NEED(chain && settings && cl_file_path && out
		,"k5_postprocess_file needs a chain, settings, a file name and somewhere to put the handle")
	*out = NULL;
	CCutterLocationData oLocations;
	oLocations.LoadFile(cl_file_path,cl_file_path);
	return RunPostprocessor(chain,tool,*settings,oLocations,out);
	K5_END(K5_ERROR_SOLVER)
}

k5_status k5_postprocess_locations(k5_chain chain, k5_tool tool, const k5_settings *settings,
	const double *positions, const double *axes, size_t count, k5_path *out)
{
	K5_BEGIN
	K5_NEED(chain && settings && out
		,"k5_postprocess_locations needs a chain, settings and somewhere to put the handle")
	K5_NEED(!count || (positions && axes),"k5_postprocess_locations needs the locations it is to solve")
	*out = NULL;
	std::vector<CR3AnchoredVector> vLocations;
	vLocations.reserve(count);
	for(size_t i=0;i<count;i++){
		const CR3Vector r3vAxis(axes[3*i],axes[3*i+1],axes[3*i+2]);
		if(r3vAxis.IsZero())
			return Fail(K5_ERROR_ARGUMENT,"the tool axis of location "+size_to_string(i+1)
				+" has no length, so it is no direction");
		vLocations.push_back(CR3AnchoredVector(r3vAxis
			,CR3Point(positions[3*i],positions[3*i+1],positions[3*i+2])));
	}
	CCutterLocationData oLocations;
	oLocations.SetLocations(vLocations,"a tool path handed over");
	return RunPostprocessor(chain,tool,*settings,oLocations,out);
	K5_END(K5_ERROR_SOLVER)
}

void k5_path_free(k5_path path)
{
	delete path;
}

k5_status k5_path_size(k5_path path, size_t *out)
{
	K5_BEGIN
	K5_NEED(path && out,"k5_path_size needs a path and somewhere to put the count")
	*out = path->m_oData.GetSize();
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

k5_status k5_path_location(k5_path path, size_t index, k5_location *out)
{
	K5_BEGIN
	K5_NEED(path && out,"k5_path_location needs a path and somewhere to put the answer")
	K5_NEED(index<path->m_oData.GetSize() && index<path->m_oResult.m_vLocations.size()
		,"there is no location of that index in this path")
	const CSolvedLocation &oRecord = path->m_oResult.m_vLocations[index];
	out->status = ToCStatus(oRecord.m_nStatus);
	out->origin = ToCOrigin(oRecord.m_nOrigin);
	out->file_location = oRecord.m_nFileLocation;
	out->file_line = oRecord.m_nFileLine;
	out->has_solution = path->m_oData.HasSolution(index) ? 1 : 0;
	for(int i=0;i<K5_AXES;i++)
		out->q[i] = 0.0;
	if(out->has_solution)
		ToQ(path->m_oData[index],out->q);
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

k5_status k5_path_count_of_status(k5_path path, int status, size_t *out)
{
	K5_BEGIN
	K5_NEED(path && out,"k5_path_count_of_status needs a path and somewhere to put the count")
	SOLVE_STATUS nStatus = SOLVE_STATUS_SOLVED;
	K5_NEED(FromCStatus(status,nStatus),"status is not one of k5_solve_status")
	*out = path->m_oResult.CountOf(nStatus);
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

k5_status k5_path_count_of_origin(k5_path path, int origin, size_t *out)
{
	K5_BEGIN
	K5_NEED(path && out,"k5_path_count_of_origin needs a path and somewhere to put the count")
	LOCATION_ORIGIN nOrigin = LOCATION_FROM_FILE;
	K5_NEED(FromCOrigin(origin,nOrigin),"origin is not one of k5_location_origin")
	*out = path->m_oResult.CountOf(nOrigin);
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

k5_status k5_path_errors(k5_path path, double *max_position_error, double *max_orientation_error)
{
	K5_BEGIN
	K5_NEED(path,"k5_path_errors needs a path")
	if(max_position_error)
		*max_position_error = path->m_oResult.m_dMaxPositionError;
	if(max_orientation_error)
		*max_orientation_error = path->m_oResult.m_dMaxOrientationErrorVectorLength;
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

k5_status k5_path_last_configuration(k5_path path, double *q)
{
	K5_BEGIN
	K5_NEED(path && q,"k5_path_last_configuration needs a path and five doubles")
	CChainConfiguration oConfig;
	if(!path->m_oData.GetLastChosenConfiguration(oConfig))
		return Fail(K5_ERROR_SOLVER,"no location of this path has a solution, so the machine goes nowhere");
	ToQ(oConfig,q);
	return K5_OK;
	K5_END(K5_ERROR_ARGUMENT)
}

k5_status k5_path_write_gcode(k5_path path, const k5_gcode_axes *axes, const char *file_path)
{
	K5_BEGIN
	K5_NEED(path && axes && file_path,"k5_path_write_gcode needs a path, the axes and a file name")
	const size_t nAxes = (size_t)K5_AXES;
	K5_NEED(axes->x<nAxes && axes->y<nAxes && axes->z<nAxes && axes->angle1<nAxes && axes->angle2<nAxes
		,"every axis of k5_gcode_axes has to be one of the five joint values")
	K5_NEED(axes->angle1_letter && axes->angle2_letter,"the two rotary axes need the letters of the control")
	FILE *pFile = fopen(file_path,"wt");
	if(!pFile)
		return Fail(K5_ERROR_FILE,std::string("cannot write the G-code to ")+file_path);
	const int nResult = path->m_oData.ExportToGCode(pFile,axes->x,axes->y,axes->z
		,axes->angle1_letter,axes->angle1,axes->angle2_letter,axes->angle2,axes->degrees==0);
	fclose(pFile);
	if(nResult<0)
		return Fail(K5_ERROR_FILE,std::string("writing the G-code to ")+file_path+" failed");
	return K5_OK;
	K5_END(K5_ERROR_SOLVER)
}

} // extern "C"
