#pragma once

#include "R3Point.h"
#include "R3Vector.h"
#include "R3CaughtVector.h"

#include "R3Box.h"
#include "Transformation.h"
#include "Rotation.h"
#include "Translation.h"

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

//A milling tool: the geometry of its cutting part (the cut volume of the voxel simulation and the collision
//volume against the machine parts) and its current position and orientation. The tool is placed by its
//reference point, which lies on the axis m_dReferencePointZ above the tip.
class CMillingTool
{
public:
	MILLING_TOOL_TYPE m_type;

	double m_dRz,m_dRzSquare;	//radius of the cylindrical part (the shank) and its square
	double m_dR,m_dRSquare;		//radius of the ball (ball nose tool) and its square
	double m_dReferencePointZ;	//height of the reference point above the tip, along the axis



	double m_dCuttingHeight;	//length of the tool from the tip, along the axis
	
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
	CR3Vector m_r3vMoveVector;
	CTransformation m_m4x4ToOZ;

	const CR3Vector &GetMoveVector() const;
	CR3Point m_r3pBoundingBoxLLN,m_r3pBoundingBoxRTF;
	string m_sCollisionLog;
	//in radians
	double m_dAxisOZAngle;
	double m_dSineAxisOZAngle;

	//The bounding box (in the world frame) of a cylinder of radius dRadius about the tool axis, from dBegin to dEnd
	//measured along the axis from the reference point.
	CR3Box AxialCylinderBoundingBox(double dRadius,double dBegin, double dEnd) const;


	//The cut volume of the tool in the tool frame of IsCut: the tip at the origin, the axis along +OZ.
	//Returns true when p is inside the cutting part of the tool.
	virtual bool IsInOZCuttingSpace(const CR3Point &p) const = 0;
	void MakeMinMax(double &d1,double &d2) const;
public:
	string GetCollisionLog() const;
	void ClearCollisionLog();
	void AddToCollisionLog(const string &s);
	//Collision test with a box of a machine part. box - in its own frame;
	//trToBoxFrame - from the tool frame (the reference point at the origin, the axis along +OZ) to the frame of box;
	//trToToolFrame - the inverse. Returns true when the tool (its whole length m_dCuttingHeight from the tip) meets the box.
	virtual bool CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const = 0;

	//The bounding box of the cutting part in the world frame, at the current position and orientation (for the rendering).
	virtual CR3Box GetCuttingPartBoundingBox() const =0 ;

	//Cut test of the material point r3pP (world frame) at the current position and orientation (the voxel simulation).
	//Returns CUT_TYPE_CUT when the point is inside the cutting part, CUT_TYPE_NOT_CUT otherwise.
	CUT_TYPE IsCut(const CR3Point &r3pP) const;


	//Places the tool: r3pNewPosition is the new place of the reference point (m_dReferencePointZ above the tip),
	//r3vNewOrientation the new direction of the axis from the tip towards the spindle.
	void SetPositionAndOrientation(const CR3Point &r3pNewPosition, const CR3Vector &r3vNewOrientation);
	void SetPositionAndOrientation(const CR3CaughtVector &cv);




	CMillingTool
		(double dRz
		,double dR
		,double dCuttingHeight
		,double dReferencePointZ);

//	virtual void GetBoundingBox(CR3Point &r3pP1, CR3Point &r3pP2) const = 0;
	~CMillingTool(void);
};
