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
const int nVERSION = 1;

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

CChainSegment ReadTerminal(const CContext &ctx, const tJson &j, const std::string &sPath, CHAIN_SEGMENT_TYPE type)
{
	ctx.CheckObject(j,sPath,{"name","translation","rotation"},{"boxes"});
	CChainSegment oSegment(type,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0));
	oSegment.SetType(type);
	oSegment.m_bParameterIsConstrained = false;
	ReadCommon(ctx,j,sPath,oSegment);
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
	ctx.CheckObject(root,"",{"format","version","tool_branch","workpiece_branch"}
		,{"$schema","name","machine","author","modified"});

	std::string sFormat = ctx.String(ctx.Member(root,"format",""),"format");
	if(sFormat != szFORMAT)
		ctx.Fail("format","'"+sFormat+"' is not a kinematic chain, expected '"+szFORMAT+"'");
	const tJson &version = ctx.Member(root,"version","");
	if(!version.is_number_integer() || version.get<int>()!=nVERSION)
		ctx.Fail("version","unsupported version, expected "+size_to_string(nVERSION));

	//the default segments of a new chain are replaced below
	CChain oChain;
	oChain.SetName(ctx.OptionalString(root,"name",""));
	oChain.SetMachine(ctx.OptionalString(root,"machine",""));
	oChain.SetAuthor(ctx.OptionalString(root,"author",""));
	oChain.SetModified(ctx.OptionalString(root,"modified",""));
	oChain.m_vToolPart = ReadBranch(ctx,root,"tool_branch","tool",CHAIN_SEGMENT_TOOL);
	oChain.m_vMaterialHolderPart = ReadBranch(ctx,root,"workpiece_branch","workpiece_holder",CHAIN_SEGMENT_MATERIAL_HOLDER);
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
	if(!oChain.GetName().empty())
		root["name"] = oChain.GetName();
	if(!oChain.GetMachine().empty())
		root["machine"] = oChain.GetMachine();
	if(!oChain.GetAuthor().empty())
		root["author"] = oChain.GetAuthor();
	if(!oChain.GetModified().empty())
		root["modified"] = oChain.GetModified();
	root["tool_branch"] = BranchToJson(oChain.m_vToolPart,"tool");
	root["workpiece_branch"] = BranchToJson(oChain.m_vMaterialHolderPart,"workpiece_holder");
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
