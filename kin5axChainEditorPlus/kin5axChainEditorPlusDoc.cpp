// kin5axChainEditorPlusDoc.cpp : implementation of the CKin5axChainEditorPlusDoc class
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "kin5axChainEditorPlusDoc.h"
#include ".\kin5axChainEditorPlusDoc.h"


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
// CKin5axChainEditorPlusDoc

IMPLEMENT_DYNCREATE(CKin5axChainEditorPlusDoc, CDocument)

CKin5axChainEditorPlusDoc::CHAIN_FILE_READER
	CKin5axChainEditorPlusDoc::s_nextReader = CKin5axChainEditorPlusDoc::CHAIN_FILE_READER_FILE;
CFcmOptions CKin5axChainEditorPlusDoc::s_fcmOptions;
CFcmReport CKin5axChainEditorPlusDoc::s_fcmReport;

BEGIN_MESSAGE_MAP(CKin5axChainEditorPlusDoc, CDocument)
END_MESSAGE_MAP()


// CKin5axChainEditorPlusDoc construction/destruction

CKin5axChainEditorPlusDoc::CKin5axChainEditorPlusDoc()
:m_pPostprocessor(0)
,m_pMillingTool(0)

{
	m_pMillingTool = new CMillingToolBall(0.016,0.016
		,0.064,0.016);

}

CKin5axChainEditorPlusDoc::~CKin5axChainEditorPlusDoc()
{
	delete m_pPostprocessor;
	delete m_pMillingTool;
}

BOOL CKin5axChainEditorPlusDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	return TRUE;
}




// CKin5axChainEditorPlusDoc serialization

void CKin5axChainEditorPlusDoc::Serialize(CArchive& ar)
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


// CKin5axChainEditorPlusDoc diagnostics

#ifdef _DEBUG
void CKin5axChainEditorPlusDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CKin5axChainEditorPlusDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CKin5axChainEditorPlusDoc commands

void CKin5axChainEditorPlusDoc::Render3D(bool bFastRendering, const CR3Vector &r3vMaterialObserver, double dPixelSize)
{
	m_oChain.Update(m_pMillingTool);
	CChainRendererGL::Render(m_oChain,bFastRendering,m_oMaterial,r3vMaterialObserver,dPixelSize,m_pMillingTool,m_oCutterLocationData);

}


BOOL CKin5axChainEditorPlusDoc::OnOpenDocument(LPCTSTR lpszPathName)
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
		if(m_pMillingTool){
			//the machine brings its own tool holder; a tool that has one of its own keeps it
			if(!m_pMillingTool->m_oHolder.HasBody() && m_oChain.HasDefaultToolHolder())
				m_pMillingTool->m_oHolder = m_oChain.GetDefaultToolHolder();
			m_oChain.SetToolAssembly(*m_pMillingTool);
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

BOOL CKin5axChainEditorPlusDoc::OnSaveDocument(LPCTSTR lpszPathName)
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

C5CPostprocessor * CKin5axChainEditorPlusDoc::GetPostprocessor(void)
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
