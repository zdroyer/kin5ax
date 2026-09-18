// MaterialBlockDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "MaterialBlockDialog.h"
#include ".\materialblockdialog.h"


extern CApplicationConfiguration oConfiguration;

// CMaterialBlockDialog dialog

IMPLEMENT_DYNAMIC(CMaterialBlockDialog, CDialog)
CMaterialBlockDialog::CMaterialBlockDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMaterialBlockDialog::IDD, pParent)
	, m_dVoxelSize(0.02)
	, m_dSizeX(0.5)
	, m_dSizeY(0.5)
	, m_dSizeZ(0.3)
	, m_dPositionX(-0.25)
	, m_dPositionY(-0.25)
	, m_dPositionZ(0.0)
{
}

CMaterialBlockDialog::~CMaterialBlockDialog()
{
}

void CMaterialBlockDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_VOXEL_SIZE, m_dVoxelSize);
	DDX_Text(pDX, IDC_EDIT_SIZE_X, m_dSizeX);
	DDX_Text(pDX, IDC_EDIT_X, m_dPositionX);
	DDX_Text(pDX, IDC_EDIT_SIZE_Y, m_dSizeY);
	DDX_Text(pDX, IDC_EDIT_Y, m_dPositionY);
	DDX_Text(pDX, IDC_EDIT_SIZE_Z, m_dSizeZ);
	DDX_Text(pDX, IDC_EDIT_Z, m_dPositionZ);
	DDX_Control(pDX, IDC_EDIT_MEMORY_USAGE, m_edtMemoryUsage);
}


BEGIN_MESSAGE_MAP(CMaterialBlockDialog, CDialog)
	ON_EN_CHANGE(IDC_EDIT_SIZE_X, OnEnChangeEditSizeX)
	ON_EN_CHANGE(IDC_EDIT_SIZE_Y, OnEnChangeEditSizeY)
	ON_EN_CHANGE(IDC_EDIT_SIZE_Z, OnEnChangeEditSizeZ)
	ON_EN_CHANGE(IDC_EDIT_VOXEL_SIZE, OnEnChangeEditVoxelSize)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CMaterialBlockDialog message handlers

void CMaterialBlockDialog::OnEnChangeEditSizeX()
{
UpdateMemoryUsage();
}

void CMaterialBlockDialog::OnEnChangeEditSizeY()
{
UpdateMemoryUsage();
}

void CMaterialBlockDialog::OnEnChangeEditSizeZ()
{
	UpdateMemoryUsage();
}

void CMaterialBlockDialog::OnEnChangeEditVoxelSize()
{
	UpdateMemoryUsage();
}

void CMaterialBlockDialog::OnBnClickedOk()
{
	UpdateData(TRUE);
	CString sMainTitle;
	GetParent()->GetWindowText(sMainTitle);
	GetParent()->SetWindowText("Generating material block ...");
	m_pDoc->m_oMaterial.GenerateBox(m_dSizeX,m_dSizeY,m_dSizeZ,m_dVoxelSize,m_dPositionX,m_dPositionY,m_dPositionZ);
	double dMax = max(m_dSizeX,m_dSizeY);
	oConfiguration.m_dUnit = dMax/0.9;
	GetParent()->SetWindowText(sMainTitle);

	OnOK();
}
void CMaterialBlockDialog::SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc *pDoc)
{
	assert(pDoc);
	m_pDoc = pDoc;
	m_pDoc->m_oMaterial.GetParameters
				(m_dSizeX
				,m_dSizeY
				,m_dSizeZ
				,m_dVoxelSize
				,m_dPositionX
				,m_dPositionY
				,m_dPositionZ);
	UpdateData(FALSE);    
	UpdateMemoryUsage();
}

void CMaterialBlockDialog::UpdateMemoryUsage(void)
{
	UpdateData(TRUE);
	if(m_dVoxelSize==0)
		return;
	double dX = (int)(m_dSizeX / m_dVoxelSize)+1;
	double dY = (int)(m_dSizeY / m_dVoxelSize)+1;
	double dZ = (int)(m_dSizeZ / m_dVoxelSize)+1;
	double dMemUsage = dX/1024.0 * dY /1024.0 * dZ/4.0;
	static char str[255];
	if(dMemUsage<0.1)
		sprintf(str,"<0.1");
	else
		sprintf(str,"%2.1f",dMemUsage);
	m_edtMemoryUsage.SetWindowText(str);
}
