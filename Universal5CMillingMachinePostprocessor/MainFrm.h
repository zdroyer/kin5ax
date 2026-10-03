// MainFrm.h : interface of the CMainFrame class
//


#pragma once
#include "kinematicchainpropertiesdialog.h"
#include "singleinputsolverdialog.h"
#include "materialblockdialog.h"
#include "cldatafiledialog.h"
#include "simulationdialog.h"
#include "MaterialSimulation.h"
#include <memory>

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	bool m_bSimulationDialogVisible;	
	//the cutting simulation of the active document, alive from Start to Stop; driven by a UI timer
	//(one thread in the application: nothing is shared with a worker)
	std::unique_ptr<kin5ax::CMaterialSimulation> m_pSimulation;
	UINT m_nSimulationTimerInterval;	//the interval the timer runs with, re-armed when the setting changes
	size_t m_nStepsSinceRepaint;
	//a timer tick: one location (or a few when the interval is below the timer resolution), a repaint
	//every refresh period, the end of the trajectory
	void RunSimulationSteps();

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileNew();
protected:
	void UpdateShowHideMenu();

	// main dialog with actual chain properties
	CKinematicChainPropertiesDialog m_dlgChainProperties;

	CSingleInputSolverDialog m_dlgSingleInputDialog;
	CMaterialBlockDialog m_dlgMaterialBlockDialog;
	CCLDataFileDialog m_dlgCLDataFileDialog;
	CSimulationDialog m_dlgSimulationDialog;
public:
	afx_msg void OnPostprocessorSingleinput();
	afx_msg void OnChainProperties();
	void UpdateDialogs(void);
	afx_msg void OnMaterialblockGenerateinitialblock();
	afx_msg void OnSimulationSetupmillingtool();
	afx_msg void OnShowMaterial();
	afx_msg void OnShowMaterialContour();
	afx_msg void OnShowMaterialFrame();
	afx_msg void OnShowTool();
	afx_msg void OnShowToolFrame();
	afx_msg void OnShowLinks();
	afx_msg void OnShowLinksFrames();
	afx_msg void OnShowTooltrajectory();
	afx_msg void OnShowMaterialHolder();
	afx_msg void On3dviewFixedMaterialFrame();
	afx_msg void OnShowBase();
	afx_msg void OnShowBaseFrame();
	afx_msg void OnSettingsProgramsettings();
	afx_msg void OnPostprocessorCldatafile();
	bool StartSimulation(void);
	void PauseSimulation(void);
	void StopAndResetSimulation(void);
	afx_msg void OnSimulationDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnShowLinksParts();
	afx_msg void OnShowToolBoundingBox();
};


