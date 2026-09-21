#pragma once

#include "../material.h"
#include "../millingtool.h"
#include "../CutterLocationData.h"
#include "chainsegment.h"
#include "chainconfiguration.h"
#include "../gltools.h"
#include "r3caughtvector.h"
#include "r3box.h"


enum CHAIN_CONSTRAINTS_RESULT{
	CHAIN_CONSTRAINTS_MET,
	CHAIN_CONSTRAINTS_SHIFTS_EXCEEDED,
	CHAIN_CONSTRAINTS_ROTATIONS_EXCEEDED,
	CHAIN_CONSTRAINTS_SHIFTS_AND_ROTATION_EXCIDED
};

class CChain
{

	CChainConfiguration m_oActualParameters;
public:
	double GetActualParameter(size_t nIndex, bool bToolPart) const;
	void DetectCollisions(CMillingTool &oMillingTool);
	CChain(void);
	CChain(MSXML2::IXMLDOMElementPtr & pDocElement, const string &sFileName);
	string toXML() const;
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
		CChainConfiguration & oConfig) const;

	void GetTranslationsIndexes(size_t &nFirst, size_t &nSecond, size_t &nThird) const;
	void GetRotationsIndexes(size_t &nFirst, size_t &nSecond) const;
	CChainConfiguration	GetParameters() const;
	void SetParameters(const CChainConfiguration & oConfig);

	CChainSegment & operator[](int nIndex);
	const CChainSegment & operator[](int nIndex) const;
	void SetVirtualToolPosition(const CR3Vector &r3v, const CR3Point &r3p);
	// part of chain ended with milling tool 
	vector<CChainSegment> m_vToolPart;
	size_t GetNumberOfRegularLinks() const; 
	//part of chain ended with material holder
	vector<CChainSegment> m_vMaterialHolderPart;

	string m_sFileName;

	int m_nActiveSegmentIndex;
	bool m_bIsToolPartActive;

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
	vector<CChainSegment> & GetActivePart();
	void SetActiveSegment(bool bIsToolPartActive, int mActiveSegmentIndex);
	void SetFileName(const string &sName);
	string GetFileName() const ;
	void Render3D(bool bFastRendering
		, CMaterial &oMaterial
		, CMillingTool *pTool
		,const CCutterLocationData &oData);
	void CheckAndCorrectSegmentsNames(void);
	
	bool HasParameterConstraints() const;
	CHAIN_CONSTRAINTS_RESULT MeetsParametersConstraints(const CChainConfiguration &oChainConfig,string &sExceededConstraints) const; 
	bool HasCollision(CMillingTool &oMillingTool,string &sLog);



};
