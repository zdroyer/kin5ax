#pragma once

#include "r3point.h"
#include "r3vector.h"
#include "r3caughtvector.h"

#include "r3box.h"
#include "transformation.h"
#include "rotation.h"
#include "translation.h"

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

class CMillingTool
{
public:
	MILLING_TOOL_TYPE m_type;

	double m_dRz,m_dRzSquare;
	double m_dR,m_dRSquare;
	double m_dReferencePointZ;



	double m_dCuttingHeight;
	
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
	double m_dSinusAxisOZAngle;

	CR3Box AxialCylinderBoundingBox(double dRadius,double dBegin, double dEnd) const;


	virtual bool IsInOZCuttingSpace(const CR3Point &p) const = 0;
	void MakeMinMax(double &d1,double &d2) const;
public:
	string GetCollisionLog() const;
	void ClearCollisionLog();
	void AddToCollisionLog(const string &s);
	virtual bool CauseCollision(const CR3Box &box, const CTransformation trToBoxFrame, const CTransformation trToToolFrame) const = 0;

	virtual CR3Box GetCuttingPartBoundingBox() const =0 ;

	CUT_TYPE IsCut(const CR3Point &r3pP) const;


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
