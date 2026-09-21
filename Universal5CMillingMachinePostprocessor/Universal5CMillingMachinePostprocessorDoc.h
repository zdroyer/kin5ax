// Universal5CMillingMachinePostprocessorDoc.h : interface of the CUniversal5CMillingMachinePostprocessorDoc class
//


#pragma once
#include "gltools.h"
#include "chain.h"
#include "5CPostprocessor.h"
#include "material.h"
#include "CutterLocationData.h"
#include "millingtool.h"
#include "simulationdata.h"

class CUniversal5CMillingMachinePostprocessorDoc : public CDocument
{
protected: // create from serialization only
	CUniversal5CMillingMachinePostprocessorDoc();
	DECLARE_DYNCREATE(CUniversal5CMillingMachinePostprocessorDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CUniversal5CMillingMachinePostprocessorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	CGLTools m_oGLTools;
public:
	void Render3D(bool bFastRendering);
	// kinametic chain of milling machine
	CChain m_oChain;
	CCutterLocationData m_oCutterLocationData;
	C5CPostprocessor *m_pPostprocessor;
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	C5CPostprocessor * GetPostprocessor(void);
	CMaterial m_oMaterial;
	CMillingTool *m_pMillingTool;
	CSimulationData m_oSimulationData;
};


