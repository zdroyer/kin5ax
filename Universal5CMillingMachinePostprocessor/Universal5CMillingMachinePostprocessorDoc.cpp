// Universal5CMillingMachinePostprocessorDoc.cpp : implementation of the CUniversal5CMillingMachinePostprocessorDoc class
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "Universal5CMillingMachinePostprocessorDoc.h"
#include ".\universal5cmillingmachinepostprocessordoc.h"


#include "millingtoolball.h"
#include "mainfrm.h"
#include "ChainRendererGL.h"
#include "ChainFileReader.h"
#include "ChainJson.h"
#include "ChainFcm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



extern CApplicationConfiguration oConfiguration;
extern CPostprocessorSettings oPostprocessorSettings;
extern CGLTools oGLTools;
// CUniversal5CMillingMachinePostprocessorDoc

IMPLEMENT_DYNCREATE(CUniversal5CMillingMachinePostprocessorDoc, CDocument)

CUniversal5CMillingMachinePostprocessorDoc::CHAIN_FILE_READER
	CUniversal5CMillingMachinePostprocessorDoc::s_nextReader = CUniversal5CMillingMachinePostprocessorDoc::CHAIN_FILE_READER_FILE;
CFcmOptions CUniversal5CMillingMachinePostprocessorDoc::s_fcmOptions;
CFcmReport CUniversal5CMillingMachinePostprocessorDoc::s_fcmReport;

BEGIN_MESSAGE_MAP(CUniversal5CMillingMachinePostprocessorDoc, CDocument)
END_MESSAGE_MAP()


// CUniversal5CMillingMachinePostprocessorDoc construction/destruction

CUniversal5CMillingMachinePostprocessorDoc::CUniversal5CMillingMachinePostprocessorDoc()
:m_pPostprocessor(0)
,m_pMillingTool(0)

{
	m_pMillingTool = new CMillingToolBall(0.016,0.016
		,0.064,0.016);

}

CUniversal5CMillingMachinePostprocessorDoc::~CUniversal5CMillingMachinePostprocessorDoc()
{
	delete m_pPostprocessor;
	delete m_pMillingTool;
}

BOOL CUniversal5CMillingMachinePostprocessorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	return TRUE;
}




// CUniversal5CMillingMachinePostprocessorDoc serialization

void CUniversal5CMillingMachinePostprocessorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{

		// TODO: add loading code here
	}
}


// CUniversal5CMillingMachinePostprocessorDoc diagnostics

#ifdef _DEBUG
void CUniversal5CMillingMachinePostprocessorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CUniversal5CMillingMachinePostprocessorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CUniversal5CMillingMachinePostprocessorDoc commands

void CUniversal5CMillingMachinePostprocessorDoc::Render3D(bool bFastRendering)
{
	
	m_oChain.Update(m_pMillingTool);
	CChainRendererGL::Render(m_oChain,bFastRendering,m_oMaterial,m_pMillingTool,m_oCutterLocationData);

}


BOOL CUniversal5CMillingMachinePostprocessorDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
//	if (!CDocument::OnOpenDocument(lpszPathName))
//		return FALSE;
	try{
		switch(s_nextReader){
			case CHAIN_FILE_READER_FCM:
				m_oChain = CChainFcmReader::ReadFile(std::string(lpszPathName),s_fcmOptions,s_fcmReport);
				break;
			default:	//.kc or .kc.json, the reader tells them apart
				m_oChain = CChainFileReader::ReadFile(std::string(lpszPathName));
				break;
		}
	} catch (CGeneratorException &e){
		AfxMessageBox(e.Message().c_str());
		return FALSE;
	}

	//GetRoutingFrame() is set only while a menu command is routed: it is NULL when the file
	//comes from the command line, the shell or drag and drop
	CMainFrame *pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if(pMainFrame)
		pMainFrame->UpdateDialogs();
	//MessageBox(0,pDOMDoc->xml,lpszPathName,MB_OK);	
	UpdateAllViews(NULL);
	return TRUE;
}	

BOOL CUniversal5CMillingMachinePostprocessorDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class

	FILE * file = fopen(lpszPathName,"wb");
	if(file==NULL){
		MessageBox(0,lpszPathName,"Cannot open file to write",MB_OK);
		return FALSE;
	}
	//the form follows the name: a document opened from a .kc.json is saved as JSON
	std::string sXML = CChainFileReader::FormatOf(std::string(lpszPathName))==CChainFileReader::FORMAT_JSON
		? CChainJsonWriter::ToJson(m_oChain) : m_oChain.toXML();
	int nResult = fputs(sXML.c_str(),file);
	fclose(file);
	m_oChain.SetFileName(std::string(lpszPathName));
	if(nResult<0)
		MessageBox(0,lpszPathName,"Error with saving file",MB_OK);
	else
		MessageBox(0,lpszPathName,"File Saved",MB_OK);
	return TRUE;//CDocument::OnSaveDocument(lpszPathName);
}

C5CPostprocessor * CUniversal5CMillingMachinePostprocessorDoc::GetPostprocessor(void)
{
	delete m_pPostprocessor;
	m_pPostprocessor = 0;
	if(m_oChain.Is5CChain()){
		m_pPostprocessor = new C5CPostprocessor(m_oChain,oPostprocessorSettings);
		return m_pPostprocessor;
	} else {
		MY_THROW(CGeneratorException,"Attempt to create postprocessor for non-5C chain ");
	}
}
