#include "../stdafx.h"
#include "ChainXmlReader.h"
#include "string_tools.h"
#include "../3rdParty/pugixml/pugixml.hpp"
#include <charconv>

//The portable reader of .kc files: pugixml, no DTD validation. Everything the DTD used to
//check, and what it could not, is checked here: the elements and attributes of every node,
//the numbers, both limits given together, and (CChain::CheckStructure) the terminals.
//Every message names the file and the line.
namespace kin5ax {

namespace {

struct CFileContext
{
	std::string m_sFilePath;
	std::string m_sData;

	size_t LineOf(ptrdiff_t nOffset) const
	{
		if(nOffset<0)
			return 0;
		size_t nEnd = std::min((size_t)nOffset,m_sData.size());
		return 1+(size_t)std::count(m_sData.begin(),m_sData.begin()+nEnd,'\n');
	}
	[[noreturn]] void Fail(const pugi::xml_node &node, const std::string &sMessage) const
	{
		MY_THROW(CGeneratorException,m_sFilePath+", line "+size_to_string(LineOf(node.offset_debug()))+": "+sMessage);
	}
};

typedef std::initializer_list<const char*> tNames;

bool Contains(const tNames &names, const char *szName)
{
	for(const char *szItem : names)
		if(strcmp(szItem,szName)==0)
			return true;
	return false;
}

std::string Tag(const pugi::xml_node &node)
{
	return std::string("<")+node.name()+">";
}

//the required attributes are present, no others
void CheckAttributes(const CFileContext &ctx, const pugi::xml_node &node, const tNames &required, const tNames &optional)
{
	for(const char *szName : required)
		if(!node.attribute(szName))
			ctx.Fail(node,Tag(node)+" has no attribute '"+szName+"'");
	for(pugi::xml_attribute a : node.attributes())
		if(!Contains(required,a.name()) && !Contains(optional,a.name()))
			ctx.Fail(node,Tag(node)+" has an unknown attribute '"+a.name()+"'");
}

//only the allowed child elements (comments and text are ignored)
void CheckChildren(const CFileContext &ctx, const pugi::xml_node &node, const tNames &allowed)
{
	for(pugi::xml_node child : node.children())
		if(child.type()==pugi::node_element && !Contains(allowed,child.name()))
			ctx.Fail(child,Tag(node)+" may not contain "+Tag(child));
}

pugi::xml_node SingleChild(const CFileContext &ctx, const pugi::xml_node &node, const char *szName)
{
	pugi::xml_node result;
	for(pugi::xml_node child : node.children(szName)){
		if(result)
			ctx.Fail(child,Tag(node)+" has more than one <"+szName+"> element");
		result = child;
	}
	if(!result)
		ctx.Fail(node,Tag(node)+" has no <"+szName+"> element");
	return result;
}

//a number in the C locale, nothing but blanks around it
double Number(const CFileContext &ctx, const pugi::xml_node &node, const char *szAttr)
{
	std::string s = node.attribute(szAttr).value();
	size_t nBegin = s.find_first_not_of(" \t\r\n");
	size_t nEnd = s.find_last_not_of(" \t\r\n");
	double d = 0.0;
	if(nBegin!=std::string::npos){
		const char *p = s.c_str()+nBegin, *pEnd = s.c_str()+nEnd+1;
		std::from_chars_result r = std::from_chars(p,pEnd,d);
		if(r.ec==std::errc() && r.ptr==pEnd)
			return d;
	}
	ctx.Fail(node,"attribute '"+std::string(szAttr)+"' of "+Tag(node)+" is not a number: '"+s+"'");
}

bool OptionalNumber(const CFileContext &ctx, const pugi::xml_node &node, const char *szAttr, double &d)
{
	if(!node.attribute(szAttr))
		return false;
	d = Number(ctx,node,szAttr);
	return true;
}

CChainSegment ReadSegment(const CFileContext &ctx, const pugi::xml_node &node)
{
	CheckAttributes(ctx,node,{"type","name"}
		,{"parameter_value_offset","default_parameter_value","parameter_value_minimum","parameter_value_maximum"});
	std::string sType = node.attribute("type").value();
	if(!Contains({"REVOLUTE_LINK","PRISMATIC_LINK","TOOL","MATERIAL_HOLDER"},sType.c_str()))
		ctx.Fail(node,"unknown segment type '"+sType+"'");
	std::string sName = node.attribute("name").value();

	//the type and the constant transformation are set below
	CChainSegment oSegment(CHAIN_SEGMENT_TOOL,CR3Vector(1.0,0.0,0.0),0.0,CR3Vector(0.0));
	oSegment.SetName(sName);
	oSegment.SetType(sType);
	oSegment.m_dActualParameterValue = 0.0;

	//written by the editor for the DTD, not used by the model: checked to be numbers only
	double dUnused;
	OptionalNumber(ctx,node,"parameter_value_offset",dUnused);
	OptionalNumber(ctx,node,"default_parameter_value",dUnused);

	double dMin = 0.0, dMax = 0.0;
	bool bMin = OptionalNumber(ctx,node,"parameter_value_minimum",dMin);
	bool bMax = OptionalNumber(ctx,node,"parameter_value_maximum",dMax);
	if(bMin!=bMax)
		ctx.Fail(node,"segment '"+sName+"': parameter_value_minimum and parameter_value_maximum must be given together");
	oSegment.m_bParameterIsConstrained = bMin;
	if(bMin){
		if(dMin>dMax)
			ctx.Fail(node,"segment '"+sName+"': parameter_value_minimum is above parameter_value_maximum");
		if(oSegment.m_type==CHAIN_SEGMENT_REVOLUTE_LINK){
			dMin = degrees_to_radians(dMin);
			dMax = degrees_to_radians(dMax);
		}
		oSegment.m_dParameterValueMinimum = dMin;
		oSegment.m_dParameterValueMaximum = dMax;
	}

	CheckChildren(ctx,node,{"initial_translation","initial_rotation","box"});
	pugi::xml_node t = SingleChild(ctx,node,"initial_translation");
	CheckAttributes(ctx,t,{"x","y","z"},{});
	oSegment.m_r3vConstTranslation = CR3Vector(Number(ctx,t,"x"),Number(ctx,t,"y"),Number(ctx,t,"z"));

	pugi::xml_node r = SingleChild(ctx,node,"initial_rotation");
	CheckAttributes(ctx,r,{"x","y","z","alpha"},{});
	oSegment.m_r3vConstRevoluteAxis = CR3Vector(Number(ctx,r,"x"),Number(ctx,r,"y"),Number(ctx,r,"z"));
	oSegment.m_dConstRevoluteAngle = Number(ctx,r,"alpha");
	if(is_zero(oSegment.m_r3vConstRevoluteAxis.GetLengthSquare())){
		if(!is_zero(oSegment.m_dConstRevoluteAngle))
			ctx.Fail(r,"segment '"+sName+"': the rotation axis is the zero vector");
		oSegment.m_r3vConstRevoluteAxis = CR3Vector(1.0,0.0,0.0);	//no rotation: any axis, the same as the MSXML reader
	}

	std::vector<CR3Box> vParts;
	for(pugi::xml_node b : node.children("box")){
		CheckAttributes(ctx,b,{"x1","x2","y1","y2","z1","z2"},{});
		vParts.push_back(CR3Box(Number(ctx,b,"x1"),Number(ctx,b,"x2")
							   ,Number(ctx,b,"y1"),Number(ctx,b,"y2")
							   ,Number(ctx,b,"z1"),Number(ctx,b,"z2")));
	}
	oSegment.SetPartVector(vParts);
	return oSegment;
}

std::vector<CChainSegment> ReadPart(const CFileContext &ctx, const pugi::xml_node &root, const char *szPartName)
{
	pugi::xml_node part = SingleChild(ctx,root,szPartName);
	CheckAttributes(ctx,part,{},{});
	CheckChildren(ctx,part,{"segment"});
	std::vector<CChainSegment> vSegments;
	for(pugi::xml_node s : part.children("segment"))
		vSegments.push_back(ReadSegment(ctx,s));
	if(vSegments.empty())
		ctx.Fail(part,Tag(part)+" has no <segment> element");
	return vSegments;
}

} // namespace

CChain CChainXmlReaderPugixml::ReadFile(const std::string &sFilePath)
{
	CFileContext ctx;
	ctx.m_sFilePath = sFilePath;
	FILE *pFile = fopen(sFilePath.c_str(),"rb");
	if(!pFile)
		MY_THROW(CGeneratorException,"Cannot open file: "+sFilePath+"\n"+strerror(errno));
	char buffer[4096];
	size_t n;
	while((n = fread(buffer,1,sizeof(buffer),pFile))>0)
		ctx.m_sData.append(buffer,n);
	fclose(pFile);

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_buffer(ctx.m_sData.data(),ctx.m_sData.size());
	if(!result)
		MY_THROW(CGeneratorException,"XML error in "+sFilePath+", line "+size_to_string(ctx.LineOf(result.offset))+": "+result.description());
	pugi::xml_node root = doc.document_element();
	if(!root)
		MY_THROW(CGeneratorException,"XML error in "+sFilePath+": no root node");
	if(strcmp(root.name(),CHAIN_XML_ROOT_NODE_TYPE_NAME)!=0)
		ctx.Fail(root,"invalid root node "+Tag(root)+", expected <"+CHAIN_XML_ROOT_NODE_TYPE_NAME+">");
	CheckAttributes(ctx,root,{},{"name"});
	CheckChildren(ctx,root,{"tool_part","material_holder_part"});

	//the default segments of a new chain are replaced below
	CChain oChain;
	oChain.SetName(root.attribute("name").value());	//absent: empty
	oChain.m_vToolPart = ReadPart(ctx,root,"tool_part");
	oChain.m_vMaterialHolderPart = ReadPart(ctx,root,"material_holder_part");
	oChain.SetFileName(sFilePath);
	try{
		oChain.CheckStructure();
	} catch (CGeneratorException &e){
		MY_THROW(CGeneratorException,sFilePath+": "+e.Message());
	}
	oChain.SetParameters(CChainConfiguration((int)oChain.GetNumberOfRegularLinks()));
	return oChain;
}

} // namespace kin5ax
