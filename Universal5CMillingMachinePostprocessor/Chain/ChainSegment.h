#pragma once
#include "r3vector.h"
#include "r3point.h"
#include "r3box.h"

#include "rotation.h"

enum CHAIN_SEGMENT_TYPE{
	CHAIN_SEGMENT_REVOLUTE_LINK=1,
	CHAIN_SEGMENT_PRISMATIC_LINK=2,
	CHAIN_SEGMENT_MATERIAL_HOLDER=3,
	CHAIN_SEGMENT_TOOL=4,

};


class CChainSegment
{

protected:
	vector<CR3Box> m_vParts;
	vector<bool> m_vCollisions;
	int m_nHighlightedPart;

public:
	bool HasPartInCollision(string &sLog) const;
	void ClearAllPartCollision();

	void SetPartCollision(size_t n);
	bool IsPartInCollision(size_t n) const;
	void SetPartVector(vector<CR3Box> vNewParts);
	const vector<CR3Box> &GetPartVector()const;
	void HighlightPart(int n);
	int GetHighlightedPartIndex() const;
	CR3Box *GetAvtivePartPointer();

	bool MeetsConstraints(double dParam) const;

	string GetConstraint() const;
	
	CChainSegment(CHAIN_SEGMENT_TYPE type
					,const CR3Vector &r3vConstRevoluteAxis
					,const double & dConstRevoluteAngle
					,const CR3Vector &r3vConstTranslation
					,bool bParameterIsConstrained = false
					,double dParamtererMinimum=0.0
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
	string m_sTypeName;
	string m_sName;
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
	void SetType(const string &sType);
	void SetName(const string &sName);
	void SetParameterValue(double dValue);
	string GetName() const;
	string toXML(void) const;
	string GetTypeName() const;
};
