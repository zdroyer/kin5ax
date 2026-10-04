#pragma once

#include "MillingTool.h"
#include "ChainSegment.h"
#include "ChainConfiguration.h"
#include "R3CaughtVector.h"
#include "R3Box.h"


//name of the root node of the XML (.kc) representation
#define CHAIN_XML_ROOT_NODE_TYPE_NAME "kinematic_chain"

namespace kin5ax {

enum CHAIN_CONSTRAINTS_RESULT{
	CHAIN_CONSTRAINTS_MET,
	CHAIN_CONSTRAINTS_SHIFTS_EXCEEDED,
	CHAIN_CONSTRAINTS_ROTATIONS_EXCEEDED,
	CHAIN_CONSTRAINTS_SHIFTS_AND_ROTATION_EXCEEDED
};

class CChain
{

	CChainConfiguration m_oActualParameters;
public:
	//the actual parameter of segment nIndex of the tool part / the material holder part (0.0 for the tool and the holder)
	double GetActualParameter(size_t nIndex, bool bToolPart) const;

	//---- the tool mount: the origin of the TOOL terminal, where the tool holder is clamped.
	//The tool point (the programmed point of the CL data) lies GetToolMountToToolPointDistance() below it,
	//along the Z axis of the terminal; the forward kinematics, the position solver and the collision test of
	//the tool all use that distance. It stays zero for a chain file that declares no tool holder, whose TOOL
	//terminal is the tool point itself, the way the files were written before the holder existed (KnownBugs #68).
	double GetToolMountToToolPointDistance() const;
	void SetToolMountToToolPointDistance(double dDistance);
	//from the frame of the tool mount to the frame of the tool point, and back
	CTransformation GetToolMountToToolPointTransformation() const;
	CTransformation GetToolPointToToolMountTransformation() const;
	//true when the chain file declares a tool mount (the tool_holder of its TOOL segment): only then does the
	//length of a tool assembly move the tool point away from the terminal
	bool IsToolMountDeclared() const;
	//the tool holder of the machine, from the chain file; declaring one declares the tool mount
	const CToolHolder & GetDefaultToolHolder() const;
	void SetDefaultToolHolder(const CToolHolder &oHolder);
	//Takes the distance from the assembly of this tool (holder, shank, cutting part, zero point), or leaves it
	//at zero when the chain declares no tool mount. Called by Update and by CCLDataPostprocessor::Process;
	//call it yourself after changing the tool or the holder of a chain you solve directly.
	void SetToolAssembly(const CMillingTool &oTool);
	//Collision test of every box of every segment against the boxes of every other segment, and of the tool against
	//the boxes of all segments, at the actual parameters. The result is stored in the segments (IsPartInCollision)
	//and in the collision log of the tool; the tool part and the material holder part are both walked from the base.
	void DetectCollisions(CMillingTool &oMillingTool);
	CChain(void);
	std::string toXML() const;
	//the tool part ends with the only TOOL segment, the material holder part with the only
	//MATERIAL_HOLDER segment, no part is empty, no limit is above its maximum: else CGeneratorException
	void CheckStructure() const;
	~CChain(void);
	CR3Box GetBoundingBox() const;
	CTransformation GetFirstConstantRotation(size_t &nFirstRotaryLinkIndex) const;
	CTransformation GetMiddleConstantRotation() const;
	CTransformation GetMiddleConstantInvertedRotation() const;
	CTransformation GetLastConstantRotation(size_t &nLastRotaryLinkIndex) const;
	CTransformation GetActualToMaterialInvertedTransformation() const;
	CTransformation GetActualToToolTransformation() const;
	CR3CaughtVector GetActualToolPositionAndOrientation() const;

	void GetTranslationAxesAndOffset(
		CR3Vector &r3vT1,
		CR3Vector &r3vT2,
		CR3Vector &r3vT3,
		CR3Point &r3pP1, 
		const CChainConfiguration & oConfig) const;

	void GetTranslationsIndexes(size_t &nFirst, size_t &nSecond, size_t &nThird) const;
	void GetRotationsIndexes(size_t &nFirst, size_t &nSecond) const;
	CChainConfiguration	GetParameters() const;
	void SetParameters(const CChainConfiguration & oConfig);

	CChainSegment & operator[](int nIndex);
	const CChainSegment & operator[](int nIndex) const;
	void SetVirtualToolPosition(const CR3Vector &r3v, const CR3Point &r3p);
	// part of chain ended with milling tool 
	std::vector<CChainSegment> m_vToolPart;
	size_t GetNumberOfRegularLinks() const; 
	//part of chain ended with material holder
	std::vector<CChainSegment> m_vMaterialHolderPart;

	//descriptive data of the chain, all free text and all optional; kept in the file, not used by the solver
	std::string m_sName;		//name of the model, e.g. "BC tilt-rotary table"
	std::string m_sMachine;		//brand and model of the real machine, e.g. "SHW UniSpeed 3500"
	std::string m_sAuthor;
	std::string m_sModified;	//last modification, ISO 8601 (e.g. "2026-09-22" or "2026-09-22T14:30:00Z")
	std::string m_sFileName;

	int m_nActiveSegmentIndex;
	bool m_bIsToolPartActive;

	double m_dToolMountToToolPoint;		//see GetToolMountToToolPointDistance
	bool m_bToolMountDeclared;
	CToolHolder m_oDefaultToolHolder;

	void ComputeAndSetError(CChainConfiguration &oConfig, const CR3CaughtVector &cv);
	CR3CaughtVector m_toolPosition;

	size_t GetNumberOfRevoluteLinks() const;
	size_t GetNumberOfPrismaticLinks() const;
	bool Is5CChain() const;

	// adds segment to specified part of chain
	void AddSegmentAfterActiveSegment(const CChainSegment & oSegment);
	void DeleteActiveSegment();
	CChainSegment * GetActiveSegment();
	void ZeroAllParameters();
	std::vector<CChainSegment> & GetActivePart();
	void SetActiveSegment(bool bIsToolPartActive, int mActiveSegmentIndex);
	void SetName(const std::string &sName);
	std::string GetName() const;
	void SetMachine(const std::string &sMachine);
	std::string GetMachine() const;
	void SetAuthor(const std::string &sAuthor);
	std::string GetAuthor() const;
	void SetModified(const std::string &sModified);
	std::string GetModified() const;
	void SetFileName(const std::string &sName);
	std::string GetFileName() const ;
	//resizes the parameters vector if needed, detects collisions and places the tool (to be called before rendering)
	void Update(CMillingTool *pTool);
	//gives every regular segment a non-empty name different from the names of the earlier segments
	void CheckAndCorrectSegmentsNames(void);
	
	bool HasParameterConstraints() const;
	CHAIN_CONSTRAINTS_RESULT MeetsParametersConstraints(const CChainConfiguration &oChainConfig,std::string &sExceededConstraints) const; 
	//DetectCollisions and a report: returns true when any part is in collision, then sLog names the parameters,
	//the segments and their boxes, and the boxes hit by the tool.
	bool HasCollision(CMillingTool &oMillingTool,std::string &sLog);



};

} // namespace kin5ax
