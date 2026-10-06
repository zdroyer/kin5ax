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
#include "SimpleSimulation.h"
#include "CLDataPostprocessor.h"
#include <filesystem>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

extern CApplicationConfiguration oConfiguration;
extern CPostprocessorSettings oPostprocessorSettings;

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
	ON_COMMAND(ID_SIMULATION_SIMPLE, OnSimulationSimple)
	ON_WM_TIMER()
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
,m_nSimulationTimerInterval(0)
,m_nStepsSinceRepaint(0)
{

}

CMainFrame::~CMainFrame()
{
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

//The simulation runs on the UI thread: a timer tick does one location of the trajectory, or a few when
//the interval is below the timer resolution, repaints the view every refresh period and reports the end.
static const UINT_PTR nSIMULATION_TIMER = 1;

bool CMainFrame::StartSimulation(void)
{
	CUniversal5CMillingMachinePostprocessorDoc* pDoc = (CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument();
	if(!pDoc)
		return false;
	if(!m_pSimulation){
		if(!pDoc->m_pMillingTool){
			MessageBox("Cannot start simulation - no milling tool defined");
			return false;
		}
		if(!pDoc->m_oChain.Is5CChain()){
			MessageBox("Cannot start simulation - this is not a valid 5C chain model");
			return false;
		}
		if(pDoc->m_oSimulationData.GetSize()==0){
			MessageBox("No data for simulation\nRead and solve CL-DATA file first");
			return false;
		}
		m_pSimulation.reset(new CMaterialSimulation(pDoc->m_oChain,*pDoc->m_pMillingTool,pDoc->m_oSimulationData,pDoc->m_oMaterial));
		m_nStepsSinceRepaint = 0;
	}
	m_nSimulationTimerInterval = (UINT)oConfiguration.m_nSimulationInterval;
	SetTimer(nSIMULATION_TIMER,std::max(m_nSimulationTimerInterval,(UINT)1),NULL);
	return true;
}

void CMainFrame::PauseSimulation(void)
{
	KillTimer(nSIMULATION_TIMER);
}

void CMainFrame::StopAndResetSimulation(void)
{
	KillTimer(nSIMULATION_TIMER);
	m_pSimulation.reset();
	Invalidate();
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent==nSIMULATION_TIMER)
		RunSimulationSteps();
	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::RunSimulationSteps()
{
	if(!m_pSimulation){
		KillTimer(nSIMULATION_TIMER);
		return;
	}
	//the interval may have been changed in the dialog while running
	if(m_nSimulationTimerInterval!=(UINT)oConfiguration.m_nSimulationInterval){
		m_nSimulationTimerInterval = (UINT)oConfiguration.m_nSimulationInterval;
		SetTimer(nSIMULATION_TIMER,std::max(m_nSimulationTimerInterval,(UINT)1),NULL);
	}
	//below the timer resolution (about 16 ms) several locations per tick keep the speed the interval asks for
	const UINT nTimerResolution = 16;
	const size_t nSteps = m_nSimulationTimerInterval>=nTimerResolution ? 1 : nTimerResolution/std::max(m_nSimulationTimerInterval,(UINT)1);
	CView *pView = GetActiveView();
	for(size_t i=0;i<nSteps;i++){
		if(!m_pSimulation->Step()){
			CUniversal5CMillingMachinePostprocessorDoc* pDoc = (CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument();
			double dRemoved = pDoc ? pDoc->m_oMaterial.GetRemovedVolume() : 0.0;
			double dTotal = pDoc ? pDoc->m_oMaterial.GetTotalVolume() : 0.0;
			size_t nGouges = m_pSimulation->GetGougeCount();
			CR3Point r3pFirstGouge = m_pSimulation->GetFirstGougePoint();
			m_dlgSimulationDialog.OnBnClickedButtonStop();	//kills the timer, frees the simulation, repaints, resets the button
			std::string sMessage = "Milling simulation finished\nRemoved "+double_to_string(dRemoved)+" of "
				+double_to_string(dTotal)+" (length unit cubed), "
				+fixed_to_string(dTotal>0.0 ? 100.0*dRemoved/dTotal : 0.0,2)+" %";
			if(nGouges)
				sMessage += ("\nThe shank or the holder of the tool met material at "+size_to_string(nGouges)
					+" location(s), the first at "+r3pFirstGouge.toString());
			MessageBox(sMessage.c_str());
			return;
		}
		m_nStepsSinceRepaint++;
		//a refresh period of 0 means no repaint during the simulation
		if(pView && oConfiguration.m_nRefreshPeriod && m_nStepsSinceRepaint>=oConfiguration.m_nRefreshPeriod){
			m_nStepsSinceRepaint = 0;
			pView->Invalidate(FALSE);
			pView->UpdateWindow();
		}
	}
}


//Simulation > Simple simulation (hemisphere): a hemisphere and a material block sized for the machine
//(CSimpleSimulationSetup), the hemisphere postprocessed with the current settings and tool, the simulation
//dialog opened and the simulation started
void CMainFrame::OnSimulationSimple()
{
	CUniversal5CMillingMachinePostprocessorDoc* pDoc = (CUniversal5CMillingMachinePostprocessorDoc*)GetActiveDocument();
	if(!pDoc)
		return;
	if(!pDoc->m_oChain.Is5CChain()){
		MessageBox("Simple simulation needs a valid 5C chain: three linear and two rotary joints in the two parts");
		return;
	}
	if(!pDoc->m_pMillingTool){
		MessageBox("Simple simulation needs a milling tool");
		return;
	}
	StopAndResetSimulation();
	CSimpleSimulationSetup oSetup = CSimpleSimulationSetup::ForMachine(pDoc->m_oChain,*pDoc->m_pMillingTool);
	std::string sFilePath = (std::filesystem::temp_directory_path()/"kin5ax_simple_simulation.cld").string();
	size_t nLocations = oSetup.WriteCLData(sFilePath);
	if(nLocations==0){
		MessageBox(("Cannot write the hemisphere tool path to "+sFilePath).c_str());
		return;
	}
	CString sTitle;
	GetWindowText(sTitle);
	SetWindowText("Simple simulation: postprocessing the hemisphere ...");
	size_t nSolved = 0;
	try{
		pDoc->m_oCutterLocationData.LoadFile(sFilePath,"hemisphere R="+double_to_string(oSetup.m_dRadius));
		CCLDataPostprocessor oPostprocessor(oPostprocessorSettings);
		CCLDataPostprocessingResult oResult = oPostprocessor.Process(pDoc->m_oChain,*pDoc->m_pMillingTool
			,pDoc->m_oCutterLocationData,pDoc->m_oSimulationData,NULL,0,NULL);
		//the start of the trajectory is the convention of the machine: the rotary axis nearest the tool,
		//as close to zero as the solutions allow
		pDoc->m_oSimulationData.FindShortestPath(oPostprocessorSettings,NULL
			,pDoc->m_oChain.GetTrajectoryStart());
		nSolved = oResult.CountOf(SOLVE_STATUS_SOLVED);
	} catch (CGeneratorException &e){
		SetWindowText(sTitle);
		MessageBox(e.Message().c_str());
		return;
	}
	SetWindowText(sTitle);
	pDoc->m_oMaterial.GenerateBox(oSetup.m_dBoxSize,oSetup.m_dBoxSize,oSetup.m_dBoxHeight,oSetup.m_dVoxelSize
		,oSetup.GetBoxStart(),oSetup.GetBoxStart(),0.0);
	oConfiguration.m_dUnit = oSetup.m_dBoxSize/0.9;	//the view scale, as the material block dialog sets it
	pDoc->UpdateAllViews(NULL);
	std::string sReport = "Hemisphere of radius "+double_to_string(oSetup.m_dRadius)+" (an eighth of the working range "
		+double_to_string(oSetup.m_dWorkingRange)+"), "+size_to_string(nLocations)+" cutter locations, "
		+size_to_string(nSolved)+" of them solved on this machine.\n"
		+"Material block "+double_to_string(oSetup.m_dBoxSize)+" x "+double_to_string(oSetup.m_dBoxSize)+" x "
		+double_to_string(oSetup.m_dBoxHeight)+" in voxels of "+double_to_string(oSetup.m_dVoxelSize)+".\n"
		+"The tool path is in "+sFilePath+"."
		+"\nTool assembly "+double_to_string(pDoc->m_pMillingTool->GetAssemblyLength())+" long, its point "
		+double_to_string(pDoc->m_oChain.GetToolMountToToolPointDistance())+" below the tool mount."
		+"\n\nThe simulation starts when you close this message.";
	MessageBox(sReport.c_str(),"Simple simulation");
	if(nSolved==0)
		return;
	if(!m_bSimulationDialogVisible)
		OnSimulationDialog();
	m_dlgSimulationDialog.SetActiveDocument(pDoc);
	m_dlgSimulationDialog.OnBnClickedButtonPlayPause();
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
