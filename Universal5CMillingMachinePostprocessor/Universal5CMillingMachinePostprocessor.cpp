// Universal5CMillingMachinePostprocessor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "MainFrm.h"
#include "gltools.h"

#include "Universal5CMillingMachinePostprocessorDoc.h"
#include "ChainJson.h"
#include "Universal5CMillingMachinePostprocessorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUniversal5CMillingMachinePostprocessorApp

BEGIN_MESSAGE_MAP(CUniversal5CMillingMachinePostprocessorApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_PUGIXML, OnFileOpenPugixml)
	ON_COMMAND(ID_FILE_OPEN_JSON, OnFileOpenJson)
	ON_COMMAND(ID_FILE_SAVE_AS_JSON, OnFileSaveAsJson)
END_MESSAGE_MAP()


// CUniversal5CMillingMachinePostprocessorApp construction

CUniversal5CMillingMachinePostprocessorApp::CUniversal5CMillingMachinePostprocessorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CUniversal5CMillingMachinePostprocessorApp object
CUniversal5CMillingMachinePostprocessorApp theApp;

// CUniversal5CMillingMachinePostprocessorApp initialization

BOOL CUniversal5CMillingMachinePostprocessorApp::InitInstance()
{
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();
	::CoInitialize(NULL);
	CWinApp::InitInstance();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(5);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CUniversal5CMillingMachinePostprocessorDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CUniversal5CMillingMachinePostprocessorView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CUniversal5CMillingMachinePostprocessorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CUniversal5CMillingMachinePostprocessorApp message handlers


//Opens a chain file with the given reader; the framework sets the title and the recent files
void CUniversal5CMillingMachinePostprocessorApp::OpenWith(int nReader, LPCTSTR lpszPathName)
{
	CUniversal5CMillingMachinePostprocessorDoc::s_nextReader =
		(CUniversal5CMillingMachinePostprocessorDoc::CHAIN_FILE_READER)nReader;
	OpenDocumentFile(lpszPathName);
	CUniversal5CMillingMachinePostprocessorDoc::s_nextReader =
		CUniversal5CMillingMachinePostprocessorDoc::CHAIN_FILE_READER_MSXML;
}

//Chain > Open (pugixml): the same as Open, read by the portable reader of kin5ax
void CUniversal5CMillingMachinePostprocessorApp::OnFileOpenPugixml()
{
	CFileDialog dlg(TRUE,"kc",NULL,OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
		"Kinematic chain files (*.kc)|*.kc|All files (*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	OpenWith(CUniversal5CMillingMachinePostprocessorDoc::CHAIN_FILE_READER_PUGIXML,dlg.GetPathName());
}

//Chain > Open (JSON): a chain from its JSON form (kin5ax/kc.schema.json)
void CUniversal5CMillingMachinePostprocessorApp::OnFileOpenJson()
{
	CFileDialog dlg(TRUE,"json",NULL,OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
		"Kinematic chain, JSON (*.kc.json;*.json)|*.kc.json;*.json|All files (*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	OpenWith(CUniversal5CMillingMachinePostprocessorDoc::CHAIN_FILE_READER_JSON,dlg.GetPathName());
}

//Chain > Save As (JSON): the chain of the active document, its XML file untouched
void CUniversal5CMillingMachinePostprocessorApp::OnFileSaveAsJson()
{
	CFrameWnd *pFrame = (CFrameWnd*)AfxGetMainWnd();
	CUniversal5CMillingMachinePostprocessorDoc *pDoc = pFrame ?
		(CUniversal5CMillingMachinePostprocessorDoc*)pFrame->GetActiveDocument() : NULL;
	if(!pDoc){
		AfxMessageBox("No kinematic chain to save");
		return;
	}
	CFileDialog dlg(FALSE,"json",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"Kinematic chain, JSON (*.kc.json)|*.kc.json|All files (*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	try{
		CChainJsonWriter::WriteFile(pDoc->m_oChain,std::string(dlg.GetPathName()));
	} catch (CGeneratorException &e){
		AfxMessageBox(e.Message().c_str());
	}
}
