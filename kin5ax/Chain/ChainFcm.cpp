#include "../stdafx.h"
#include "ChainFcm.h"
#include "string_tools.h"
#include "../3rdParty/nlohmann/json.hpp"

//The bridge to the FreeCAD CAM machine file. Both directions work on frames built from the
//segments themselves (GetConstantTransformation and its inverse), so no matrix is decomposed
//here: going to .fcm only applies a frame to the origin and to OZ, and coming from .fcm only
//needs the rotation that takes OZ onto the axis of the joint.
namespace kin5ax {

CFcmOptions::CFcmOptions()
	:m_bNegateTableLinearAxes(false)
	,m_bNegateTableRotaryAxes(false)
{
}

void CFcmReport::Add(const std::string &sRemark)
{
	m_vRemarks.push_back(sRemark);
}

void CFcmReport::Clear()
{
	m_vRemarks.clear();
}

bool CFcmReport::IsExact() const
{
	return m_vRemarks.empty();
}

std::string CFcmReport::ToString(const std::string &sSeparator) const
{
	std::string sResult;
	for(size_t i=0;i<m_vRemarks.size();i++){
		if(i)
			sResult += sSeparator;
		sResult += m_vRemarks[i];
	}
	return sResult;
}

namespace {

typedef nlohmann::ordered_json tJson;

struct CContext
{
	std::string m_sWhere;

	[[noreturn]] void Fail(const std::string &sPath, const std::string &sMessage) const
	{
		std::string s;
		if(!m_sWhere.empty())
			s += m_sWhere+": ";
		if(!sPath.empty())
			s += sPath+": ";
		MY_THROW(CGeneratorException,s+sMessage);
	}
};

double Clean(double d)
{
	return fabs(d)<1E-12 ? 0.0 : d;		//no "-0" and no 1e-17 in the file
}

tJson ToJson(const CR3Vector &v)
{
	return tJson::array({Clean(v.GetX()),Clean(v.GetY()),Clean(v.GetZ())});
}

tJson ToJson(const CR3Point &p)
{
	return tJson::array({Clean(p.GetX()),Clean(p.GetY()),Clean(p.GetZ())});
}

CR3Vector JsonToVector(const CContext &ctx, const tJson &j, const std::string &sPath)
{
	if(!j.is_array() || j.size()!=3)
		ctx.Fail(sPath,"an array of three numbers is expected");
	for(size_t i=0;i<3;i++)
		if(!j[i].is_number())
			ctx.Fail(sPath,"an array of three numbers is expected");
	return CR3Vector(j[0].get<double>(),j[1].get<double>(),j[2].get<double>());
}

//the rotation taking OZ onto the direction v: its axis, and its angle in degrees
void MinimalRotationFromOZ(const CR3Vector &v, CR3Vector &r3vAxis, double &dAngleDegrees)
{
	CR3Vector u = v.GetUnitVector();
	CR3Vector oz(0.0,0.0,1.0);
	CR3Vector cross = oz.vectorProduct(u);
	double dSin = cross.GetLength();
	double dCos = oz.scalarProduct(u);
	if(dSin<1E-12){
		r3vAxis = CR3Vector(1.0,0.0,0.0);		//any axis perpendicular to OZ will do
		dAngleDegrees = (dCos>0.0) ? 0.0 : 180.0;
		return;
	}
	r3vAxis = cross.GetUnitVector();
	dAngleDegrees = radians_to_degrees(atan2(dSin,dCos));
}

std::string ReadWholeFile(const CContext &ctx, const std::string &sFilePath)
{
	FILE *pFile = fopen(sFilePath.c_str(),"rb");
	if(!pFile)
		MY_THROW(CGeneratorException,"Cannot open file: "+sFilePath+"\n"+strerror(errno));
	std::string sData;
	char buffer[4096];
	size_t n;
	while((n = fread(buffer,1,sizeof(buffer),pFile))>0)
		sData.append(buffer,n);
	fclose(pFile);
	return sData;
}

size_t LineOf(const std::string &sText, size_t nByte)
{
	size_t nEnd = std::min(nByte,sText.size());
	return 1+(size_t)std::count(sText.begin(),sText.begin()+nEnd,'\n');
}

tJson ParseJson(const CContext &ctx, const std::string &sText)
{
	try{
		return tJson::parse(sText);
	} catch (const nlohmann::json::parse_error &e){
		std::string s;
		if(!ctx.m_sWhere.empty())
			s += ctx.m_sWhere+", ";
		MY_THROW(CGeneratorException,"JSON error in "+s+"line "+size_to_string(LineOf(sText,e.byte))+": "+e.what());
	}
}

//---- reading

struct CFcmAxis
{
	CFcmAxis() :m_bHasParent(false), m_nSequence(0), m_bRotary(false), m_bHasLimits(false)
		,m_dMinimum(0.0), m_dMaximum(0.0) {}
	std::string m_sName;
	std::string m_sParent;
	bool m_bHasParent;
	int m_nSequence;
	bool m_bRotary;
	CR3Point m_r3pOrigin;
	CR3Vector m_r3vDirection;
	bool m_bHasLimits;
	double m_dMinimum, m_dMaximum;
};

//root -> leaf by "parent"; "sequence" when the parent links do not form one chain
std::vector<CFcmAxis> OrderBranch(const std::vector<CFcmAxis> &vAxes, const std::string &sBranch, CFcmReport &oReport)
{
	if(vAxes.size()<2)
		return vAxes;
	std::vector<std::string> vNames;
	for(size_t i=0;i<vAxes.size();i++)
		vNames.push_back(vAxes[i].m_sName);

	std::vector<size_t> vRoots;
	for(size_t i=0;i<vAxes.size();i++){
		bool bParentKnown = vAxes[i].m_bHasParent
			&& std::find(vNames.begin(),vNames.end(),vAxes[i].m_sParent)!=vNames.end();
		if(!bParentKnown)
			vRoots.push_back(i);
	}

	std::vector<CFcmAxis> vResult;
	bool bByParents = (vRoots.size()==1);
	if(bByParents){
		std::string sCurrent = vAxes[vRoots[0]].m_sName;
		std::vector<std::string> vSeen;
		while(!sCurrent.empty() && std::find(vSeen.begin(),vSeen.end(),sCurrent)==vSeen.end()){
			vSeen.push_back(sCurrent);
			for(size_t i=0;i<vAxes.size();i++)
				if(vAxes[i].m_sName==sCurrent)
					vResult.push_back(vAxes[i]);
			std::string sNext;
			size_t nChildren = 0;
			for(size_t i=0;i<vAxes.size();i++)
				if(vAxes[i].m_bHasParent && vAxes[i].m_sParent==sCurrent
					&& std::find(vSeen.begin(),vSeen.end(),vAxes[i].m_sName)==vSeen.end()){
					sNext = vAxes[i].m_sName;
					nChildren++;
				}
			if(nChildren>1){
				bByParents = false;
				break;
			}
			sCurrent = sNext;
		}
		if(bByParents && vResult.size()!=vAxes.size())
			bByParents = false;
	}
	if(!bByParents){
		oReport.Add("the "+sBranch+" axes of the file do not form one chain through 'parent'; "
			"they were ordered by 'sequence' - check the order of the segments");
		vResult = vAxes;
		std::stable_sort(vResult.begin(),vResult.end()
			,[](const CFcmAxis &a, const CFcmAxis &b){ return a.m_nSequence<b.m_nSequence; });
	}
	return vResult;
}

//the joints of one branch, followed by its terminal segment
std::vector<CChainSegment> BuildBranch(const std::vector<CFcmAxis> &vAxes, bool bTable
									  ,const CFcmOptions &oOptions, CHAIN_SEGMENT_TYPE terminalType)
{
	std::vector<CChainSegment> vSegments;
	CTransformation trPreviousInverted(1.0);
	for(size_t i=0;i<vAxes.size();i++){
		const CFcmAxis &oAxis = vAxes[i];
		CR3Vector r3vDirection = oAxis.m_r3vDirection;
		if(bTable && (oAxis.m_bRotary ? oOptions.m_bNegateTableRotaryAxes : oOptions.m_bNegateTableLinearAxes))
			r3vDirection = -r3vDirection;

		//the joint seen from the frame of the previous segment
		CR3Vector r3vLocalDirection = trPreviousInverted*r3vDirection;
		CR3Point r3pLocalOrigin = trPreviousInverted*oAxis.m_r3pOrigin;
		CR3Vector r3vRotationAxis;
		double dRotationAngle = 0.0;
		MinimalRotationFromOZ(r3vLocalDirection,r3vRotationAxis,dRotationAngle);

		double dMinimum = oAxis.m_dMinimum, dMaximum = oAxis.m_dMaximum;
		if(oAxis.m_bRotary){					//degrees in the file, radians in the model
			dMinimum = degrees_to_radians(dMinimum);
			dMaximum = degrees_to_radians(dMaximum);
		}
		CChainSegment oSegment(oAxis.m_bRotary ? CHAIN_SEGMENT_REVOLUTE_LINK : CHAIN_SEGMENT_PRISMATIC_LINK
			,r3vRotationAxis,dRotationAngle,r3pLocalOrigin.toR3Vector()
			,oAxis.m_bHasLimits,dMinimum,dMaximum);
		oSegment.SetName(oAxis.m_sName);
		oSegment.m_dActualParameterValue = 0.0;
		vSegments.push_back(oSegment);
		trPreviousInverted = oSegment.GetInvertedConstantTransformation()*trPreviousInverted;
	}
	//Every joint turns its own OZ onto the axis of the joint, and that turn is carried by the
	//whole rest of the branch. At the home position an .fcm machine has the tool frame and the
	//workpiece frame on the base frame, so the terminal undoes everything accumulated so far:
	//its constant transformation is the inverse of the product of the ones before it.
	CR3Point r3pTerminal = trPreviousInverted*CR3Point(0.0);
	CRotation oTerminalRotation(trPreviousInverted);
	CChainSegment oTerminal(terminalType,oTerminalRotation.GetAxis(),oTerminalRotation.GetAngle()
		,r3pTerminal.toR3Vector());
	oTerminal.m_dActualParameterValue = 0.0;
	vSegments.push_back(oTerminal);
	return vSegments;
}

} // namespace

CChain CChainFcmReader::Parse(const std::string &sFcm, const std::string &sWhere
							 ,const CFcmOptions &oOptions, CFcmReport &oReport)
{
	CContext ctx;
	ctx.m_sWhere = sWhere;
	oReport.Clear();
	tJson root = ParseJson(ctx,sFcm);
	if(!root.is_object())
		ctx.Fail("","an object is expected at the top of an .fcm file");
	const tJson &machine = root.contains("machine") ? root["machine"] : root;
	if(!machine.is_object() || !machine.contains("axes"))
		ctx.Fail("machine.axes","the file has no machine axes");
	const tJson &axes = machine["axes"];
	//FreeCAD keeps its machines in millimetres and says "metric", or "imperial" for inches; a chain
	//read from one carries that unit, since the numbers are copied and not converted
	CHAIN_UNIT nUnit = CHAIN_UNIT_MM;
	if(machine.contains("units")){
		const std::string sUnits = machine["units"].is_string() ? machine["units"].get<std::string>() : "";
		if(sUnits=="imperial")
			nUnit = CHAIN_UNIT_IN;
		else if(sUnits!="metric")
			ctx.Fail("machine.units","'"+sUnits+"' is neither metric nor imperial");
	}
	if(!axes.is_object())
		ctx.Fail("machine.axes","an object of axes is expected");
	//a base frame of its own would move every origin and direction of the file
	if(machine.contains("kinematics") && machine["kinematics"].is_object()
		&& machine["kinematics"].contains("base_frame")){
		const tJson &frame = machine["kinematics"]["base_frame"];
		bool bMoved = false;
		if(frame.is_object()){
			if(frame.contains("origin") && frame["origin"].is_array())
				bMoved = bMoved || !JsonToVector(ctx,frame["origin"],"machine.kinematics.base_frame.origin").IsZero(1E-9);
			if(frame.contains("orientation_quaternion") && frame["orientation_quaternion"].is_array()
				&& frame["orientation_quaternion"].size()==4
				&& frame["orientation_quaternion"][3].is_number()
				&& fabs(fabs(frame["orientation_quaternion"][3].get<double>())-1.0)>1E-9)
				bMoved = true;
		}
		if(bMoved)
			oReport.Add("machine.kinematics.base_frame is not the identity and was ignored: "
				"the joints were read as they stand in the file");
	}
	for(tJson::const_iterator it = machine.begin(); it != machine.end(); ++it)
		if(it.key()=="toolheads" && it.value().is_array() && !it.value().empty())
			oReport.Add("the tool heads of the file have no place in a kinematic chain and were dropped");

	std::vector<CFcmAxis> vHead, vTable;
	for(tJson::const_iterator it = axes.begin(); it != axes.end(); ++it){
		const tJson &a = it.value();
		std::string sPath = "machine.axes."+it.key();
		if(!a.is_object())
			ctx.Fail(sPath,"an object is expected");
		CFcmAxis oAxis;
		oAxis.m_sName = it.key();
		if(oAxis.m_sName.empty())
			ctx.Fail(sPath,"the name of an axis is empty");

		std::string sType = a.value("type",std::string("linear"));
		oAxis.m_bRotary = (sType=="rotary" || sType=="angular");
		if(!oAxis.m_bRotary && sType!="linear")
			ctx.Fail(sPath+".type","unknown axis type '"+sType+"', expected \"linear\" or \"rotary\"");

		std::string sRole = a.value("role",std::string());
		if(sRole.empty()){
			oReport.Add("axis '"+oAxis.m_sName+"' has no 'role': it was put in the workpiece branch");
			sRole = "table";
		}
		bool bHead = (sRole.compare(0,4,"head")==0);

		if(a.contains("parent") && a["parent"].is_string()){
			oAxis.m_sParent = a["parent"].get<std::string>();
			oAxis.m_bHasParent = !oAxis.m_sParent.empty();
		}
		if(a.contains("sequence") && a["sequence"].is_number_integer())
			oAxis.m_nSequence = a["sequence"].get<int>();

		//"joint": [origin, direction]; an object {origin, axis} and the old [direction, origin]
		//order are accepted the same way the FreeCAD reader accepts them
		if(!a.contains("joint"))
			ctx.Fail(sPath,"member 'joint' is missing");
		const tJson &joint = a["joint"];
		if(joint.is_object()){
			if(!joint.contains("origin") || !(joint.contains("axis")||joint.contains("vector")))
				ctx.Fail(sPath+".joint","'origin' and 'axis' are expected");
			CR3Vector o = JsonToVector(ctx,joint["origin"],sPath+".joint.origin");
			oAxis.m_r3pOrigin = CR3Point(o.GetX(),o.GetY(),o.GetZ());
			oAxis.m_r3vDirection = JsonToVector(ctx
				,joint.contains("axis") ? joint["axis"] : joint["vector"],sPath+".joint.axis");
		} else if(joint.is_array() && joint.size()>=2){
			CR3Vector v0 = JsonToVector(ctx,joint[0],sPath+".joint[0]");
			CR3Vector v1 = JsonToVector(ctx,joint[1],sPath+".joint[1]");
			if(!v1.IsZero(1E-6)){
				oAxis.m_r3pOrigin = CR3Point(v0.GetX(),v0.GetY(),v0.GetZ());
				oAxis.m_r3vDirection = v1;
			} else if(!v0.IsZero(1E-6)){
				oReport.Add("axis '"+oAxis.m_sName+"': 'joint' holds the direction before the origin");
				oAxis.m_r3pOrigin = CR3Point(v1.GetX(),v1.GetY(),v1.GetZ());
				oAxis.m_r3vDirection = v0;
			} else
				ctx.Fail(sPath+".joint","the axis has no direction");
		} else
			ctx.Fail(sPath+".joint","an array [origin, direction] or an object is expected");
		if(oAxis.m_r3vDirection.IsZero(1E-9))
			ctx.Fail(sPath+".joint","the direction of the axis is the zero vector");

		if(a.contains("limits") && a["limits"].is_object()){
			const tJson &limits = a["limits"];
			if(limits.contains("min") && limits.contains("max")
				&& limits["min"].is_number() && limits["max"].is_number()){
				oAxis.m_bHasLimits = true;
				oAxis.m_dMinimum = limits["min"].get<double>();
				oAxis.m_dMaximum = limits["max"].get<double>();
				if(oAxis.m_dMinimum>oAxis.m_dMaximum)
					ctx.Fail(sPath+".limits","min is above max");
			}
		}
		//everything else of the axis has no place in the model
		for(tJson::const_iterator itMember = a.begin(); itMember != a.end(); ++itMember){
			const std::string &sKey = itMember.key();
			if(sKey!="type" && sKey!="role" && sKey!="parent" && sKey!="sequence"
				&& sKey!="joint" && sKey!="limits" && sKey!="name")
				oReport.Add("axis '"+oAxis.m_sName+"': '"+sKey+"' is not part of a kinematic chain and was dropped");
		}
		if(bHead)
			vHead.push_back(oAxis);
		else
			vTable.push_back(oAxis);
	}

	vHead = OrderBranch(vHead,"head",oReport);
	vTable = OrderBranch(vTable,"table",oReport);

	CChain oChain;
	oChain.m_vToolPart = BuildBranch(vHead,false,oOptions,CHAIN_SEGMENT_TOOL);
	oChain.m_vMaterialHolderPart = BuildBranch(vTable,true,oOptions,CHAIN_SEGMENT_MATERIAL_HOLDER);
	std::string sName = oOptions.m_sMachineName.empty()
		? machine.value("name",std::string()) : oOptions.m_sMachineName;
	oChain.SetName(sName);
	oChain.SetMachine(machine.value("manufacturer",std::string()).empty()
		? machine.value("name",std::string())
		: machine.value("manufacturer",std::string())+" "+machine.value("name",std::string()));
	try{
		oChain.CheckStructure();
	} catch (CGeneratorException &e){
		ctx.Fail("",e.Message());
	}
	oChain.SetUnit(nUnit);
	oChain.SetParameters(CChainConfiguration((int)oChain.GetNumberOfRegularLinks()));

	if(!oChain.Is5CChain())
		oReport.Add("the chain read from the file is not a 5C chain ("
			+size_to_string(oChain.GetNumberOfPrismaticLinks())+" linear and "
			+size_to_string(oChain.GetNumberOfRevoluteLinks())+" rotary joints): the solver needs three and two");
	return oChain;
}

CChain CChainFcmReader::ReadFile(const std::string &sFilePath, const CFcmOptions &oOptions, CFcmReport &oReport)
{
	CContext ctx;
	ctx.m_sWhere = sFilePath;
	CChain oChain = Parse(ReadWholeFile(ctx,sFilePath),sFilePath,oOptions,oReport);
	oChain.SetFileName(sFilePath);
	return oChain;
}

namespace {

tJson MinimalFcm()
{
	tJson kinematics;
	tJson baseFrame;
	baseFrame["origin"] = tJson::array({0,0,0});
	baseFrame["orientation_quaternion"] = tJson::array({0,0,0,1});
	kinematics["base_frame"] = baseFrame;
	kinematics["tcp_supported"] = false;
	kinematics["dwo_supported"] = false;
	kinematics["notes"] = "";

	tJson machine;
	machine["name"] = "Machine";
	machine["manufacturer"] = "";
	machine["description"] = "";
	machine["units"] = "metric";
	machine["kinematics"] = kinematics;
	machine["axes"] = tJson::object();
	machine["toolheads"] = tJson::array();

	tJson postprocessor;
	postprocessor["file_name"] = "";
	postprocessor["properties"] = tJson::object();

	tJson fcm;
	fcm["freecad_version"] = "1.1";
	fcm["machine"] = machine;
	fcm["version"] = 1;
	fcm["postprocessor"] = postprocessor;
	return fcm;
}

bool IsGCodeAxisLetter(const std::string &sName)
{
	return sName.size()==1 && sName[0]>='A' && sName[0]<='Z';
}

} // namespace

std::string CChainFcmWriter::ToFcm(const CChain &oChain, const CFcmOptions &oOptions, CFcmReport &oReport)
{
	CContext ctx;
	ctx.m_sWhere = oChain.GetFileName();
	oReport.Clear();

	//the frame of the workpiece holder at the home position becomes the base frame of the file
	CTransformation trToBase(1.0);		//the inverse of that frame
	for(size_t i=0;i<oChain.m_vMaterialHolderPart.size();i++)
		trToBase = oChain.m_vMaterialHolderPart[i].GetInvertedConstantTransformation()*trToBase;

	tJson axes = tJson::object();
	std::vector<std::string> vUsedNames;
	bool bBoxesDropped = false;
	//every length of the file in millimetres, which is what FreeCAD reads, whatever the chain is in
	const double dToMillimetres = oChain.GetUnitInMillimetres();

	for(int nBranch=0;nBranch<2;nBranch++){
		bool bTable = (nBranch==1);
		const std::vector<CChainSegment> &vPart = bTable ? oChain.m_vMaterialHolderPart : oChain.m_vToolPart;
		CTransformation trFrame(1.0);		//the frame of the segment at the home position
		std::string sPreviousName;
		for(size_t i=0;i<vPart.size();i++){
			const CChainSegment &oSegment = vPart[i];
			trFrame = trFrame*oSegment.GetConstantTransformation();
			if(!oSegment.GetPartVector().empty())
				bBoxesDropped = true;
			if(i+1==vPart.size())
				break;						//the terminal is handled below

			CTransformation trInBase = trToBase*trFrame;
			CR3Point r3pOrigin = (trInBase*CR3Point(0.0))*dToMillimetres;
			CR3Vector r3vDirection = trInBase*CR3Vector(0.0,0.0,1.0);
			bool bRotary = (oSegment.GetType()==CHAIN_SEGMENT_REVOLUTE_LINK);
			if(bTable && (bRotary ? oOptions.m_bNegateTableRotaryAxes : oOptions.m_bNegateTableLinearAxes))
				r3vDirection = -r3vDirection;

			std::string sName = oSegment.GetName();
			if(sName.empty())
				ctx.Fail("","a segment of the "+std::string(bTable?"workpiece":"tool")
					+" branch has no name; every axis of an .fcm file is named");
			if(std::find(vUsedNames.begin(),vUsedNames.end(),sName)!=vUsedNames.end())
				ctx.Fail("","two segments are named '"+sName+"'; the axes of an .fcm file are "
					"an object keyed by name, so the names have to differ");
			vUsedNames.push_back(sName);
			if(!IsGCodeAxisLetter(sName))
				oReport.Add("axis '"+sName+"' is not a single upper-case letter; the FreeCAD "
					"post-processors expect X, Y, Z, A, B or C");

			tJson a;
			a["type"] = bRotary ? "rotary" : "linear";
			a["role"] = std::string(bTable ? "table_" : "head_")+(bRotary ? "rotary" : "linear");
			if(sPreviousName.empty())
				a["parent"] = nullptr;
			else
				a["parent"] = sPreviousName;
			a["sequence"] = (int)i;
			a["joint"] = tJson::array({ToJson(r3pOrigin),ToJson(r3vDirection)});
			if(oSegment.IsParameterConstrained()){
				tJson limits;
				limits["min"] = Clean(bRotary ? radians_to_degrees(oSegment.m_dParameterValueMinimum)
											  : oSegment.m_dParameterValueMinimum*dToMillimetres);
				limits["max"] = Clean(bRotary ? radians_to_degrees(oSegment.m_dParameterValueMaximum)
											  : oSegment.m_dParameterValueMaximum*dToMillimetres);
				a["limits"] = limits;
			} else {
				tJson limits;
				limits["min"] = bRotary ? -360 : 0;
				limits["max"] = bRotary ? 360 : 1000;
				a["limits"] = limits;
				oReport.Add("axis '"+sName+"' has no limits in the chain; the file was given the "
					"whole range, narrow it before using the machine");
			}
			a["max_velocity"] = bRotary ? 36000 : 10000;
			if(bRotary){
				a["solution_preference"] = "shortest";
				a["allow_flip"] = true;
				a["wrap_strategy"] = "unwound";
			}
			axes[sName] = a;
			sPreviousName = sName;
		}

		//the terminal: an .fcm machine has both terminals on the base frame at the home position
		if(!bTable){
			CTransformation trTool = trToBase*trFrame;
			CR3Vector r3vToolAxis = trTool*CR3Vector(0.0,0.0,1.0);
			CR3Point r3pToolPoint = trTool*CR3Point(0.0);
			if(fabs(r3vToolAxis.GetZ()-1.0)>1E-6)
				oReport.Add("the tool axis at the home position is ["
					+double_to_string(r3vToolAxis.GetX())+" "+double_to_string(r3vToolAxis.GetY())+" "
					+double_to_string(r3vToolAxis.GetZ())+"] in the workpiece frame, an .fcm machine "
					"has it on +Z: the file describes a different machine unless the chain is re-zeroed");
			if(r3pToolPoint.toR3Vector().GetLength()>1E-6)
				oReport.Add("the tool reference point at the home position is ["
					+double_to_string(r3pToolPoint.GetX())+" "+double_to_string(r3pToolPoint.GetY())+" "
					+double_to_string(r3pToolPoint.GetZ())+"] from the workpiece origin; an .fcm machine "
					"has it at the origin: carry it as a work offset or as a tool length");
		}
	}
	if(bBoxesDropped)
		oReport.Add("the collision boxes of the chain were dropped: an .fcm file has no room for them");

	tJson fcm;
	if(oOptions.m_sTemplateFilePath.empty())
		fcm = MinimalFcm();
	else {
		CContext ctxTemplate;
		ctxTemplate.m_sWhere = oOptions.m_sTemplateFilePath;
		fcm = ParseJson(ctxTemplate,ReadWholeFile(ctxTemplate,oOptions.m_sTemplateFilePath));
		if(!fcm.is_object() || !fcm.contains("machine") || !fcm["machine"].is_object())
			ctxTemplate.Fail("machine","the template is not an .fcm file");
	}
	tJson &machine = fcm["machine"];
	if(!oOptions.m_sMachineName.empty())
		machine["name"] = oOptions.m_sMachineName;
	else if(oOptions.m_sTemplateFilePath.empty())
		machine["name"] = oChain.GetName().empty() ? std::string("Machine") : oChain.GetName();
	machine["axes"] = axes;
	if(!machine.contains("kinematics") || !machine["kinematics"].is_object())
		machine["kinematics"] = MinimalFcm()["machine"]["kinematics"];
	std::string sNotes;
	if(machine["kinematics"].contains("notes") && machine["kinematics"]["notes"].is_string())
		sNotes = machine["kinematics"]["notes"].get<std::string>();		//of the template
	if(!oReport.IsExact()){
		if(!sNotes.empty())
			sNotes += " ";
		sNotes += "kin5ax: "+oReport.ToString("; ");
	}
	machine["kinematics"]["notes"] = sNotes;
	return json_numbers_on_one_line(fcm.dump(2))+"\n";
}

void CChainFcmWriter::WriteFile(const CChain &oChain, const std::string &sFilePath
							   ,const CFcmOptions &oOptions, CFcmReport &oReport)
{
	std::string sFcm = ToFcm(oChain,oOptions,oReport);
	FILE *pFile = fopen(sFilePath.c_str(),"wb");
	if(!pFile)
		MY_THROW(CGeneratorException,"Cannot open file to write: "+sFilePath+"\n"+strerror(errno));
	size_t nWritten = fwrite(sFcm.data(),1,sFcm.size(),pFile);
	int nClose = fclose(pFile);
	if(nWritten!=sFcm.size() || nClose!=0)
		MY_THROW(CGeneratorException,"Error while writing file: "+sFilePath);
}

} // namespace kin5ax
