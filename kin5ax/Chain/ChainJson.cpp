#include "../stdafx.h"
#include "ChainJson.h"
#include "string_tools.h"
#include "../3rdParty/nlohmann/json.hpp"

//nlohmann/json has no schema validation, so kc.schema.json is the documentation of the format
//and the reader below checks the same rules itself: known members only, the required ones
//present, the right types, both limits together and ordered, vectors of exactly three numbers.
//Every message names the member by its path, e.g. tool_branch.links[1].rotation.axis.
namespace kin5ax {

namespace {

typedef nlohmann::ordered_json tJson;		//ordered: the file keeps the order the writer uses
typedef std::initializer_list<const char*> tNames;

const char *const szFORMAT = "kin5ax-chain";
const int nVERSION = 2;

bool Contains(const tNames &names, const std::string &sName)
{
	for(const char *szItem : names)
		if(sName == szItem)
			return true;
	return false;
}

std::string Index(const std::string &sPath, size_t n)
{
	return sPath+"["+size_to_string(n)+"]";
}

struct CContext
{
	std::string m_sWhere;			//file name, empty for text from memory

	[[noreturn]] void Fail(const std::string &sPath, const std::string &sMessage) const
	{
		std::string s;
		if(!m_sWhere.empty())
			s += m_sWhere+": ";
		if(!sPath.empty())
			s += sPath+": ";
		MY_THROW(CGeneratorException,s+sMessage);
	}

	//an object with the required members and no unknown ones
	void CheckObject(const tJson &j, const std::string &sPath, const tNames &required, const tNames &optional) const
	{
		if(!j.is_object())
			Fail(sPath,"an object is expected");
		for(const char *szName : required)
			if(!j.contains(szName))
				Fail(sPath,"member '"+std::string(szName)+"' is missing");
		for(tJson::const_iterator it = j.begin(); it != j.end(); ++it)
			if(!Contains(required,it.key()) && !Contains(optional,it.key()))
				Fail(sPath,"unknown member '"+it.key()+"'");
	}

	const tJson &Member(const tJson &j, const char *szName, const std::string &sPath) const
	{
		tJson::const_iterator it = j.find(szName);
		if(it == j.end())
			Fail(sPath,"member '"+std::string(szName)+"' is missing");
		return *it;
	}

	double Number(const tJson &j, const std::string &sPath) const
	{
		if(!j.is_number())
			Fail(sPath,"a number is expected");
		return j.get<double>();
	}

	std::string String(const tJson &j, const std::string &sPath) const
	{
		if(!j.is_string())
			Fail(sPath,"a string is expected");
		return j.get<std::string>();
	}

	//an optional string member, empty when absent
	std::string OptionalString(const tJson &j, const char *szName, const std::string &sPath) const
	{
		tJson::const_iterator it = j.find(szName);
		if(it == j.end())
			return std::string();
		return String(*it,sPath+"."+szName);
	}

	CR3Vector Vector3(const tJson &j, const std::string &sPath) const
	{
		if(!j.is_array() || j.size()!=3)
			Fail(sPath,"an array of three numbers is expected");
		return CR3Vector(Number(j[0],Index(sPath,0)),Number(j[1],Index(sPath,1)),Number(j[2],Index(sPath,2)));
	}
};

//name, translation, rotation and the collision boxes: common to a joint and to the end of a branch
void ReadCommon(const CContext &ctx, const tJson &j, const std::string &sPath, CChainSegment &oSegment)
{
	std::string sName = ctx.String(ctx.Member(j,"name",sPath),sPath+".name");
	if(sName.empty())
		ctx.Fail(sPath+".name","the name is empty");
	oSegment.SetName(sName);
	oSegment.m_dActualParameterValue = 0.0;
	oSegment.m_r3vConstTranslation = ctx.Vector3(ctx.Member(j,"translation",sPath),sPath+".translation");

	const tJson &rotation = ctx.Member(j,"rotation",sPath);
	std::string sRotation = sPath+".rotation";
	ctx.CheckObject(rotation,sRotation,{"axis","angle_deg"},{});
	oSegment.m_r3vConstRevoluteAxis = ctx.Vector3(ctx.Member(rotation,"axis",sRotation),sRotation+".axis");
	oSegment.m_dConstRevoluteAngle = ctx.Number(ctx.Member(rotation,"angle_deg",sRotation),sRotation+".angle_deg");
	if(is_zero(oSegment.m_r3vConstRevoluteAxis.GetLengthSquare())){
		if(!is_zero(oSegment.m_dConstRevoluteAngle))
			ctx.Fail(sRotation+".axis","the rotation axis is the zero vector");
		oSegment.m_r3vConstRevoluteAxis = CR3Vector(1.0,0.0,0.0);	//no rotation: any axis will do
	}

	std::vector<CR3Box> vBoxes;
	tJson::const_iterator itBoxes = j.find("boxes");
	if(itBoxes != j.end()){
		std::string sBoxes = sPath+".boxes";
		if(!itBoxes->is_array())
			ctx.Fail(sBoxes,"an array is expected");
		for(size_t i=0;i<itBoxes->size();i++){
			std::string sBox = Index(sBoxes,i);
			const tJson &box = (*itBoxes)[i];
			ctx.CheckObject(box,sBox,{"min","max"},{});
			CR3Vector min = ctx.Vector3(ctx.Member(box,"min",sBox),sBox+".min");
			CR3Vector max = ctx.Vector3(ctx.Member(box,"max",sBox),sBox+".max");
			vBoxes.push_back(CR3Box(min.GetX(),max.GetX(),min.GetY(),max.GetY(),min.GetZ(),max.GetZ()));
		}
	}
	oSegment.SetPartVector(vBoxes);
}

CChainSegment ReadLink(const CContext &ctx, const tJson &j, const std::string &sPath)
{
	ctx.CheckObject(j,sPath,{"type","name","translation","rotation"},{"limits","boxes"});
	std::string sType = ctx.String(ctx.Member(j,"type",sPath),sPath+".type");
	CHAIN_SEGMENT_TYPE type;
	if(sType=="rotary")
		type = CHAIN_SEGMENT_REVOLUTE_LINK;
	else if(sType=="linear")
		type = CHAIN_SEGMENT_PRISMATIC_LINK;
	else
		ctx.Fail(sPath+".type","unknown link type '"+sType+"', expected \"rotary\" or \"linear\"");

	CChainSegment oSegment(type,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0));
	oSegment.SetType(type);
	ReadCommon(ctx,j,sPath,oSegment);

	tJson::const_iterator itLimits = j.find("limits");
	oSegment.m_bParameterIsConstrained = (itLimits != j.end());
	if(oSegment.m_bParameterIsConstrained){
		std::string sLimits = sPath+".limits";
		ctx.CheckObject(*itLimits,sLimits,{"min","max"},{});
		double dMin = ctx.Number(ctx.Member(*itLimits,"min",sLimits),sLimits+".min");
		double dMax = ctx.Number(ctx.Member(*itLimits,"max",sLimits),sLimits+".max");
		if(dMin>dMax)
			ctx.Fail(sLimits,"min is above max");
		if(type==CHAIN_SEGMENT_REVOLUTE_LINK){		//degrees in the file, radians in the model
			dMin = degrees_to_radians(dMin);
			dMax = degrees_to_radians(dMax);
		}
		oSegment.m_dParameterValueMinimum = dMin;
		oSegment.m_dParameterValueMaximum = dMax;
	}
	return oSegment;
}

/**
 * The tool a chain file describes for the nose of its spindle: a shape, its sizes, how far it sticks
 * out of the holder and where the point a cutter location programs lies on it.
 */
CToolDescription ReadTool(const CContext &ctx, const tJson &j, const std::string &sPath)
{
	ctx.CheckObject(j,sPath,{"type","radius","cutting_height","length"}
		,{"corner_radius","reference_point","reference_height"});
	CToolDescription oTool;
	const std::string sType = ctx.String(ctx.Member(j,"type",sPath),sPath+".type");
	if(!MillingToolTypeFromName(sType,oTool.m_nType))
		ctx.Fail(sPath+".type","unknown tool type '"+sType
			+"', expected \"ball_end\", \"cylinder\" or \"toroidal\"");
	oTool.m_dRadius = ctx.Number(ctx.Member(j,"radius",sPath),sPath+".radius");
	oTool.m_dCuttingHeight = ctx.Number(ctx.Member(j,"cutting_height",sPath),sPath+".cutting_height");
	oTool.m_dLength = ctx.Number(ctx.Member(j,"length",sPath),sPath+".length");
	//the ball of a ball nose tool is its corner radius; a file that leaves it out means a full ball
	tJson::const_iterator itCorner = j.find("corner_radius");
	if(itCorner != j.end())
		oTool.m_dCornerRadius = ctx.Number(*itCorner,sPath+".corner_radius");
	else
		oTool.m_dCornerRadius = (oTool.m_nType==MILLING_TOOL_BALL) ? oTool.m_dRadius : 0.0;
	tJson::const_iterator itReference = j.find("reference_point");
	if(itReference != j.end()){
		const std::string sReference = ctx.String(*itReference,sPath+".reference_point");
		if(!ToolReferencePointFromName(sReference,oTool.m_nReference))
			ctx.Fail(sPath+".reference_point","unknown reference point '"+sReference
				+"', expected \"tip\", \"ball_centre\" or \"custom\"");
	}
	tJson::const_iterator itHeight = j.find("reference_height");
	if(itHeight != j.end())
		oTool.m_dReferenceHeight = ctx.Number(*itHeight,sPath+".reference_height");
	else if(oTool.m_nReference==TOOL_REFERENCE_CUSTOM)
		ctx.Fail(sPath+".reference_height","a custom reference point needs the height it sits at");
	oTool.SetGiven(true);
	const std::string sProblem = oTool.DimensionProblem();
	if(!sProblem.empty())
		ctx.Fail(sPath,sProblem);
	return oTool;
}

/**
 * The workpiece holder a chain file describes for its table: a designation, the frame the part sits
 * in and the solids the holder is. The frame is given the way a segment gives its constant one.
 */
CWorkpieceHolder ReadWorkpieceHolder(const CContext &ctx, const tJson &j, const std::string &sPath)
{
	ctx.CheckObject(j,sPath,{},{"model","translation","rotation","boxes"});
	CWorkpieceHolder oHolder;
	oHolder.SetModel(ctx.OptionalString(j,"model",sPath));
	CR3Vector r3vTranslation(0.0,0.0,0.0), r3vAxis(0.0,0.0,1.0);
	double dAngle = 0.0;
	tJson::const_iterator itTranslation = j.find("translation");
	if(itTranslation != j.end())
		r3vTranslation = ctx.Vector3(*itTranslation,sPath+".translation");
	tJson::const_iterator itRotation = j.find("rotation");
	if(itRotation != j.end()){
		const std::string sRotation = sPath+".rotation";
		ctx.CheckObject(*itRotation,sRotation,{"axis","angle_deg"},{});
		r3vAxis = ctx.Vector3(ctx.Member(*itRotation,"axis",sRotation),sRotation+".axis");
		dAngle = ctx.Number(ctx.Member(*itRotation,"angle_deg",sRotation),sRotation+".angle_deg");
		if(r3vAxis.IsZero(dEPSILON) && !is_zero(dAngle))
			ctx.Fail(sRotation+".axis","a turn about no axis");
	}
	oHolder.SetFrame(r3vTranslation,r3vAxis,dAngle);
	tJson::const_iterator itBoxes = j.find("boxes");
	if(itBoxes != j.end()){
		if(!itBoxes->is_array())
			ctx.Fail(sPath+".boxes","an array of boxes is expected");
		std::vector<CR3Box> vBoxes;
		for(size_t i=0;i<itBoxes->size();i++){
			const tJson &box = (*itBoxes)[i];
			const std::string sBox = sPath+".boxes["+size_to_string(i)+"]";
			ctx.CheckObject(box,sBox,{"min","max"},{});
			const CR3Vector min = ctx.Vector3(ctx.Member(box,"min",sBox),sBox+".min");
			const CR3Vector max = ctx.Vector3(ctx.Member(box,"max",sBox),sBox+".max");
			vBoxes.push_back(CR3Box(min.GetX(),max.GetX(),min.GetY(),max.GetY(),min.GetZ(),max.GetZ()));
		}
		oHolder.SetBoxes(vBoxes);
	}
	return oHolder;
}

CChainSegment ReadTerminal(const CContext &ctx, const tJson &j, const std::string &sPath, CHAIN_SEGMENT_TYPE type)
{
	//only the end of the tool branch carries the tool holder of the machine
	if(type==CHAIN_SEGMENT_TOOL_SPINDLE_NOSE)
		ctx.CheckObject(j,sPath,{"name","translation","rotation"},{"boxes","tool_holder","tool"});
	else
		ctx.CheckObject(j,sPath,{"name","translation","rotation"},{"boxes","workpiece_holder"});
	CChainSegment oSegment(type,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0));
	oSegment.SetType(type);
	oSegment.m_bParameterIsConstrained = false;
	ReadCommon(ctx,j,sPath,oSegment);
	tJson::const_iterator itHolder = j.find("tool_holder");
	if(itHolder != j.end()){
		const std::string sHolder = sPath+".tool_holder";
		//length and radius are the one cylinder this object carried before holders had a shape;
		//sections give it several, and model is the designation a host matches its own against.
		//A holder with no cylinder at all is NO_HOLDER: the tool is clamped into the spindle.
		ctx.CheckObject(*itHolder,sHolder,{},{"length","radius","model","sections"});
		std::vector<CToolHolderSection> vSections;
		tJson::const_iterator itSections = itHolder->find("sections");
		if(itSections != itHolder->end()){
			if(!itSections->is_array())
				ctx.Fail(sHolder+".sections","an array of sections is expected");
			if(itSections->size()>nTOOL_HOLDER_SECTIONS_MAX)
				ctx.Fail(sHolder+".sections","a tool holder is made of at most "
					+size_to_string(nTOOL_HOLDER_SECTIONS_MAX)+" sections");
			for(size_t i=0;i<itSections->size();i++){
				const std::string sOne = sHolder+".sections["+size_to_string(i)+"]";
				ctx.CheckObject((*itSections)[i],sOne,{"length","radius"},{});
				vSections.push_back(CToolHolderSection(
					ctx.Number(ctx.Member((*itSections)[i],"length",sOne),sOne+".length")
					,ctx.Number(ctx.Member((*itSections)[i],"radius",sOne),sOne+".radius")));
			}
		}
		const bool bOneCylinder = itHolder->contains("length") || itHolder->contains("radius");
		if(bOneCylinder && !vSections.empty())
			ctx.Fail(sHolder,"a tool holder has sections or a length and a radius, not both");
		if(bOneCylinder)
			vSections.push_back(CToolHolderSection(
				ctx.Number(ctx.Member(*itHolder,"length",sHolder),sHolder+".length")
				,ctx.Number(ctx.Member(*itHolder,"radius",sHolder),sHolder+".radius")));
		oSegment.SetToolHolder(CToolHolder(ctx.OptionalString(*itHolder,"model",sHolder),vSections));
	}
	tJson::const_iterator itTool = j.find("tool");
	if(itTool != j.end())
		oSegment.SetTool(ReadTool(ctx,*itTool,sPath+".tool"));
	tJson::const_iterator itWorkpiece = j.find("workpiece_holder");
	if(itWorkpiece != j.end())
		oSegment.SetWorkpieceHolder(ReadWorkpieceHolder(ctx,*itWorkpiece,sPath+".workpiece_holder"));
	return oSegment;
}

//the joints of a branch followed by its end: one segment vector of the model
std::vector<CChainSegment> ReadBranch(const CContext &ctx, const tJson &jRoot, const char *szBranch,
									  const char *szTerminal, CHAIN_SEGMENT_TYPE terminalType)
{
	const tJson &branch = ctx.Member(jRoot,szBranch,"");
	std::string sPath = szBranch;
	ctx.CheckObject(branch,sPath,{"links",szTerminal},{});
	const tJson &links = ctx.Member(branch,"links",sPath);
	std::string sLinks = sPath+".links";
	if(!links.is_array())
		ctx.Fail(sLinks,"an array is expected");
	std::vector<CChainSegment> vSegments;
	for(size_t i=0;i<links.size();i++)
		vSegments.push_back(ReadLink(ctx,links[i],Index(sLinks,i)));
	vSegments.push_back(ReadTerminal(ctx,ctx.Member(branch,szTerminal,sPath),sPath+"."+szTerminal,terminalType));
	return vSegments;
}

//the line of a byte offset, for the message of a syntax error
size_t LineOf(const std::string &sText, size_t nByte)
{
	size_t nEnd = std::min(nByte,sText.size());
	return 1+(size_t)std::count(sText.begin(),sText.begin()+nEnd,'\n');
}

//---- writing

tJson Vector3ToJson(const CR3Vector &v)
{
	return tJson::array({v.GetX(),v.GetY(),v.GetZ()});
}

void CommonToJson(const CChainSegment &oSegment, tJson &j)
{
	j["name"] = oSegment.GetName();
	j["translation"] = Vector3ToJson(oSegment.m_r3vConstTranslation);
	tJson rotation;
	rotation["axis"] = Vector3ToJson(oSegment.m_r3vConstRevoluteAxis);
	rotation["angle_deg"] = oSegment.m_dConstRevoluteAngle;
	j["rotation"] = rotation;
}

void BoxesToJson(const CChainSegment &oSegment, tJson &j)
{
	const std::vector<CR3Box> &vBoxes = oSegment.GetPartVector();
	if(vBoxes.empty())
		return;
	tJson boxes = tJson::array();
	for(size_t i=0;i<vBoxes.size();i++){
		tJson box;
		box["min"] = Vector3ToJson(vBoxes[i].GetMinPoint().toR3Vector());
		box["max"] = Vector3ToJson(vBoxes[i].GetMaxPoint().toR3Vector());
		boxes.push_back(box);
	}
	j["boxes"] = boxes;
}

tJson LinkToJson(const CChainSegment &oSegment)
{
	tJson j;
	j["type"] = (oSegment.GetType()==CHAIN_SEGMENT_REVOLUTE_LINK) ? "rotary" : "linear";
	CommonToJson(oSegment,j);
	if(oSegment.IsParameterConstrained()){
		tJson limits;
		if(oSegment.GetType()==CHAIN_SEGMENT_REVOLUTE_LINK){
			limits["min"] = radians_to_degrees(oSegment.m_dParameterValueMinimum);
			limits["max"] = radians_to_degrees(oSegment.m_dParameterValueMaximum);
		} else {
			limits["min"] = oSegment.m_dParameterValueMinimum;
			limits["max"] = oSegment.m_dParameterValueMaximum;
		}
		j["limits"] = limits;
	}
	BoxesToJson(oSegment,j);
	return j;
}

tJson TerminalToJson(const CChainSegment &oSegment)
{
	tJson j;
	CommonToJson(oSegment,j);
	if(oSegment.IsToolHolderDeclared()){
		tJson holder;
		const CToolHolder &oHolder = oSegment.GetToolHolder();
		if(!oHolder.GetModel().empty())
			holder["model"] = oHolder.GetModel();
		const std::vector<CToolHolderSection> &vSections = oHolder.GetSections();
		if(vSections.size()==1){
			//one cylinder keeps the shape this object always had
			holder["length"] = vSections[0].GetLength();
			holder["radius"] = vSections[0].GetRadius();
		} else if(!vSections.empty()){
			tJson sections = tJson::array();
			for(size_t i=0;i<vSections.size();i++){
				tJson one;
				one["length"] = vSections[i].GetLength();
				one["radius"] = vSections[i].GetRadius();
				sections.push_back(one);
			}
			holder["sections"] = sections;
		}
		j["tool_holder"] = holder;
	}
	if(oSegment.IsWorkpieceHolderDeclared()){
		const CWorkpieceHolder &oHolder = oSegment.GetWorkpieceHolder();
		tJson holder;
		if(!oHolder.GetModel().empty())
			holder["model"] = oHolder.GetModel();
		if(!oHolder.IsFrameIdentity()){
			holder["translation"] = Vector3ToJson(oHolder.GetTranslation());
			tJson rotation;
			rotation["axis"] = Vector3ToJson(oHolder.GetRotationAxis());
			rotation["angle_deg"] = oHolder.GetRotationAngleDegrees();
			holder["rotation"] = rotation;
		}
		if(oHolder.HasBody()){
			tJson boxes = tJson::array();
			for(size_t i=0;i<oHolder.GetBoxes().size();i++){
				tJson box;
				const CR3Point min = oHolder.GetBoxes()[i].GetMinPoint();
				const CR3Point max = oHolder.GetBoxes()[i].GetMaxPoint();
				box["min"] = tJson::array({min.GetX(),min.GetY(),min.GetZ()});
				box["max"] = tJson::array({max.GetX(),max.GetY(),max.GetZ()});
				boxes.push_back(box);
			}
			holder["boxes"] = boxes;
		}
		j["workpiece_holder"] = holder;
	}
	if(oSegment.GetTool().IsGiven()){
		const CToolDescription &oTool = oSegment.GetTool();
		tJson tool;
		tool["type"] = MillingToolTypeName(oTool.m_nType);
		tool["radius"] = oTool.m_dRadius;
		if(oTool.m_dCornerRadius!=0.0)
			tool["corner_radius"] = oTool.m_dCornerRadius;
		tool["cutting_height"] = oTool.m_dCuttingHeight;
		tool["length"] = oTool.m_dLength;
		if(oTool.m_nReference!=TOOL_REFERENCE_TIP){
			tool["reference_point"] = ToolReferencePointName(oTool.m_nReference);
			if(oTool.m_nReference==TOOL_REFERENCE_CUSTOM)
				tool["reference_height"] = oTool.m_dReferenceHeight;
		}
		j["tool"] = tool;
	}
	BoxesToJson(oSegment,j);
	return j;
}

tJson BranchToJson(const std::vector<CChainSegment> &vPart, const char *szTerminal)
{
	tJson branch;
	tJson links = tJson::array();
	//the last segment is the end of the branch, everything before it is a joint
	for(size_t i=0;i+1<vPart.size();i++)
		links.push_back(LinkToJson(vPart[i]));
	branch["links"] = links;
	if(!vPart.empty())
		branch[szTerminal] = TerminalToJson(vPart.back());
	return branch;
}

} // namespace

CChain CChainJsonReader::Parse(const std::string &sJson, const std::string &sWhere)
{
	CContext ctx;
	ctx.m_sWhere = sWhere;
	tJson root;
	try{
		root = tJson::parse(sJson);
	} catch (const nlohmann::json::parse_error &e){
		std::string s;
		if(!sWhere.empty())
			s += sWhere+", ";
		MY_THROW(CGeneratorException,"JSON error in "+s+"line "+size_to_string(LineOf(sJson,e.byte))+": "+e.what());
	}
	//a foreign document is told that it is one before anything about what it lacks
	if(root.is_object() && root.contains("format") && root["format"].is_string()
		&& root["format"].get<std::string>()!=szFORMAT)
		ctx.Fail("format","'"+root["format"].get<std::string>()+"' is not a kinematic chain, expected '"+szFORMAT+"'");
	ctx.CheckObject(root,"",{"format","version","unit","tool_branch","workpiece_branch"}
		,{"$schema","name","machine","author","modified"});

	std::string sFormat = ctx.String(ctx.Member(root,"format",""),"format");
	if(sFormat != szFORMAT)
		ctx.Fail("format","'"+sFormat+"' is not a kinematic chain, expected '"+szFORMAT+"'");
	const tJson &version = ctx.Member(root,"version","");
	if(version.is_number_integer() && version.get<int>()==1)
		//version 1 called the terminals tool and workpiece_holder, and meant something else by the
		//first of them: its origin was the tool point in a file that named no holder
		ctx.Fail("version","this is version 1 of the format, which named the terminals differently "
			"and placed the tool one of them. Convert the file with tools/kc2json, or write the "
			"terminals as tool_spindle_nose and table and say version 2");
	if(!version.is_number_integer() || version.get<int>()!=nVERSION)
		ctx.Fail("version","unsupported version, expected "+size_to_string(nVERSION));

	//the default segments of a new chain are replaced below
	CChain oChain;
	oChain.SetName(ctx.OptionalString(root,"name",""));
	oChain.SetMachine(ctx.OptionalString(root,"machine",""));
	oChain.SetAuthor(ctx.OptionalString(root,"author",""));
	oChain.SetModified(ctx.OptionalString(root,"modified",""));
	//required, and one of three spellings exactly; see the XML reader for why no guessing
	const std::string sUnit = ctx.String(ctx.Member(root,"unit",""),"unit");
	CHAIN_UNIT nUnit;
	if(!ParseChainUnit(sUnit,nUnit))
		ctx.Fail("unit","'"+sUnit+"' is none of mm, m and in");
	oChain.SetUnit(nUnit);
	oChain.m_vToolPart = ReadBranch(ctx,root,"tool_branch","tool_spindle_nose",CHAIN_SEGMENT_TOOL_SPINDLE_NOSE);
	oChain.m_vMaterialHolderPart = ReadBranch(ctx,root,"workpiece_branch","table",CHAIN_SEGMENT_TABLE);
	try{
		oChain.CheckStructure();
	} catch (CGeneratorException &e){
		ctx.Fail("",e.Message());
	}
	oChain.SetParameters(CChainConfiguration((int)oChain.GetNumberOfRegularLinks()));
	return oChain;
}

CChain CChainJsonReader::ReadFile(const std::string &sFilePath)
{
	FILE *pFile = fopen(sFilePath.c_str(),"rb");
	if(!pFile)
		MY_THROW(CGeneratorException,"Cannot open file: "+sFilePath+"\n"+strerror(errno));
	std::string sJson;
	char buffer[4096];
	size_t n;
	while((n = fread(buffer,1,sizeof(buffer),pFile))>0)
		sJson.append(buffer,n);
	fclose(pFile);
	CChain oChain = Parse(sJson,sFilePath);
	oChain.SetFileName(sFilePath);
	return oChain;
}

std::string CChainJsonWriter::ToJson(const CChain &oChain)
{
	tJson root;
	root["format"] = szFORMAT;
	root["version"] = nVERSION;
	root["unit"] = ChainUnitName(oChain.GetUnit());
	if(!oChain.GetName().empty())
		root["name"] = oChain.GetName();
	if(!oChain.GetMachine().empty())
		root["machine"] = oChain.GetMachine();
	if(!oChain.GetAuthor().empty())
		root["author"] = oChain.GetAuthor();
	if(!oChain.GetModified().empty())
		root["modified"] = oChain.GetModified();
	root["tool_branch"] = BranchToJson(oChain.m_vToolPart,"tool_spindle_nose");
	root["workpiece_branch"] = BranchToJson(oChain.m_vMaterialHolderPart,"table");
	return json_numbers_on_one_line(root.dump(2))+"\n";
}

void CChainJsonWriter::WriteFile(const CChain &oChain, const std::string &sFilePath)
{
	std::string sJson = ToJson(oChain);
	FILE *pFile = fopen(sFilePath.c_str(),"wb");
	if(!pFile)
		MY_THROW(CGeneratorException,"Cannot open file to write: "+sFilePath+"\n"+strerror(errno));
	size_t nWritten = fwrite(sJson.data(),1,sJson.size(),pFile);
	int nClose = fclose(pFile);
	if(nWritten!=sJson.size() || nClose!=0)
		MY_THROW(CGeneratorException,"Error while writing file: "+sFilePath);
}

} // namespace kin5ax
