#pragma once
#include "R3Vector.h"
#include "R3Point.h"
#include "R3Box.h"

#include "Rotation.h"

enum CHAIN_SEGMENT_TYPE{
	CHAIN_SEGMENT_REVOLUTE_LINK=1,
	CHAIN_SEGMENT_PRISMATIC_LINK=2,
	CHAIN_SEGMENT_MATERIAL_HOLDER=3,
	CHAIN_SEGMENT_TOOL=4,

};


class CChainSegment
{

protected:
	std::vector<CR3Box> m_vParts;		//the solids of the segment, in the frame after its constant transformation
	std::vector<bool> m_vCollisions;	//one flag per box, set by CChain::DetectCollisions
	int m_nHighlightedPart;

public:
	//Is any box of the segment flagged? Then sLog names the segment and the boxes.
	bool HasPartInCollision(std::string &sLog) const;
	void ClearAllPartCollision();

	//the collision flag of box n (0-based)
	void SetPartCollision(size_t n);
	bool IsPartInCollision(size_t n) const;
	void SetPartVector(std::vector<CR3Box> vNewParts);
	const std::vector<CR3Box> &GetPartVector()const;
	void HighlightPart(int n);
	int GetHighlightedPartIndex() const;
	CR3Box *GetActivePartPointer();

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

	double m_dActualParameterValue;
	double m_dParameterValueMinimum;
	double m_dParameterValueMaximum;
	bool m_bParameterIsConstrained;

	bool IsParameterConstrained() const;
	bool IsRegularJoint() const;
	CHAIN_SEGMENT_TYPE GetType() const;
	void SetType(CHAIN_SEGMENT_TYPE type);
	void SetType(const std::string &sType);
	void SetName(const std::string &sName);
	void SetParameterValue(double dValue);
	std::string GetName() const;
	std::string toXML(void) const;
	std::string GetTypeName() const;
};
