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
	//the tool comes from the machine file; an empty chain names none, so there is none yet
	UpdateToolFromChain();
	UpdateViewUnitFromChain();
}

void CKin5axChainEditorPlusDoc::UpdateViewUnitFromChain()
{
	//1 for a chain in metres, 1000 for one in millimetres, 39.37 for one in inches: an .fcm file
	//is always millimetres, so this is not a corner case
	const double dInMillimetres = m_oChain.GetUnitInMillimetres();
	oConfiguration.m_dUnit = (dInMillimetres>0.0) ? 1000.0/dInMillimetres : 1.0;
}

void CKin5axChainEditorPlusDoc::UpdateToolFromChain()
{
	delete m_pMillingTool;
	const CToolAssembly oAssembly = m_oChain.GetDefaultToolAssembly();
	m_pMillingTool = oAssembly.MakeTool();		//null when the file names no tool
	//the offset is given to the chain whether or not there are solids to check: a holder with no
	//tool in it still holds the programmed point away from the nose
	m_oChain.SetToolOffset(oAssembly.GetToolOffset());
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
			default:	//.kc.json, the only form the library reads
				m_oChain = CChainFileReader::ReadFile(std::string(lpszPathName));
				break;
		}
		//the machine brings its own holder and its own tool: that assembly is what is in the spindle
		UpdateToolFromChain();
		UpdateViewUnitFromChain();		//and the view draws its frames in the unit of this file
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
	//one form, whatever the name says: the XML one was retired with tools/kc2json as the way out
	const std::string sText = CChainJsonWriter::ToJson(m_oChain);
	int nResult = fputs(sText.c_str(),file);
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
