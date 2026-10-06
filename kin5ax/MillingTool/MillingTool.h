#pragma once

#include "R3Point.h"
#include "R3Vector.h"
#include "R3AnchoredVector.h"

#include "R3Box.h"
#include "Transformation.h"
#include "Rotation.h"
#include "Translation.h"
#include "ToolHolder.h"

namespace kin5ax {

enum MILLING_TOOL_TYPE{
	MILLING_TOOL_BALL,
	MILLING_TOOL_BARREL,
	MILLING_TOOL_CONE,
	MILLING_TOOL_CYLINDER,
	MILLING_TOOL_TORUS
};

typedef unsigned int CUT_TYPE;
#define CUT_TYPE_NOT_CUT 0x00
#define CUT_TYPE_CUT 0x01
#define CUT_TYPE_IN_SIDE_NOT_CUTTING_PART 0x02
#define CUT_TYPE_IN_AXIAL_NOT_CUTTING_PART 0x04


inline int is_point_not_cut(CUT_TYPE type)
{ return (type == 0x00);}

inline int is_point_cut(CUT_TYPE type)
{ return (type & 0x01);}

inline int is_point_in_SNCP(CUT_TYPE type)
{ return (type & 0x02);}

inline int is_point_in_ANCP(CUT_TYPE type)
{ return (type & 0x04);}

//A milling tool with its holder: the geometry of the three solids of the assembly and its current position
//and orientation. Measured along the axis from the tip, in the tip frame of IsInOZCuttingSpace:
//  the cutting part    0 .. m_dCuttingHeight                     cuts material, must not meet a machine part
//  the shank           m_dCuttingHeight .. GetToolLength()       must meet neither material nor a machine part
//  the holder          GetToolLength() .. GetAssemblyLength()    the same, radius of CToolHolder
//The tool is placed by its reference point (the programmed point of the CL data), which lies on the axis
//m_dReferencePointZ above the tip; DefaultZeroPointZ is where that point lies for the shape of the tool.
//The whole assembly hangs GetMountToToolPointDistance() below the tool mount of the machine, which is the
//origin of the TOOL terminal of the kinematic chain.
class CMillingTool
{
public:
	MILLING_TOOL_TYPE m_type;

	double m_dRz,m_dRzSquare;	//radius of the cylindrical part (the shank) and its square
	double m_dR,m_dRSquare;		//radius of the ball (ball nose tool) and its square
	double m_dReferencePointZ;	//height of the reference point above the tip, along the axis



	double m_dCuttingHeight;	//length of the cutting part from the tip, along the axis
	double m_dNonCuttingLength;	//the shank between the cutting part and the holder
	CToolHolder m_oHolder;		//between the tool and the tool mount of the machine
	
/*
	double m_dSideNotCuttingBegin;
	double m_dSideNotCuttingEnd;
	double m_dSideNotCuttingRadius;
	double m_dSideNotCuttingRadiusSquare;
	
	double m_dAxialNotCuttingBegin;
	double m_dAxialNotCuttingEnd;
	double m_dAxialNotCuttingRadius;
	double m_dAxialNotCuttingRadiusSquare;
//*/
protected:
	CR3Point m_r3pPosition;
	CR3Vector m_r3vOrientation;
	CTransformation m_m4x4ToOZ;

	CR3Point m_r3pBoundingBoxLLN,m_r3pBoundingBoxRTF;
	std::string m_sCollisionLog;

	//The bounding box (in the world frame) of a cylinder of radius dRadius about the tool axis, from dBegin to dEnd
	//measured along the axis from the reference point.
	CR3Box AxialCylinderBoundingBox(double dRadius,double dBegin, double dEnd) const;
	//Collision of a cylinder of radius dRadius about the tool axis, from dBeginZ to dBeginZ+dLength measured
	//along the axis from the tip, with a box; the frames are those of CauseCollision. False for an empty cylinder.
	bool CylinderMeetsBox(const CR3Box &box, double dBeginZ, double dLength, double dRadius
		,const CTransformation &trToBoxFrame, const CTransformation &trToToolFrame) const;


	//Throws CArgsMyException when DimensionProblem has something to say about this tool. Called by the
	//constructor of each shape, which is where m_type is known.
	void CheckDimensions() const;

	//The cut volume of the tool in the tool frame of IsCut: the tip at the origin, the axis along +OZ.
	//Returns true when p is inside the cutting part of the tool.
	virtual bool IsInOZCuttingSpace(const CR3Point &p) const = 0;
	void MakeMinMax(double &d1,double &d2) const;
public:
	//Why these dimensions do not make a tool of that shape, or an empty string when they do: the dimensions
	//are no longer made positive behind the caller's back, they are refused (KnownBugs #50). A user interface
	//that builds a tool out of fields being typed into asks this before it constructs one.
	static std::string DimensionProblem(MILLING_TOOL_TYPE type, double dRz, double dR, double dCuttingHeight
		,double dReferencePointZ, double dNonCuttingLength);

	std::string GetCollisionLog() const;
	void ClearCollisionLog();
	void AddToCollisionLog(const std::string &s);
	//Collision test with a box of a machine part. box - in its own frame;
	//trToBoxFrame - from the tool frame (the reference point at the origin, the axis along +OZ) to the frame of box;
	//trToToolFrame - the inverse. Returns true when the tool (its whole length m_dCuttingHeight from the tip) meets the box.
	virtual bool CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const = 0;

	//The bounding box of the cutting part in the world frame, at the current position and orientation (for the rendering).
	virtual CR3Box GetCuttingPartBoundingBox() const =0 ;

	//Cut test of the material point r3pP (world frame) at the current position and orientation (the voxel simulation).
	//Returns CUT_TYPE_CUT when the point is inside the cutting part, CUT_TYPE_NOT_CUT otherwise.
	CUT_TYPE IsCut(const CR3Point &r3pP) const;

	//the tool without its holder: the cutting part and the shank
	double GetToolLength() const;
	//from the tip to the tool mount: the tool and its holder
	double GetAssemblyLength() const;
	//From the tool mount down to the reference point, along the axis: the assembly length less the height of the
	//reference point above the tip. This is what CChain puts between the TOOL terminal and the tool point.
	double GetMountToToolPointDistance() const;
	//Where the programmed point of this shape of tool lies above the tip: the tip itself for a flat end mill,
	//the centre of the ball for a ball nose tool. The caller decides whether to use it (m_dReferencePointZ).
	virtual double DefaultZeroPointZ() const = 0;

	//true when the assembly has a shank or a holder with a body
	bool HasNonCuttingPart() const;
	//The non-cutting envelope (the shank and the holder) in the tip frame: the tip at the origin, the axis
	//along +OZ. Nothing of it may cut material or meet a machine part.
	bool IsInOZNonCuttingSpace(const CR3Point &p) const;
	//the same test for a point of the world frame, at the current position and orientation
	bool IsInNonCuttingPart(const CR3Point &r3pP) const;
	//the bounding box of the non-cutting envelope in the world frame; the default (empty) box when there is none
	CR3Box GetNonCuttingPartBoundingBox() const;
	//Collision of the non-cutting envelope with a box of a machine part; the frames are those of CauseCollision.
	bool CauseNonCuttingCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const;


	//Places the tool: r3pNewPosition is the new place of the reference point (m_dReferencePointZ above the tip),
	//r3vNewOrientation the new direction of the axis from the tip towards the spindle.
	void SetPositionAndOrientation(const CR3Point &r3pNewPosition, const CR3Vector &r3vNewOrientation);
	void SetPositionAndOrientation(const CR3AnchoredVector &cv);




	CMillingTool
		(double dRz
		,double dR
		,double dCuttingHeight
		,double dReferencePointZ
		,double dNonCuttingLength = 0.0
		,const CToolHolder &oHolder = CToolHolder());

//	virtual void GetBoundingBox(CR3Point &r3pP1, CR3Point &r3pP2) const = 0;
	//the tools are owned and deleted through CMillingTool pointers (the document, the tool dialog)
	virtual ~CMillingTool(void);
};

} // namespace kin5ax
