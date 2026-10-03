// Universal5CMillingMachinePostprocessorDoc.h : interface of the CUniversal5CMillingMachinePostprocessorDoc class
//


#pragma once
#include "gltools.h"
#include "chain.h"
#include "ChainFcm.h"
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
	//r3vMaterialObserver (in the material frame) and dPixelSize come from the view, see CMaterialRendererGL::Render
	void Render3D(bool bFastRendering, const CR3Vector &r3vMaterialObserver, double dPixelSize);
	// kinametic chain of milling machine
	CChain m_oChain;
	CCutterLocationData m_oCutterLocationData;
	C5CPostprocessor *m_pPostprocessor;
	//which reader the next OnOpenDocument uses; set by the application before OpenDocumentFile
	//(FILE: a .kc or a .kc.json, CChainFileReader tells them apart; the default. FCM: the import)
	enum CHAIN_FILE_READER { CHAIN_FILE_READER_FILE, CHAIN_FILE_READER_FCM };
	static CHAIN_FILE_READER s_nextReader;
	//the conventions the .fcm reader cannot read from the file, and what the last import could
	//not carry across
	static kin5ax::CFcmOptions s_fcmOptions;
	static kin5ax::CFcmReport s_fcmReport;
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	C5CPostprocessor * GetPostprocessor(void);
	CMaterial m_oMaterial;
	CMillingTool *m_pMillingTool;
	CSimulationData m_oSimulationData;
};


