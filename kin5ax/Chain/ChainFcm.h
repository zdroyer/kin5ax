#pragma once

#include "Chain.h"

//Imports and exports the machine definition of the FreeCAD CAM workbench (*.fcm).
//This header is free of any JSON library: nlohmann/json is included by ChainFcm.cpp alone.
//
//The two files describe the same machine in different ways:
//  .fcm   names every axis and gives its joint as an origin and a direction in the machine base
//         frame with all axes at zero; "role" puts the axis in the head (tool) or the table
//         (workpiece) branch, "parent" and "sequence" order it; the tool frame and the workpiece
//         frame both coincide with the base frame at the home position.
//  kin5ax gives every segment a constant transformation relative to the previous one and moves
//         along or about the local OZ, and closes each branch with a terminal segment.
//
//Neither file can hold everything the other one has. Nothing is dropped silently: what could not
//be carried across is collected in a CFcmReport. See Readme.md, section "The FreeCAD bridge",
//for the whole list and for what to do about each item.
namespace kin5ax {

//Conventions that cannot be read from the file and have to be checked against the real machine.
struct CFcmOptions
{
	CFcmOptions();
	//The value of an axis in the file is taken as the value of the joint parameter of the model.
	//Set these when the machine counts a table axis the other way round.
	bool m_bNegateTableLinearAxes;
	bool m_bNegateTableRotaryAxes;
	//import: the name of the chain when not empty; export: machine.name when not empty
	std::string m_sMachineName;
	//export: an .fcm whose toolheads, post-processor and other sections are kept as they are;
	//empty writes a minimal file
	std::string m_sTemplateFilePath;
};

//What a conversion could not carry across; empty means both files describe the same machine.
//A reader or a writer empties it first: it always describes the conversion that filled it.
class CFcmReport
{
public:
	std::vector<std::string> m_vRemarks;
	void Clear();
	void Add(const std::string &sRemark);
	bool IsExact() const;
	std::string ToString(const std::string &sSeparator = "\n") const;
};

class CChainFcmReader
{
public:
	//throws CGeneratorException naming the file and the axis at fault
	static CChain ReadFile(const std::string &sFilePath, const CFcmOptions &oOptions, CFcmReport &oReport);
	static CChain Parse(const std::string &sFcm, const std::string &sWhere,
						const CFcmOptions &oOptions, CFcmReport &oReport);
};

class CChainFcmWriter
{
public:
	static std::string ToFcm(const CChain &oChain, const CFcmOptions &oOptions, CFcmReport &oReport);
	static void WriteFile(const CChain &oChain, const std::string &sFilePath,
						  const CFcmOptions &oOptions, CFcmReport &oReport);
};

} // namespace kin5ax
