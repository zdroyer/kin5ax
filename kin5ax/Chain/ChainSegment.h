#pragma once
#include "R3Vector.h"
#include "R3Point.h"
#include "R3Box.h"
#include "ToolHolder.h"
#include "ToolDescription.h"
#include "WorkpieceHolder.h"
#include "ToolAssembly.h"
#include "../3DGeometry/Solid.h"

#include "Rotation.h"

namespace kin5ax {

enum CHAIN_SEGMENT_TYPE{
	CHAIN_SEGMENT_REVOLUTE_LINK=1,
	CHAIN_SEGMENT_PRISMATIC_LINK=2,
	CHAIN_SEGMENT_TABLE=3,
	CHAIN_SEGMENT_TOOL_SPINDLE_NOSE=4,

};


class CChainSegment
{

protected:
	//the solids of the segment - boxes, pieces of cylinders, pieces of spheres - in the frame
	//after its constant transformation and before its joint motion
	std::vector<CSolid> m_vParts;
	std::vector<bool> m_vCollisions;	//one flag per solid, set by CChain::DetectCollisions
	int m_nHighlightedPart;

public:
	//Is any solid of the segment flagged? Then sLog names the segment and the solids.
	bool HasPartInCollision(std::string &sLog) const;
	void ClearAllPartCollision();

	//the collision flag of solid n (0-based)
	void SetPartCollision(size_t n);
	bool IsPartInCollision(size_t n) const;
	//A box is a solid, so a caller that has boxes can still hand them over one by one. The
	//vector itself is a vector of solids: see Solid.h for what one may be.
	void SetPartVector(std::vector<CSolid> vNewParts);
	const std::vector<CSolid> &GetPartVector()const;
	void HighlightPart(int n);
	int GetHighlightedPartIndex() const;
	//the highlighted solid, to be edited in place by a host; null when none is highlighted
	CSolid *GetActivePartPointer();

	bool MeetsConstraints(double dParam) const;

	std::string GetConstraint() const;
	
	CChainSegment(CHAIN_SEGMENT_TYPE type
					,const CR3Vector &r3vConstRevoluteAxis
					,const double & dConstRevoluteAngle
					,const CR3Vector &r3vConstTranslation
					,bool bParameterIsConstrained = false
					,double dParameterMinimum=0.0
					,double dParameterMaximum=0.0
					);
	~CChainSegment(void);
	CTransformation GetConstantTransformation() const;
	CTransformation GetInvertedConstantTransformation() const;
	CRotation GetConstantRotation() const;
	CRotation GetInvertedConstantRotation() const;
	CTransformation GetParametrizedTransformation(double dParam) const;
	CTransformation GetInvertedParametrizedTransformation(double dParam) const;
	CTransformation GetVariablePartTransformation(double dParam) const;
	CTransformation GetInvertedVariablePartTransformation(double dParam) const;


	CHAIN_SEGMENT_TYPE m_type;
	std::string m_sTypeName;
	std::string m_sName;
	CR3Vector m_r3vConstRevoluteAxis;
	double m_dConstRevoluteAngle;
	CR3Vector m_r3vConstTranslation;

	CToolHolder m_oToolHolder;
	bool m_bToolHolderDeclared;
	CToolDescription m_oTool;
	CWorkpieceHolder m_oWorkpieceHolder;
	bool m_bWorkpieceHolderDeclared;
	double m_dActualParameterValue;
	double m_dParameterValueMinimum;
	double m_dParameterValueMaximum;
	bool m_bParameterIsConstrained;

	//The tool holder of a TOOL_SPINDLE_NOSE segment, from the chain file: what the spindle of this
	//machine usually takes, hanging from the nose, as a default to offer. It places nothing - what
	//hangs the tool below the nose is the offset a caller gives, see CChain::GetToolOffset. No
	//other type of segment may carry one.
	const CToolHolder & GetToolHolder() const { return m_oToolHolder; }
	/**
	 * The tool the machine usually holds, from the chain file, on a TOOL_SPINDLE_NOSE segment and
	 * nowhere else. A default for whoever puts something in the spindle: it takes no part in the
	 * kinematics, which work from the tool offset given at run time.
	 */
	const CToolDescription & GetTool() const { return m_oTool; }
	void SetTool(const CToolDescription &oTool) { m_oTool = oTool; }
	/**
	 * The workpiece holder the machine usually carries, from the chain file, on a TABLE segment and
	 * nowhere else: the vice, the chuck, the plate the part is bolted to. A default to offer, like
	 * the tool and its holder at the other end of the machine.
	 */
	const CWorkpieceHolder & GetWorkpieceHolder() const { return m_oWorkpieceHolder; }
	bool IsWorkpieceHolderDeclared() const { return m_bWorkpieceHolderDeclared; }
	void SetWorkpieceHolder(const CWorkpieceHolder &oHolder)
	{ m_oWorkpieceHolder = oHolder; m_bWorkpieceHolderDeclared = true; }
	bool IsToolHolderDeclared() const { return m_bToolHolderDeclared; }
	void SetToolHolder(const CToolHolder &oHolder) { m_oToolHolder = oHolder; m_bToolHolderDeclared = true; }

	bool IsParameterConstrained() const;
	bool IsRegularJoint() const;
	CHAIN_SEGMENT_TYPE GetType() const;
	void SetType(CHAIN_SEGMENT_TYPE type);
	void SetType(const std::string &sType);
	void SetName(const std::string &sName);
	void SetParameterValue(double dValue);
	std::string GetName() const;
	std::string GetTypeName() const;
};

} // namespace kin5ax
