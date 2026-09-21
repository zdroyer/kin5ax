// Universal5CMillingMachinePostprocessorDoc.cpp : implementation of the CUniversal5CMillingMachinePostprocessorDoc class
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "Universal5CMillingMachinePostprocessorDoc.h"
#include ".\universal5cmillingmachinepostprocessordoc.h"


#include "millingtoolball.h"
#include "mainfrm.h"
#include "ChainRendererGL.h"
#include "ChainXmlReader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



extern CApplicationConfiguration oConfiguration;
extern CPostprocessorSettings oPostprocessorSettings;
extern CGLTools oGLTools;
// CUniversal5CMillingMachinePostprocessorDoc

IMPLEMENT_DYNCREATE(CUniversal5CMillingMachinePostprocessorDoc, CDocument)

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
	MSXML2::IXMLDOMDocument2Ptr pDOMDoc;
	
	HRESULT hr = pDOMDoc.CreateInstance(__uuidof(DOMDocument60));
	MSXML2::IXMLDOMDocument2 * pXMLDoc = pDOMDoc;
	if (FAILED(hr))
	{
		_com_error er(hr);
		AfxMessageBox(er.ErrorMessage()+CString("xreate instance error"));
		return FALSE;
	}
	pDOMDoc->put_async(VARIANT_FALSE);
	pDOMDoc->put_validateOnParse(VARIANT_TRUE);
	// MSXML 6 prohibits DTDs by default; .kc files carry an inline DTD
	pDOMDoc->setProperty(_bstr_t("ProhibitDTD"), _variant_t(false));
	_bstr_t bstrURL = lpszPathName;
	VARIANT_BOOL varResult = pDOMDoc->load(_variant_t(bstrURL));
	if (VARIANT_FALSE == varResult)
	{
		// Load failed
		MSXML2::IXMLDOMParseErrorPtr pParseError = pDOMDoc->GetparseError();
		_bstr_t bstrReason = pParseError->Getreason();
		CString strError;
		strError.Format(_T("XML Parse Error\nLine : %ld; Char : %ld\nError: %s"),pParseError->line, pParseError->linepos,(char*)bstrReason);
		AfxMessageBox(strError);
		return FALSE;
	}
	m_oChain = CChainXmlReader::ReadChain(pXMLDoc->GetdocumentElement(),string(lpszPathName));

	
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
	string sXML = m_oChain.toXML();
	int nResult = fprintf(file,sXML.c_str());
	fclose(file);
	m_oChain.SetFileName(string(lpszPathName));
	if(nResult<0)
		MessageBox(0,lpszPathName,"Error with saving file",MB_OK);
	else
		MessageBox(0,lpszPathName,"File Saved",MB_OK);
	return TRUE;//CDocument::OnSaveDocument(lpszPathName);
}

C5CPostprocessor * CUniversal5CMillingMachinePostprocessorDoc::GetPostprocessor(void)
{
	if(m_pPostprocessor!=0)
		delete m_pPostprocessor;
	if(m_oChain.Is5CChain()){
		m_pPostprocessor = new C5CPostprocessor(m_oChain,oPostprocessorSettings);
		return m_pPostprocessor;
	} else {
		MY_THROW(CGeneratorException,"Attempt to create postprocessor for non-5C chain ");
	}
}
