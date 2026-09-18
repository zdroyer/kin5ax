// CLDataFileDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include ".\postprocessingdialog.h"
#include "cutterlocationdata.h"
#include ".\cldatafiledialog.h"


// CCLDataFileDialog dialog

IMPLEMENT_DYNAMIC(CCLDataFileDialog, CDialog)
CCLDataFileDialog::CCLDataFileDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCLDataFileDialog::IDD, pParent)
	, m_pDoc(0)
	, m_dScale(1.0)
	, m_dAngleDegrees(0.0)
	, m_dAxisX(1.0)
	, m_dAxisY(0.0)
	, m_dAxisZ(0.0)
	, m_dOffsetX(0.0)
	, m_dOffsetY(0.0)
	, m_dOffsetZ(0.0)
	, m_sOriginalDialogTitle("CL Data file processing")
	, m_bShowOnlyErrors(FALSE)
	, m_bTurnAlsoPosition(TRUE)
	, m_bBoxTransformed(FALSE)
	, m_bInvertOrientation(FALSE)
{

}

CCLDataFileDialog::~CCLDataFileDialog()
{
}

void CCLDataFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_SCALE, m_dScale);
	DDX_Text(pDX, IDC_EDIT_TURN_ANGLE, m_dAngleDegrees);
	DDX_Text(pDX, IDC_EDIT_TURN_AXIS_X, m_dAxisX);
	DDX_Text(pDX, IDC_EDIT_TURN_AXIS_Y, m_dAxisY);
	DDX_Text(pDX, IDC_EDIT_TURN_AXIS_Z, m_dAxisZ);
	DDX_Text(pDX, IDC_EDIT_X_OFFSET, m_dOffsetX);
	DDX_Text(pDX, IDC_EDIT_Y_OFFSET, m_dOffsetY);
	DDX_Text(pDX, IDC_EDIT_Z_OFFSET, m_dOffsetZ);

	DDX_Control(pDX, IDC_EDIT_P, m_editP);
	DDX_Control(pDX, IDC_EDIT_V, m_editV);
	DDX_Control(pDX, IDC_EDIT_LINE_NUMBER, m_editLineNumber);
	DDX_Control(pDX, IDC_EDIT_LINE_STATUS, m_editLineStatus);

	DDX_Control(pDX, IDC_EDIT_P_PRIM, m_editPPrim);
	DDX_Control(pDX, IDC_EDIT_V_PRIM, m_editVPrim);
	DDX_Control(pDX, IDC_LIST_FILE_CONTENTS, m_listboxFileContents);
	DDX_Radio(pDX, IDC_RADIO_SHOW_ALL_LINES, m_bShowOnlyErrors);
	DDX_Check(pDX, IDC_CHECK_TURN_ALSO_POSITION, m_bTurnAlsoPosition);
	DDX_Control(pDX, IDC_EDIT_X_MAX, m_editXMax);
	DDX_Control(pDX, IDC_EDIT_Y_MAX, m_editYMax);
	DDX_Control(pDX, IDC_EDIT_Z_MAX, m_editZMax);
	DDX_Control(pDX, IDC_EDIT_X_MIN, m_editXMin);
	DDX_Control(pDX, IDC_EDIT_Y_MIN, m_editYMin);
	DDX_Control(pDX, IDC_EDIT_Z_MIN, m_editZMin);
	DDX_Control(pDX, IDC_EDIT_LINES_COUNT, m_editLinesCount);
	DDX_Control(pDX, IDC_EDIT_ERRORS_COUNT, m_editErrorsCount);
	DDX_Radio(pDX, IDC_RADIO_P, m_bBoxTransformed);
	DDX_Check(pDX, IDC_CHECK_INVERT_ORIENTATION, m_bInvertOrientation);
}


BEGIN_MESSAGE_MAP(CCLDataFileDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CHOOSE_FILE, OnBnClickedButtonChooseFile)
	ON_BN_CLICKED(IDC_BUTTON_RELOAD, OnBnClickedButtonReload)
	ON_BN_CLICKED(IDC_RADIO_SHOW_ALL_LINES, OnBnClickedRadioShowAllLines)
	ON_BN_CLICKED(IDC_RADIO_SHOW_LINES_WITH_ERROR, OnBnClickedRadioShowLinesWithError)
	ON_LBN_SELCHANGE(IDC_LIST_FILE_CONTENTS, OnLbnSelchangeListFileContents)
	ON_EN_CHANGE(IDC_EDIT_X_OFFSET, OnEnChangeEditXOffset)
	ON_EN_CHANGE(IDC_EDIT_Y_OFFSET, OnEnChangeEditYOffset)
	ON_EN_CHANGE(IDC_EDIT_Z_OFFSET, OnEnChangeEditZOffset)
	ON_EN_CHANGE(IDC_EDIT_TURN_AXIS_X, OnEnChangeEditTurnAxisX)
	ON_EN_CHANGE(IDC_EDIT_TURN_AXIS_Y, OnEnChangeEditTurnAxisY)
	ON_EN_CHANGE(IDC_EDIT_TURN_AXIS_Z, OnEnChangeEditTurnAxisZ)
	ON_EN_CHANGE(IDC_EDIT_TURN_ANGLE, OnEnChangeEditTurnAngle)
	ON_BN_CLICKED(IDC_CHECK_TURN_ALSO_POSITION, OnBnClickedCheckTurnAlsoPosition)
	ON_EN_CHANGE(IDC_EDIT_SCALE, OnEnChangeEditScale)
	ON_BN_CLICKED(IDC_RADIO_P, OnBnClickedRadioP)
	ON_BN_CLICKED(IDC_RADIO_P_PRIM, OnBnClickedRadioPPrim)
	ON_BN_CLICKED(IDC_BUTTON_PROCESS, OnBnClickedButtonProcess)
	ON_BN_CLICKED(IDC_CHECK_INVERT_ORIENTATION, OnBnClickedCheckInvertOrientation)
END_MESSAGE_MAP()


// CCLDataFileDialog message handlers

void CCLDataFileDialog::SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc *pDoc)
{
	m_pDoc = pDoc;
}
void CCLDataFileDialog::UpdateDialogTitle()
{
	SetWindowText(m_sOriginalDialogTitle+" - "+m_sOpenedFileTitle);
}
void CCLDataFileDialog::OnBnClickedButtonChooseFile()
{

	CFileDialog dlgOpenFile(1,"cld",0,OFN_HIDEREADONLY,"Cutter location data file (*.cld)|*.cld|All Files (*.*)|*.*||");
	if(IDOK==dlgOpenFile.DoModal()){
		m_sOpenedFileTitle = dlgOpenFile.GetFileTitle();
		LoadCLFile(dlgOpenFile.GetPathName(),m_sOpenedFileTitle);

	}

}

void CCLDataFileDialog::OnBnClickedButtonReload()
{
	if(m_sOpenedFileTitle.GetLength())
		LoadCLFile(m_sOpenedFilePath,m_sOpenedFileTitle);
}

void CCLDataFileDialog::LoadCLFile(const CString &sFilePath,const CString &sFileTitle )
{
	UpdateCutterLocationDataObject();
	if(m_pDoc) {
		m_sOpenedFilePath = sFilePath;
		UpdateDialogTitle();
		try{
			m_pDoc->m_oCutterLocationData.LoadFile(m_sOpenedFilePath.GetBuffer(m_sOpenedFilePath.GetLength()),m_sOpenedFileTitle.GetBuffer(m_sOpenedFileTitle.GetLength()));				
			m_pDoc->m_oCutterLocationData.FillListBox(m_listboxFileContents,m_bShowOnlyErrors);
		} catch (CGeneratorException e){
			MessageBox(e.GetMessage().c_str());
		}

		CString s;
		s.Format("%d",m_pDoc->m_oCutterLocationData.GetErrorsCount());
		m_editErrorsCount.SetWindowText(s.GetBuffer(s.GetLength()));
		s.Format("%d",m_pDoc->m_oCutterLocationData.GetLinesCount());
		m_editLinesCount.SetWindowText(s.GetBuffer(s.GetLength()));
		UpdateBoundingBoxControls();
		m_pDoc->UpdateAllViews(NULL);
	}
}


void CCLDataFileDialog::UpdateCutterLocationDataObject()
{
	assert(m_pDoc);
	if(m_pDoc){
		UpdateData();
		CTransformation trP = (CScale(m_dScale)*CTranslation(m_dOffsetX,m_dOffsetY,m_dOffsetZ));
		CTransformation trV = (CRotation(CR3Vector(m_dAxisX,m_dAxisY,m_dAxisZ),m_dAngleDegrees,0.0));
		if(m_bTurnAlsoPosition)
			trP= trV * trP;
		m_pDoc->m_oCutterLocationData.SetTransformation(trP,trV,m_bInvertOrientation);
		UpdateBoundingBoxControls();
		OnLbnSelchangeListFileContents();
		m_pDoc->UpdateAllViews(NULL);
	}
}

void CCLDataFileDialog::OnBnClickedRadioShowAllLines()
{
	assert(m_pDoc);
	if(m_pDoc){
		UpdateData();
		m_pDoc->m_oCutterLocationData.FillListBox(m_listboxFileContents,m_bShowOnlyErrors);
		m_listboxFileContents.Invalidate();
	}
}

void CCLDataFileDialog::OnBnClickedRadioShowLinesWithError()
{
	assert(m_pDoc);
	if(m_pDoc){
		UpdateData();
		m_pDoc->m_oCutterLocationData.FillListBox(m_listboxFileContents,m_bShowOnlyErrors);
		m_listboxFileContents.Invalidate();
	}
}

void CCLDataFileDialog::OnLbnSelchangeListFileContents()
{
	assert(m_pDoc);
	if(m_pDoc){
		int index = m_listboxFileContents.GetCurSel();
		size_t nLineNo = (size_t)m_listboxFileContents.GetItemData(index);
		if((size_t)m_pDoc->m_oCutterLocationData.GetLinesCount()<= nLineNo)
			return;
		const CCutterLocationData::CCLLine line = m_pDoc->m_oCutterLocationData.GetLine(nLineNo);
		CString s;
		s.Format("%d",nLineNo);
		m_editLineNumber.SetWindowText(s.GetBuffer(s.GetLength()));
		m_editLineStatus.SetWindowText(line.GetStatus().c_str());
		if(line.m_status==CL_LINE_OK){
			m_editP.SetWindowText(line.m_p.toString().c_str());
			m_editV.SetWindowText(line.m_v.toString().c_str());
			CR3Point p;
			CR3Vector v;
			m_pDoc->m_oCutterLocationData.GetLocation(p,v,nLineNo);
			m_editPPrim.SetWindowText(p.toString().c_str());
			m_editVPrim.SetWindowText(v.toString().c_str());
		} else {
			static char* sNoData="NO DATA";
			m_editP.SetWindowText(sNoData);
			m_editV.SetWindowText(sNoData);
			m_editPPrim.SetWindowText(sNoData);
			m_editVPrim.SetWindowText(sNoData);
		}


	}
	// TODO: Add your control notification handler code here
}

void CCLDataFileDialog::OnEnChangeEditXOffset()
{
	UpdateCutterLocationDataObject();
	UpdateBoundingBoxControls();
}

void CCLDataFileDialog::OnEnChangeEditYOffset()
{
	UpdateCutterLocationDataObject();
	UpdateBoundingBoxControls();
}

void CCLDataFileDialog::OnEnChangeEditZOffset()
{
	UpdateCutterLocationDataObject();
	UpdateBoundingBoxControls();
}

void CCLDataFileDialog::OnEnChangeEditTurnAxisX()
{
	UpdateCutterLocationDataObject();
	UpdateBoundingBoxControls();
}

void CCLDataFileDialog::OnEnChangeEditTurnAxisY()
{
	UpdateCutterLocationDataObject();
	UpdateBoundingBoxControls();
}

void CCLDataFileDialog::OnEnChangeEditTurnAxisZ()
{
	UpdateCutterLocationDataObject();
	UpdateBoundingBoxControls();
}

void CCLDataFileDialog::OnEnChangeEditTurnAngle()
{
	UpdateCutterLocationDataObject();
	UpdateBoundingBoxControls();
}

void CCLDataFileDialog::OnBnClickedCheckTurnAlsoPosition()
{
	UpdateCutterLocationDataObject();
	UpdateBoundingBoxControls();
}

void CCLDataFileDialog::OnEnChangeEditScale()
{
	UpdateCutterLocationDataObject();
	UpdateBoundingBoxControls();
}


void CCLDataFileDialog::UpdateBoundingBoxControls(void)
{
	UpdateData();
	CR3Box box = m_pDoc->m_oCutterLocationData.GetBoundingBox(m_bBoxTransformed);
	SetText(m_editXMax,box.GetMaxPoint().GetX());
	SetText(m_editYMax,box.GetMaxPoint().GetY());
	SetText(m_editZMax,box.GetMaxPoint().GetZ());
	SetText(m_editXMin,box.GetMinPoint().GetX());
	SetText(m_editYMin,box.GetMinPoint().GetY());
	SetText(m_editZMin,box.GetMinPoint().GetZ());
}

void CCLDataFileDialog::SetText(CWnd & window, double d)
{
	CString s;
	s.Format("%lf",d);
	window.SetWindowText(s.GetBuffer(s.GetLength()));
}

void CCLDataFileDialog::OnBnClickedRadioP()
{
	UpdateBoundingBoxControls();
}

void CCLDataFileDialog::OnBnClickedRadioPPrim()
{
	UpdateBoundingBoxControls();
}


void CCLDataFileDialog::OnBnClickedButtonProcess()
{
	assert(m_pDoc);
	if(m_pDoc){
		if(m_pDoc->m_oChain.Is5CChain()){
			CPostprocessingDialog dlg(this->GetParent());
			dlg.SetActiveDocument(m_pDoc);
			dlg.DoModal();
		} else {
			MessageBox("Actual kinematic chain model is not valid 5C chain\ncorrect structure and try again");
		}
	}
}

void CCLDataFileDialog::OnBnClickedCheckInvertOrientation()
{
	UpdateCutterLocationDataObject();
}
