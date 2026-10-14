// kin5axChainEditorPlus.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "MainFrm.h"
#include "gltools.h"

#include "kin5axChainEditorPlusDoc.h"
#include "ChainJson.h"
#include "ChainFcm.h"
#include "kin5axChainEditorPlusView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CKin5axChainEditorPlusApp

BEGIN_MESSAGE_MAP(CKin5axChainEditorPlusApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE_AS_JSON, OnFileSaveAsJson)
	ON_COMMAND(ID_FILE_IMPORT_FCM, OnFileImportFcm)
	ON_COMMAND(ID_FILE_EXPORT_FCM, OnFileExportFcm)
END_MESSAGE_MAP()


// CKin5axChainEditorPlusApp construction

CKin5axChainEditorPlusApp::CKin5axChainEditorPlusApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CKin5axChainEditorPlusApp object
CKin5axChainEditorPlusApp theApp;

// CKin5axChainEditorPlusApp initialization

BOOL CKin5axChainEditorPlusApp::InitInstance()
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
		RUNTIME_CLASS(CKin5axChainEditorPlusDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CKin5axChainEditorPlusView));
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
void CKin5axChainEditorPlusApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CKin5axChainEditorPlusApp message handlers


//Opens a chain file with the given reader; the framework sets the title and the recent files
void CKin5axChainEditorPlusApp::OpenWith(int nReader, LPCTSTR lpszPathName)
{
	CKin5axChainEditorPlusDoc::s_nextReader =
		(CKin5axChainEditorPlusDoc::CHAIN_FILE_READER)nReader;
	OpenDocumentFile(lpszPathName);
	CKin5axChainEditorPlusDoc::s_nextReader =
		CKin5axChainEditorPlusDoc::CHAIN_FILE_READER_FILE;
}

//Chain > Save As (JSON): the chain of the active document, its XML file untouched
void CKin5axChainEditorPlusApp::OnFileSaveAsJson()
{
	CFrameWnd *pFrame = (CFrameWnd*)AfxGetMainWnd();
	CKin5axChainEditorPlusDoc *pDoc = pFrame ?
		(CKin5axChainEditorPlusDoc*)pFrame->GetActiveDocument() : NULL;
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

//An .fcm file and a kinematic chain cannot hold the same things; the conversion says what it
//had to leave behind instead of dropping it silently.
static void ShowFcmReport(const CFcmReport &oReport, const char *szWhat)
{
	if(oReport.IsExact())
		return;
	AfxMessageBox((std::string(szWhat)+":\n\n- "+oReport.ToString("\n\n- ")).c_str());
}

//Chain > Import (FreeCAD .fcm): the machine of a FreeCAD CAM machine file
void CKin5axChainEditorPlusApp::OnFileImportFcm()
{
	CFileDialog dlg(TRUE,"fcm",NULL,OFN_HIDEREADONLY|OFN_FILEMUSTEXIST,
		"FreeCAD machine (*.fcm)|*.fcm|All files (*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	CKin5axChainEditorPlusDoc::s_fcmReport.Clear();
	OpenWith(CKin5axChainEditorPlusDoc::CHAIN_FILE_READER_FCM,dlg.GetPathName());
	ShowFcmReport(CKin5axChainEditorPlusDoc::s_fcmReport
		,"The machine was read from the file, with these remarks");
}

//Chain > Export (FreeCAD .fcm): the chain of the active document as a FreeCAD machine
void CKin5axChainEditorPlusApp::OnFileExportFcm()
{
	CFrameWnd *pFrame = (CFrameWnd*)AfxGetMainWnd();
	CKin5axChainEditorPlusDoc *pDoc = pFrame ?
		(CKin5axChainEditorPlusDoc*)pFrame->GetActiveDocument() : NULL;
	if(!pDoc){
		AfxMessageBox("No kinematic chain to export");
		return;
	}
	CFileDialog dlg(FALSE,"fcm",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"FreeCAD machine (*.fcm)|*.fcm|All files (*.*)|*.*||");
	if(dlg.DoModal()!=IDOK)
		return;
	CFcmOptions oOptions;
	CFcmReport oReport;
	try{
		CChainFcmWriter::WriteFile(pDoc->m_oChain,std::string(dlg.GetPathName()),oOptions,oReport);
	} catch (CGeneratorException &e){
		AfxMessageBox(e.Message().c_str());
		return;
	}
	ShowFcmReport(oReport,"The machine was written to the file, with these remarks");
}
