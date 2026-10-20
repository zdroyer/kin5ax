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
	CHAIN_PART_SPINDLE,			//ends with the TOOL_SPINDLE_NOSE segment, whose origin is the nose of the spindle
	CHAIN_PART_TABLE	//ends with the TABLE segment, whose origin is the frame of the material
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
	//tool part this is the nose of the spindle - the programmed point hangs GetToolOffset below it
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

	/**
	 * @name The tool offset
	 *
	 * The terminal of the tool branch is the nose of the tool spindle, in every chain. The point a
	 * cutter location programs lies the TOOL OFFSET below it, along the Z of the nose: a positive
	 * distance, measured from the nose down to the reference point of whatever is in the spindle -
	 * the tip of the tool, the centre of its ball, or a height somebody gave. That measurement
	 * belongs to the tool and the holder, not to the machine, so the chain is told it at run time
	 * and never guesses: a chain that was not told refuses to solve (CChain::IsToolOffsetGiven).
	 *
	 * Until 2026-10-20 the distance stayed zero for a file that named no holder, whose terminal was
	 * then read as the tool point itself. The same element meant two different places and the hosts
	 * disagreed about which (KnownBugs #68).
	 * @{
	 */
	double GetToolOffset() const;
	void SetToolOffset(double dOffset);
	/// True once somebody has said how far the tool reaches below the nose.
	bool IsToolOffsetGiven() const;
	/// Takes the offset from a tool and its holder: what the assembly adds up to, less the reference point.
	void SetToolOffsetFromTool(const CMillingTool &oTool);
	/// From the frame of the spindle nose to the frame of the programmed point, and back.
	CTransformation GetSpindleNoseToToolPointTransformation() const;
	CTransformation GetToolPointToSpindleNoseTransformation() const;
	/// @}
	/// Whether the file names a tool holder for the spindle: a default to offer, nothing more.
	bool HasDefaultToolHolder() const;
	//the tool holder of the machine, from the chain file: what hangs from the nose of the spindle
	const CToolHolder & GetDefaultToolHolder() const;
	void SetDefaultToolHolder(const CToolHolder &oHolder);
	/**
	 * The holder and the tool the file describes, as one assembly: what a host would put in the
	 * spindle unless told otherwise, and what a postprocessor call takes like any other assembly.
	 * An empty assembly when the file describes neither.
	 */
	CToolAssembly GetDefaultToolAssembly() const;
	/// Whether the file names a workpiece holder for the table: a default to offer, nothing more.
	bool HasDefaultWorkpieceHolder() const;
	/// The workpiece holder of the table, as the file describes it.
	const CWorkpieceHolder & GetDefaultWorkpieceHolder() const;
	void SetDefaultWorkpieceHolder(const CWorkpieceHolder &oHolder);
	/// Whether the file describes a tool for this machine: a default to offer, nothing more.
	bool HasDefaultTool() const;
	/// The tool of the spindle nose, as the file describes it.
	const CToolDescription & GetDefaultTool() const;
	void SetDefaultTool(const CToolDescription &oTool);

	//Collision test of every box of every segment against the boxes of every other segment, and of the whole tool
	//assembly (the cutting part, the shank and the holder) against the boxes of all segments, at the actual parameters. The result is stored in the segments (IsPartInCollision)
	//and in the collision log of the tool; the tool part and the material holder part are both walked from the base.
	void DetectCollisions(CMillingTool &oMillingTool);
	/// The same with nothing in the spindle: the machine against itself, and no tool to test.
	void DetectCollisions(CMillingTool *pMillingTool);
	//Which solids met in the last DetectCollisions - and so in the last HasCollision, and in the last
	//check the solver made on this chain. Empty when nothing met.
	const CCollisionReport &GetCollisions() const;
	//The name a report gives one solid of a segment of a branch: the name of the segment, or its place
	//in the branch when the file gives it none, and the number of the box when the segment has more
	//than one. The host that draws a machine needs the same names to show what met.
	std::string GetSolidName(CHAIN_PART part, size_t nSegment, size_t nBox) const;
	CChain(void);
	//the tool part ends with the only TOOL_SPINDLE_NOSE segment, the material holder part with
	//the only TABLE segment, no part is empty, no limit is above its maximum: else CGeneratorException
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
	CTrajectoryStart GetTrajectoryStart(CHAIN_PART nCountFrom = CHAIN_PART_SPINDLE
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

	double m_dToolOffset;				//see GetToolOffset
	bool m_bToolOffsetGiven;

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
	//values. GetActualFrames(CHAIN_PART_SPINDLE).back().m_trOutput is the nose of the spindle, and
	//GetActualFrames(CHAIN_PART_TABLE).back().m_trOutput the frame of the material.
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
	/// The same with nothing in the spindle.
	bool HasCollision(CMillingTool *pMillingTool,std::string &sLog);



};

} // namespace kin5ax
