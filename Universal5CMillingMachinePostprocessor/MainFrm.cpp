// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "kinematicchainpropertiesdialog.h"
#include "MillingToolPropertiesDialog.h"
#include "settingsdialog.h"
#include ".\mainfrm.h"
#include "string_tools.h"
#include "MaterialSimulation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

extern CApplicationConfiguration oConfiguration;

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_POSTPROCESSOR_SINGLEINPUT, OnPostprocessorSingleinput)
	ON_COMMAND(ID_CHAIN_PROPERTIES, OnChainProperties)
	ON_COMMAND(ID_MATERIALBLOCK_GENERATEINITIALBLOCK, OnMaterialblockGenerateinitialblock)
	ON_COMMAND(ID_SIMULATION_SETUPMILLINGTOOL, OnSimulationSetupmillingtool)
	ON_COMMAND(ID_SHOW_MATERIAL, OnShowMaterial)
	ON_COMMAND(ID_SHOW_MATERIAL_CONTOUR, OnShowMaterialContour)
	ON_COMMAND(ID_SHOW_MATERIAL_FRAME, OnShowMaterialFrame)
	ON_COMMAND(ID_SHOW_TOOL, OnShowTool)
	ON_COMMAND(ID_SHOW_TOOL_FRAME, OnShowToolFrame)
	ON_COMMAND(ID_SHOW_LINKS, OnShowLinks)
	ON_COMMAND(ID_SHOW_LINKS_FRAMES, OnShowLinksFrames)
	ON_COMMAND(ID_SHOW_TOOL_TRAJECTORY, OnShowTooltrajectory)
	ON_COMMAND(ID_SHOW_MATERIAL_HOLDER, OnShowMaterialHolder)
	ON_COMMAND(ID_3DVIEW_FIXED_MATERIAL_FRAME, On3dviewFixedMaterialFrame)
	ON_COMMAND(ID_SHOW_BASE, OnShowBase)
	ON_COMMAND(ID_SHOW_BASE_FRAME, OnShowBaseFrame)
	ON_COMMAND(ID_SETTINGS_PROGRAMSETTINGS, OnSettingsProgramsettings)
	ON_COMMAND(ID_POSTPROCESSOR_CLDATAFILE, OnPostprocessorCldatafile)
	ON_WM_SHOWWINDOW()
	ON_COMMAND(ID_SIMULATION_DIALOG, OnSimulationDialog)
	ON_MESSAGE(WM_APP,OnAppMessage)
	ON_COMMAND(ID_SHOW_LINKS_PARTS, OnShowLinksParts)
	ON_COMMAND(ID_SHOW_TOOL_BOUNDING_BOX, OnShowToolBoundingBox)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
:m_bSimulationDialogVisible(false)
,m_pSimulationThread(0)
{

}

CMainFrame::~CMainFrame()
{
	oConfiguration.m_bStopSimulation = true;
	Sleep(100);
	::CoUninitialize();
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	if(oConfiguration.m_bPerspectiveProjection)
		GetMenu()->CheckMenuRadioItem(ID_3DVIEW_ORTHOGONAL_PROJECTION,ID_3DVIEW_PERSPECTIVE_PROJECTION,ID_3DVIEW_PERSPECTIVE_PROJECTION,MF_BYCOMMAND);
	else 
		GetMenu()->CheckMenuRadioItem(ID_3DVIEW_ORTHOGONAL_PROJECTION,ID_3DVIEW_PERSPECTIVE_PROJECTION,ID_3DVIEW_ORTHOGONAL_PROJECTION,MF_BYCOMMAND);
	UpdateShowHideMenu();
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers


void CMainFrame::OnFileNew()
{
	int nAction = MessageBox("Do you want to save actual chain ?","Confirmation",MB_YESNOCANCEL);
	if(nAction == IDCANCEL )
		return;
	if(nAction == IDYES){
		((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument())->DoFileSave();
	} 
	((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument())->m_oChain = CChain();
	GetActiveDocument()->SetTitle("Untitled");
	OnChainProperties();
}

void CMainFrame::OnPostprocessorSingleinput()
{
	static bool bSingleInputDialogCreated=false;
	if(!bSingleInputDialogCreated){
		m_dlgSingleInputDialog.Create(ID_SINGLE_INPUT_DIALOG,this);
		bSingleInputDialogCreated=true;
	}
	m_dlgSingleInputDialog.SetActiveDocument((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument());
	m_dlgSingleInputDialog.ShowWindow(SW_SHOW);
}

void CMainFrame::OnChainProperties()
{
	static bool bCreated=false;
	if(!bCreated){
		m_dlgChainProperties.Create(ID_CHAIN_CONSTRUCTING_DIALOG,this);
		bCreated=true;
	}
	m_dlgChainProperties.SetActiveDocument((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument());
	m_dlgChainProperties.ShowWindow(SW_SHOW);
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::UpdateDialogs(void)
{

	if(m_dlgSingleInputDialog.m_hWnd)
		m_dlgSingleInputDialog.SetActiveDocument((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument());
	if(m_dlgChainProperties.m_hWnd)
		m_dlgChainProperties.SetActiveDocument((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument());
}


void CMainFrame::OnMaterialblockGenerateinitialblock()
{
	static bool bCreated=false;
	if(!bCreated){
		m_dlgMaterialBlockDialog.Create(ID_MATERIAL_DEFINITION_DIALOG,this);
		bCreated=true;
	}
	m_dlgMaterialBlockDialog.SetActiveDocument((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument());
	m_dlgMaterialBlockDialog.ShowWindow(SW_SHOW);
}

void CMainFrame::OnSimulationSetupmillingtool()
{
	CMillingToolPropertiesDialog m_dlg(this);
	m_dlg.SetActiveDocument((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument());
	m_dlg.DoModal();
}

void CMainFrame::OnShowMaterial()
{
	oConfiguration.m_bShowMaterial = (!oConfiguration.m_bShowMaterial);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
	// TODO: Add your command handler code here
}

void CMainFrame::OnShowMaterialContour()
{
	oConfiguration.m_bShowMaterialContour = (!oConfiguration.m_bShowMaterialContour);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::OnShowMaterialFrame()
{
	oConfiguration.m_bShowMaterialFrame = (!oConfiguration.m_bShowMaterialFrame);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::OnShowMaterialHolder()
{
	oConfiguration.m_bShowMaterialHolder = (!oConfiguration.m_bShowMaterialHolder);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}


void CMainFrame::OnShowTool()
{
	oConfiguration.m_bShowTool = (!oConfiguration.m_bShowTool);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::OnShowToolFrame()
{
	oConfiguration.m_bShowToolFrame = (!oConfiguration.m_bShowToolFrame);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::OnShowTooltrajectory()
{
	oConfiguration.m_bShowToolTrajectory = (!oConfiguration.m_bShowToolTrajectory);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::OnShowLinks()
{
	oConfiguration.m_bShowLinks = (!oConfiguration.m_bShowLinks);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::OnShowLinksFrames()
{
	oConfiguration.m_bShowLinksFrames = (!oConfiguration.m_bShowLinksFrames);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}


void CMainFrame::UpdateShowHideMenu()
{
	UINT nCheck;
	nCheck = (oConfiguration.m_bShowLinks ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_LINKS,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowLinksFrames ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_LINKS_FRAMES,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowMaterial ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_MATERIAL,MF_BYCOMMAND | nCheck);
	
	nCheck = (oConfiguration.m_bShowMaterialContour ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_MATERIAL_CONTOUR,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowMaterialFrame ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_MATERIAL_FRAME,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowMaterialHolder ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_MATERIAL_HOLDER,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowTool ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_TOOL,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowToolBoundingBox ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_TOOL_BOUNDING_BOX,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowToolFrame ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_TOOL_FRAME,MF_BYCOMMAND | nCheck);
	
	nCheck = (oConfiguration.m_bShowToolTrajectory ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_TOOL_TRAJECTORY,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bFixedMaterialFrame ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_3DVIEW_FIXED_MATERIAL_FRAME,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowBase ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_BASE,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowBaseFrame ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_BASE_FRAME,MF_BYCOMMAND | nCheck);

	nCheck = (oConfiguration.m_bShowParts ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->CheckMenuItem(ID_SHOW_LINKS_PARTS,MF_BYCOMMAND | nCheck);


}

void CMainFrame::On3dviewFixedMaterialFrame()
{
	oConfiguration.m_bFixedMaterialFrame = (!oConfiguration.m_bFixedMaterialFrame);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::OnShowBase()
{
	oConfiguration.m_bShowBase = (!oConfiguration.m_bShowBase);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);

}

void CMainFrame::OnShowBaseFrame()
{
	oConfiguration.m_bShowBaseFrame = (!oConfiguration.m_bShowBaseFrame);
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::OnSettingsProgramsettings()
{
	CSettingsDialog dlg(this);
	dlg.DoModal();
}

void CMainFrame::OnPostprocessorCldatafile()
{
	static bool bCreated=false;
	if(!bCreated){
		m_dlgCLDataFileDialog.Create(ID_CLDATA_FILE_DIALOG,this);
		bCreated=true;
	}
	m_dlgCLDataFileDialog.SetActiveDocument((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument());
	m_dlgCLDataFileDialog.ShowWindow(SW_SHOW);

}

UINT simulationFunction(LPVOID pParam )
{
	CUniversal5CMillingMachinePostprocessorDoc* pDoc = (CUniversal5CMillingMachinePostprocessorDoc*)pParam;
	size_t i=0;
	if(!pDoc)
		AfxEndThread(1,TRUE);
	if(!pDoc->m_pMillingTool){
		MessageBox(0,"Cannot start simulation - no milling tool defined","Error",MB_OK);
		AfxEndThread(1,TRUE);
	}
	if(!pDoc->m_oChain.Is5CChain()){
		MessageBox(0,"Cannot start simulation - this is not valid 5C chain model","Error",MB_OK);
		AfxEndThread(1,TRUE);
	}
	POSITION pos = pDoc->GetFirstViewPosition();
	CView *pView = pDoc->GetNextView(pos);
	if(pView==0){
		MessageBox(0,"Cannot start simulation - invalid document object","Error",MB_OK);
		AfxEndThread(1,TRUE);
	}

	CMaterialSimulation oSimulation(pDoc->m_oChain,*pDoc->m_pMillingTool,pDoc->m_oSimulationData,pDoc->m_oMaterial);
	for(size_t i=0;;i++){
		if(oConfiguration.m_bStopSimulation){
			pView->Invalidate(FALSE);
			pView->GetParent()->SendNotifyMessage(WM_APP,1,0);
			AfxEndThread(0,TRUE);
			return 0;
		}
		// a refresh period of 0 means no repaint during the simulation
		if(oConfiguration.m_nRefreshPeriod && i>=oConfiguration.m_nRefreshPeriod){
			i=0;
			pView->SendNotifyMessage(WM_PAINT,0,0);
		}
		if(!oSimulation.Step())
			break;
		Sleep(oConfiguration.m_nSimulationInterval);
	}
	pView->GetParent()->SendNotifyMessage(WM_APP,0,0);
	AfxEndThread(0,TRUE);
	return 0;
}

bool CMainFrame::StartSimulation(void)
{
	oConfiguration.m_bStopSimulation=false;
	if(m_pSimulationThread){
		m_pSimulationThread->ResumeThread();	
		return true;
	}
	CUniversal5CMillingMachinePostprocessorDoc* pDoc = (CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument();
	if(pDoc->m_oSimulationData.GetSize()==0){
		MessageBox("No data fo simulation\nRead and solve CL-DATA file first");
		return false;
	}
	m_pSimulationThread = AfxBeginThread(simulationFunction,(LPVOID)pDoc,THREAD_PRIORITY_BELOW_NORMAL);	
	return true;
}

void CMainFrame::PauseSimulation(void)
{
	if(m_pSimulationThread)
		m_pSimulationThread->SuspendThread();

}

void CMainFrame::StopAndResetSimulation(void)
{
	oConfiguration.m_bStopSimulation=true;
	Sleep(200);
	m_pSimulationThread=0;
}


void CMainFrame::OnSimulationDialog()
{
	static bool bCreated = false;
	if(!bCreated){
		m_dlgSimulationDialog.Create(ID_SIMULATION_DIALOG,this);
		m_dlgSimulationDialog.SetActiveDocument((CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument());
		bCreated=true;
	}
	if(m_bSimulationDialogVisible){
		m_dlgSimulationDialog.ShowWindow(SW_HIDE);
		GetMenu()->CheckMenuItem(ID_SIMULATION_DIALOG,MF_BYCOMMAND | MF_UNCHECKED);
	} else {
		m_dlgSimulationDialog.ShowWindow(SW_SHOW);
		GetMenu()->CheckMenuItem(ID_SIMULATION_DIALOG,MF_BYCOMMAND | MF_CHECKED);

	}
	m_bSimulationDialogVisible= (!m_bSimulationDialogVisible);
}

LRESULT CMainFrame::OnAppMessage(WPARAM w,LPARAM l)
{
	m_dlgSimulationDialog.OnBnClickedButtonStop();
	if(w==0)
		MessageBox("Milling simulation finished");
	else
		MessageBox("Milling simulation aborded by user");
	Invalidate();
	return 0;
}


void CMainFrame::OnShowLinksParts()
{
	oConfiguration.m_bShowParts = !oConfiguration.m_bShowParts;
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}

void CMainFrame::OnShowToolBoundingBox()
{
	oConfiguration.m_bShowToolBoundingBox = !oConfiguration.m_bShowToolBoundingBox;
	UpdateShowHideMenu();
	GetActiveDocument()->UpdateAllViews(NULL);
}
