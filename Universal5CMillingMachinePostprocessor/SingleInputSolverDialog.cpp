// SingleInputSolverDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "SingleInputSolverDialog.h"
#include ".\singleinputsolverdialog.h"


// CSingleInputSolverDialog dialog

IMPLEMENT_DYNAMIC(CSingleInputSolverDialog, CDialog)
CSingleInputSolverDialog::CSingleInputSolverDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSingleInputSolverDialog::IDD, pParent)
	, m_nSolutionIndex(0)
	, m_pDoc(0)
	, m_dX(0.0)
	, m_dY(0.0)
	, m_dZ(0.0)
	, m_dI(1.0)
	, m_dJ(0.0)
	, m_dK(0.0)
	, m_dXComputed(0)
	, m_dYComputed(0)
	, m_dZComputed(0)
	, m_dIComputed(0)
	, m_dJComputed(0)
	, m_dKComputed(0)
	, m_dPositionError(0)
	, m_dOrientationError(0)
{
}

CSingleInputSolverDialog::~CSingleInputSolverDialog()
{
}

void CSingleInputSolverDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_LBIndex(pDX, IDC_LIST_SOLUTIONS, m_nSolutionIndex);
	DDX_Control(pDX, IDC_LIST_SOLUTIONS, m_listboxSolutions);
	DDX_Text(pDX, IDC_EDIT_X, m_dX);
	DDX_Text(pDX, IDC_EDIT_Y, m_dY);
	DDX_Text(pDX, IDC_EDIT_Z, m_dZ);
	DDX_Text(pDX, IDC_EDIT_I, m_dI);
	DDX_Text(pDX, IDC_EDIT_J, m_dJ);
	DDX_Text(pDX, IDC_EDIT_K, m_dK);
	DDX_Text(pDX, IDC_EDIT_X_COMPUTED, m_dXComputed);
	DDX_Text(pDX, IDC_EDIT_Y_COMPUTED, m_dYComputed);
	DDX_Text(pDX, IDC_EDIT_Z_COMPUTED, m_dZComputed);
	DDX_Text(pDX, IDC_EDIT_I_COMPUTED, m_dIComputed);
	DDX_Text(pDX, IDC_EDIT_J_COMPUTED, m_dJComputed);
	DDX_Text(pDX, IDC_EDIT_K_COMPUTED, m_dKComputed);
	DDX_Text(pDX, IDC_EDIT_POSITION_ERROR, m_dPositionError);
	DDX_Text(pDX, IDC_EDIT_ORIENTATION_ERROR, m_dOrientationError);
}


BEGIN_MESSAGE_MAP(CSingleInputSolverDialog, CDialog)
	ON_BN_CLICKED(ID_BTN_SOLVE, OnBnClickedBtnSolve)
	ON_LBN_SELCHANGE(IDC_LIST_SOLUTIONS, OnLbnSelchangeListSolutions)
	ON_LBN_DBLCLK(IDC_LIST_SOLUTIONS, OnLbnDblclkListSolutions)
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(ID_BTN_SOLVE2, OnBnClickedBtnSolve2)
	ON_EN_CHANGE(IDC_EDIT_Z, OnEnChangeEditZ)
	ON_EN_CHANGE(IDC_EDIT_Y, OnEnChangeEditY)
	ON_EN_CHANGE(IDC_EDIT_X, OnEnChangeEditX)
	ON_EN_CHANGE(IDC_EDIT_I, OnEnChangeEditI)
	ON_EN_CHANGE(IDC_EDIT_J, OnEnChangeEditJ)
	ON_EN_CHANGE(IDC_EDIT_K, OnEnChangeEditK)
END_MESSAGE_MAP()


// CSingleInputSolverDialog message handlers

void CSingleInputSolverDialog::OnBnClickedBtnSolve()
{
	assert(m_pDoc);
	if(m_pDoc==0)
		return;
	UpdateData();
	try{
		CR3Point r3pP0(m_dX,m_dY,m_dZ);
		CR3Vector r3vV0(m_dI,m_dJ,m_dK);
		if(r3vV0.IsZero())
			MY_THROW(CArgsMyException,"Zero-length orientation vector");
		m_pDoc->m_oChain.SetVirtualToolPosition(r3vV0,r3pP0);
		string sLog;
		m_vConfigurations = m_pDoc->GetPostprocessor()->SolveWithLocations(r3pP0,r3vV0,m_pDoc->m_oChain,*m_pDoc->m_pMillingTool,sLog);
		m_listboxSolutions.ResetContent();
		if(m_vConfigurations.size()==0)
			m_listboxSolutions.AddString("NO SOLUTION");
		else {			
			for(size_t i=0; i<m_vConfigurations.size();i++){
				m_listboxSolutions.AddString(m_vConfigurations[i].first.toString().c_str());
			}
		}

		m_pDoc->UpdateAllViews(NULL);
		MessageBox(sLog.c_str(),"Solver log");
	} catch (CGeneratorException e){
		AfxMessageBox(e.Message().c_str());
	}
	
}

void CSingleInputSolverDialog::SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc * pDocument)
{
	m_pDoc = pDocument;
	if(m_pDoc)
		m_pDoc->UpdateAllViews(NULL);
}

void CSingleInputSolverDialog::OnLbnSelchangeListSolutions()
{

}

void CSingleInputSolverDialog::OnLbnDblclkListSolutions()
{
	UpdateData();
	if((size_t)m_nSolutionIndex < m_vConfigurations.size()
		&& (size_t)m_nSolutionIndex >= 0){
		assert(m_pDoc);
		if(!m_pDoc)
			return;
		m_pDoc->m_oChain.SetParameters(m_vConfigurations[m_nSolutionIndex].first);
		m_vConfigurations[m_nSolutionIndex].second.m_v.CopyCoordinates(m_dIComputed,m_dJComputed,m_dKComputed);
		m_vConfigurations[m_nSolutionIndex].second.m_p.CopyCoordinates(m_dXComputed,m_dYComputed,m_dZComputed);
		m_dOrientationError = radians_to_degrees(m_vConfigurations[m_nSolutionIndex].second.m_v.UndirectedAngle(CR3Vector(m_dI,m_dJ,m_dK)));
		m_dPositionError = d_E(m_vConfigurations[m_nSolutionIndex].second.m_p,CR3Point(m_dX,m_dY,m_dZ));


		UpdateData(FALSE);
		m_pDoc->UpdateAllViews(NULL);
		//AfxMessageBox(m_vConfigurtions[m_nSolutionIndex].toString().c_str());
	}
}

void CSingleInputSolverDialog::OnBnClickedBtnSolve2()
{
	if(!m_pDoc)
		return;
	m_pDoc->m_oChain.ZeroAllParameters();
	m_pDoc->UpdateAllViews(NULL);
}

void CSingleInputSolverDialog::UpdateVirtualToolPosition()
{
	if(!m_pDoc)
		return;
	UpdateData(TRUE);
	m_pDoc->m_oChain.SetVirtualToolPosition(CR3Vector(m_dI,m_dJ,m_dK),CR3Point(m_dX,m_dY,m_dZ));
	m_pDoc->UpdateAllViews(NULL);
}
void CSingleInputSolverDialog::OnEnChangeEditZ()
{
	UpdateVirtualToolPosition();
}

void CSingleInputSolverDialog::OnEnChangeEditY()
{
	UpdateVirtualToolPosition();
}

void CSingleInputSolverDialog::OnEnChangeEditX()
{
	UpdateVirtualToolPosition();
}

void CSingleInputSolverDialog::OnEnChangeEditI()
{
	UpdateVirtualToolPosition();
}

void CSingleInputSolverDialog::OnEnChangeEditJ()
{
	UpdateVirtualToolPosition();
}

void CSingleInputSolverDialog::OnEnChangeEditK()
{
	UpdateVirtualToolPosition();
}
