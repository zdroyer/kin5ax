#pragma once

#include "MillingTool.h"
#include "ChainSegment.h"
#include "CollisionReport.h"
#include "ChainConfiguration.h"
#include "R3AnchoredVector.h"
#include "R3Box.h"
#include "../TrajectoryStart.h"


//name of the root node of the XML (.kc) representation
#define CHAIN_XML_ROOT_NODE_TYPE_NAME "kinematic_chain"

namespace kin5ax {

//Which of the two branches of the chain. Both are stored from the base outwards: index 0 is the segment
//sitting on the machine base, the last segment is the terminal of the branch.
//The unit every length in a chain file is in. A file names it, and a file that does not is refused:
//the models shipped with the library are metres, FreeCAD works in millimetres, and 0.5 in a file is
//not a length until the file says which. Nothing inside the library converts - a chain's numbers
//stay in its own unit - but a host can ask, and scale at its boundary knowingly instead of guessing.
enum CHAIN_UNIT{
	CHAIN_UNIT_MM = 0,
	CHAIN_UNIT_M,
	CHAIN_UNIT_IN,
	CHAIN_UNIT_COUNT
};
//"mm", "m", "in": the spelling the files use
const char *ChainUnitName(CHAIN_UNIT unit);
//the unit of that spelling, exactly; false when it is none of the three
bool ParseChainUnit(const std::string &sName, CHAIN_UNIT &unit);
//how many millimetres one of that unit is: 1, 1000, 25.4
double ChainUnitInMillimetres(CHAIN_UNIT unit);

enum CHAIN_PART{
	CHAIN_PART_TOOL,			//ends with the TOOL segment, whose origin is the tool mount
	CHAIN_PART_MATERIAL_HOLDER	//ends with the MATERIAL_HOLDER segment, whose origin is the material frame
};

//One segment of the chain at the actual parameter values, in the frame of the machine base. The walk the
//solver and DetectCollisions do inside themselves, available to whoever draws the machine, places
//something on a segment or shows where a joint axis points (CChain::GetActualFrames).
struct CSegmentFrame
{
	//after the constant transformation of the segment, before its joint motion: the frame the collision
	//boxes of the segment are given in (CChainSegment::GetPartVector), and the frame whose Z is the axis
	//the joint turns about or slides along
	CTransformation m_trJoint;
	//after the joint motion as well: the input frame of the next segment. For the last segment of the
	//tool part this is the tool mount - the tool point hangs GetToolMountToToolPointDistance below it
	//along its Z - and for the last of the material holder part it is the frame of the material.
	CTransformation m_trOutput;
};

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

	//---- the tool mount: the origin of the TOOL terminal, where a tool holder is clamped.
	//
	//The terminal of the tool branch is the mount, in every chain, and the chain knows nothing about what
	//is clamped in it: a machine takes a short drill and a long slender cutter, and the end of its tool
	//branch is the same place for both. SetToolAssembly puts a tool in the spindle, and the tool point - the
	//programmed point of the CL data - then lies GetToolMountToToolPointDistance() below the terminal along
	//its Z. The forward kinematics, the position solver and the collision test of the tool all use that
	//distance. It is zero until a tool is given, which is the bare machine.
	//
	//Until 2026-10-15 this depended on whether the file declared a tool holder: without one the terminal was
	//read as the tool point itself, the way the files were written before holders existed (KnownBugs #68).
	//That made the same element of the same format mean two different places, and the hosts disagreed about
	//which - the chain editor hung the tool from the terminal, the FreeCAD addon hung a holder and a tool
	//below it. There is one meaning now.
	double GetToolMountToToolPointDistance() const;
	void SetToolMountToToolPointDistance(double dDistance);
	//from the frame of the tool mount to the frame of the tool point, and back
	CTransformation GetToolMountToToolPointTransformation() const;
	CTransformation GetToolPointToToolMountTransformation() const;
	//whether the file suggests a holder for this machine - a default for a host to offer, nothing more
	bool HasDefaultToolHolder() const;
	//the tool holder the machine usually carries, from the chain file
	const CToolHolder & GetDefaultToolHolder() const;
	void SetDefaultToolHolder(const CToolHolder &oHolder);
	//Takes the distance from the assembly of this tool (holder, shank, cutting part, zero point). Called by
	//Update and by CCLDataPostprocessor::Process; call it yourself after changing the tool or the holder of
	//a chain you solve directly.
	void SetToolAssembly(const CMillingTool &oTool);

	//---- the workpiece holder: the vice, the chuck, the plate the part is bolted to.
	//
	//The terminal of the workpiece branch is where the holder is mounted, in every chain, and the chain knows
	//nothing about what is bolted to it. The holder is a frame and not a solid: the transformation from the
	//frame a cutter location is given in - the frame of the part being cut - to that terminal. It is the
	//identity until one is given, which is the part sitting on the table with nothing in between, and it is
	//the mirror image of the tool mount: the one measurement that says where the work is, as the gauge length
	//says where the tool is.
	//
	//Everything the library answers about a location is in the frame of the part: the solver takes a cutter
	//location there, and GetActualToolPositionAndOrientation answers there. The frames of the segments are
	//the machine's own and carry no holder, so a host draws the part by this transformation.
	void SetWorkpieceHolder(const CTransformation &trHolder);
	const CTransformation & GetWorkpieceHolder() const;
	const CTransformation & GetWorkpieceHolderInverted() const;
	//true when a holder was given that is not the identity: a part that sits somewhere else than on the table
	bool HasWorkpieceHolder() const;
	//Collision test of every box of every segment against the boxes of every other segment, and of the whole tool
	//assembly (the cutting part, the shank and the holder) against the boxes of all segments, at the actual parameters. The result is stored in the segments (IsPartInCollision)
	//and in the collision log of the tool; the tool part and the material holder part are both walked from the base.
	void DetectCollisions(CMillingTool &oMillingTool);
	//Which solids met in the last DetectCollisions - and so in the last HasCollision, and in the last
	//check the solver made on this chain. Empty when nothing met.
	const CCollisionReport &GetCollisions() const;
	//The name a report gives one solid of a segment of a branch: the name of the segment, or its place
	//in the branch when the file gives it none, and the number of the box when the segment has more
	//than one. The host that draws a machine needs the same names to show what met.
	std::string GetSolidName(CHAIN_PART part, size_t nSegment, size_t nBox) const;
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
	CR3AnchoredVector GetActualToolPositionAndOrientation() const;

	void GetTranslationAxesAndOffset(
		CR3Vector &r3vT1,
		CR3Vector &r3vT2,
		CR3Vector &r3vT3,
		CR3Point &r3pP1, 
		const CChainConfiguration & oConfig) const;

	void GetTranslationsIndexes(size_t &nFirst, size_t &nSecond, size_t &nThird) const;
	void GetRotationsIndexes(size_t &nFirst, size_t &nSecond) const;
	//The parameter of a CChainConfiguration holding the first rotary axis met walking the chain from the
	//terminal of that part: from the tool end the axis nearest the tool (the head axis of a head/table
	//machine), from the material end the one nearest the part (its table axis). Throws when the chain is
	//not a 5C chain, because then there is no such thing.
	size_t GetFirstRotaryParameterIndex(CHAIN_PART nCountFrom) const;
	//The policy CSimulationData::FindShortestPath starts a trajectory with, resolved against this chain:
	//by default the first rotary axis counted from the tool end, closest to zero, which is what the
	//TWP/TCP functions of the usual controls do. pHint, when given, has to outlive the policy.
	CTrajectoryStart GetTrajectoryStart(CHAIN_PART nCountFrom = CHAIN_PART_TOOL
		,TRAJECTORY_START_CRITERION nCriterion = TRAJECTORY_START_MIN_ABS
		,const CChainConfiguration *pHint = NULL) const;
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
	CCollisionReport m_oCollisions;	//what met in the last DetectCollisions

	//descriptive data of the chain, all free text and all optional; kept in the file, not used by the solver
	std::string m_sName;		//name of the model, e.g. "BC tilt-rotary table"
	std::string m_sMachine;		//brand and model of the real machine, e.g. "SHW UniSpeed 3500"
	std::string m_sAuthor;
	std::string m_sModified;	//last modification, ISO 8601 (e.g. "2026-09-22" or "2026-09-22T14:30:00Z")
	std::string m_sFileName;
	//the unit of every length in this chain: required in a file, metres for a chain made in memory
	CHAIN_UNIT m_nUnit;

	int m_nActiveSegmentIndex;
	CHAIN_PART m_nActivePart;

	double m_dToolMountToToolPoint;		//see GetToolMountToToolPointDistance
	//see SetWorkpieceHolder; the inverse is kept because every walk of the chain needs that one
	CTransformation m_trWorkpieceHolder;
	CTransformation m_trWorkpieceHolderInverted;
	bool m_bWorkpieceHolder;

	void ComputeAndSetError(CChainConfiguration &oConfig, const CR3AnchoredVector &cv);
	CR3AnchoredVector m_toolPosition;

	size_t GetNumberOfRevoluteLinks() const;
	size_t GetNumberOfPrismaticLinks() const;
	bool Is5CChain() const;

	// adds segment to specified part of chain
	void AddSegmentAfterActiveSegment(const CChainSegment & oSegment);
	void DeleteActiveSegment();
	CChainSegment * GetActiveSegment();
	void ZeroAllParameters();
	std::vector<CChainSegment> & GetActivePart();
	void SetActiveSegment(CHAIN_PART part, int nActiveSegmentIndex);
	//the segments of one branch, in the order they are stored (base first)
	std::vector<CChainSegment> & GetPart(CHAIN_PART part);
	const std::vector<CChainSegment> & GetPart(CHAIN_PART part) const;
	//One frame per segment of the part, in the order the segments are stored, at the actual parameter
	//values. GetActualFrames(CHAIN_PART_TOOL).back().m_trOutput is the tool mount, and
	//GetActualFrames(CHAIN_PART_MATERIAL_HOLDER).back().m_trOutput the frame of the material.
	std::vector<CSegmentFrame> GetActualFrames(CHAIN_PART part) const;
	//the frame of one segment; throws CArgsMyException when the index is past the end of the part
	CSegmentFrame GetActualFrame(CHAIN_PART part, size_t nIndex) const;
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
	void SetUnit(CHAIN_UNIT unit);
	CHAIN_UNIT GetUnit() const;
	//ChainUnitInMillimetres of the unit: what a host multiplies by to get millimetres
	double GetUnitInMillimetres() const;
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
