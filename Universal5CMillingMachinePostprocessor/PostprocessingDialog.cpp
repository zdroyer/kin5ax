// PostprocessingDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "postprocessingdialog.h"
#include "settingsdialog.h"
#include "string_tools.h"
#include "ProgressCtrlCallback.h"
#include "CLDataPostprocessor.h"
#include ".\postprocessingdialog.h"
// CPostprocessingDialog dialog

extern CPostprocessorSettings oPostprocessorSettings;

IMPLEMENT_DYNAMIC(CPostprocessingDialog, CDialog)
CPostprocessingDialog::CPostprocessingDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPostprocessingDialog::IDD, pParent)
	, m_pDoc(0)
	, m_dPositionDifference(oPostprocessorSettings.m_dMaxPositionDifference)
	, m_dOrientationDifference(oPostprocessorSettings.m_dMaxOrientationDifferenceDegrees)
	, m_bDetailedReport(FALSE)
	, m_nXNameIndex(0)
	, m_nYNameIndex(1)
	, m_nZNameIndex(2)
	, m_nAngle1NameIndex(0)
	, m_nAngle2NameIndex(1)
	, m_bRadians(FALSE)
	, m_sAngle1GCodeName(_T(""))
	, m_sAngle2GCodeName(_T(""))
	, m_nDetailedReportContents(0)
{
}

CPostprocessingDialog::~CPostprocessingDialog()
{
}

void CPostprocessingDialog::SetActiveDocument(CUniversal5CMillingMachinePostprocessorDoc *pDoc)
{
	m_pDoc = pDoc;
}
void CPostprocessingDialog::SetText(CWnd & window, double d)
{
	CString s;
	s.Format("%lf",d);
	window.SetWindowText(s.GetBuffer(1));
	//window.SetWindowText(s.GetString());
}
void CPostprocessingDialog::SetText(CWnd & window, size_t n)
{
	CString s;
	//sprintf(s,9,"%d",n);
	s.Format("%zu",n);
	window.SetWindowText(s.GetBuffer(1));
}

void CPostprocessingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOCATIONS_COUNT, m_editLocationsCount);
	DDX_Control(pDX, IDC_EDIT_ORIENTATION_CHANGE, m_editTotalOrientationChange);
	DDX_Control(pDX, IDC_EDIT_TRAJECTORY_LENGTH, m_editTrajectoryLength);
	DDX_Control(pDX, IDC_EDIT_NUMBER_OF_PROBLEMS, m_editProblemsCount);

	DDX_Text(pDX, IDC_EDIT_POSITION_DIFFERENCE, m_dPositionDifference);
	DDX_Text(pDX, IDC_EDIT_ORIENTATION_DIFFERENCE, m_dOrientationDifference);
	DDX_Control(pDX, IDC_PROGRESS_POSTPROCESSING, m_progressPostprocessing);
	DDX_Control(pDX, IDC_EDIT_REPORT, m_editReport);
	DDX_Control(pDX, IDC_BUTTON_START, m_buttonStart);
	DDX_Control(pDX, IDC_COMBO_X_NAME, m_comboXName);
	DDX_Control(pDX, IDC_COMBO_Y_NAME, m_comboYName);
	DDX_Control(pDX, IDC_COMBO_Z_NAME, m_comboZName);

	DDX_Control(pDX, IDC_COMBO_ANGLE1_NAME, m_comboAngle1Name);
	DDX_Control(pDX, IDC_COMBO_ANGLE2_NAME, m_comboAngle2Name);
	DDX_CBIndex(pDX, IDC_COMBO_X_NAME, m_nXNameIndex);
	DDX_CBIndex(pDX, IDC_COMBO_Y_NAME, m_nYNameIndex);
	DDX_CBIndex(pDX, IDC_COMBO_Z_NAME, m_nZNameIndex);
	DDX_CBIndex(pDX, IDC_COMBO_ANGLE1_NAME, m_nAngle1NameIndex);
	DDX_CBIndex(pDX, IDC_COMBO_ANGLE2_NAME, m_nAngle2NameIndex);

	DDX_Radio(pDX, IDC_RADIO_ANGLE_UNIT_RADIANS1, m_bRadians);
	DDX_CBString(pDX, IDC_COMBO_ANGLE1_CHAR, m_sAngle1GCodeName);
	DDX_CBString(pDX, IDC_COMBO_ANGLE2_CHAR, m_sAngle2GCodeName);
	DDX_Control(pDX, IDC_BUTTON_SAVE_GCODE, m_buttonSaveGCode);
	DDX_Control(pDX, IDC_COMBO_ANGLE1_CHAR, m_comboAngle1Char);
	DDX_Control(pDX, IDC_COMBO_ANGLE2_CHAR, m_comboAngle2Char);
	DDX_Control(pDX, IDC_RADIO_DETAILED_REPORT_CONTENTS0, m_radioDetailedReportContents0);
	DDX_Control(pDX, IDC_RADIO_DETAILED_REPORT_CONTENTS1, m_radioDetailedReportContents1);
	DDX_Control(pDX, IDC_RADIO_DETAILED_REPORT_CONTENTS2, m_radioDetailedReportContents2);
	DDX_Radio(pDX, IDC_RADIO_DETAILED_REPORT_CONTENTS0, m_nDetailedReportContents);
	DDX_Check(pDX, IDC_CHECK_DETAILED_REPORT, m_bDetailedReport);
	DDX_Control(pDX, IDC_EDIT_DETAILED_REPORT_FILE_NAME, m_editDetailedReportFileName);
	DDX_Control(pDX, IDC_BUTTON_BROWSE_FOR_DETAILED_REPORT_FILE_NAME, m_buttonBrowseForDetailedReportFileName);
}


BEGIN_MESSAGE_MAP(CPostprocessingDialog, CDialog)
	ON_WM_SHOWWINDOW()
	ON_EN_CHANGE(IDC_EDIT_ORIENTATION_DIFFERENCE, OnEnChangeEditOrientationDifference)
	ON_EN_CHANGE(IDC_EDIT_POSITION_DIFFERENCE, OnEnChangeEditPositionDifference)
	ON_BN_CLICKED(IDC_BUTTON_START, OnBnClickedStart)
	ON_BN_CLICKED(IDC_BUTTON_PROGRAM_SETTINGS, OnBnClickedButtonProgramSettings)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_GCODE, OnBnClickedButtonSaveGcode)
	ON_BN_CLICKED(IDC_CHECK_DETAILED_REPORT, OnBnClickedCheckDetailedReport)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FOR_DETAILED_REPORT_FILE_NAME, OnBnClickedButtonBrowseForDetailedReportFileName)
END_MESSAGE_MAP()


// CPostprocessingDialog message handlers

void CPostprocessingDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	if(!bShow)
		return;
	assert(m_pDoc);
	if(m_pDoc){
		SetText(m_editLocationsCount,m_pDoc->m_oCutterLocationData.GetLocationsCount());
		SetText(m_editTotalOrientationChange,radians_to_degrees( m_pDoc->m_oCutterLocationData.GetTotalOrientationChangeRadians()));
		SetText(m_editTrajectoryLength,m_pDoc->m_oCutterLocationData.GetTrajectoryLength());
		UpdateProblemsCountControl();
		m_vSolutions.reserve(m_pDoc->m_oCutterLocationData.GetLocationsCount());
		m_pDoc->m_oSimulationData.ReserveMemory(m_pDoc->m_oCutterLocationData.GetLocationsCount());
	}
}

void CPostprocessingDialog::OnEnChangeEditOrientationDifference()
{
	UpdateData();
	oPostprocessorSettings.m_dMaxOrientationDifferenceDegrees = fabs(m_dOrientationDifference);
	oPostprocessorSettings.m_dMaxOrientationDifferenceRadians = degrees_to_radians(oPostprocessorSettings.m_dMaxOrientationDifferenceDegrees);
	UpdateProblemsCountControl();
}

void CPostprocessingDialog::OnEnChangeEditPositionDifference()
{
	UpdateData();
	oPostprocessorSettings.m_dMaxPositionDifference = fabs(m_dPositionDifference);
	UpdateProblemsCountControl();
}

void CPostprocessingDialog::UpdateProblemsCountControl()
{
	assert(m_pDoc);
	if(m_pDoc){
		//an estimate: the average move between two locations, divided as the interpolation limits demand
		double dLocationsCount = (double) m_pDoc->m_oCutterLocationData.GetLocationsCount();
		double dProblemsCount = 0.0;
		if(dLocationsCount>0.0){
			double dAvgDistance = m_pDoc->m_oCutterLocationData.GetTrajectoryLength()/dLocationsCount;
			double dAvgAngle = m_pDoc->m_oCutterLocationData.GetTotalOrientationChangeRadians()/dLocationsCount;
			dProblemsCount = dLocationsCount*oPostprocessorSettings.InterpolationSteps(dAvgDistance,dAvgAngle);
		}
		SetText(m_editProblemsCount,(size_t)(1.2*dProblemsCount));
		if(dProblemsCount<1E7){	//an estimate from a limit typed half way is no reason to grab the memory
			m_vSolutions.reserve(size_t(1.2*dProblemsCount));
			m_pDoc->m_oSimulationData.ReserveMemory(size_t(1.2*dProblemsCount));
		}
	}
}

void CPostprocessingDialog::OnBnClickedStart()
{
	assert(m_pDoc);
	if(!m_pDoc)
		return;
	if(!m_pDoc->m_oChain.Is5CChain()){
		MessageBox("Actual model is not valid 5C kinematic chain");
		return;
	}
	if(!m_pDoc->m_pMillingTool){
		MessageBox("No milling tool defined - set up the milling tool first");
		return;
	}
		
	m_buttonSaveGCode.EnableWindow(FALSE);

	m_buttonStart.EnableWindow(FALSE);
	m_progressPostprocessing.SetRange(0,100);
	m_progressPostprocessing.SetPos(0);

	m_dActualMaxOrientationDifferenceDegrees = oPostprocessorSettings.m_dMaxOrientationDifferenceDegrees;
	m_dActualMaxPositionDifference = oPostprocessorSettings.m_dMaxPositionDifference;
	m_nTotalProblemsCount=0;

	m_dMaxOrientationErrorVectorLength=-1.0;
	m_dMaxPositionError=-1.0;
	

	FILE *pFile = 0;
	if(m_bDetailedReport){
		pFile = fopen(m_sReportFile.c_str(),"wt");
		if(!pFile){
			MessageBox(("Cannot open file: "+m_sReportFile+" for writing").c_str());
			return;
		}
		fputs(GenerateReportHead().c_str(),pFile);
	}
	
	m_vsProblemsLogs.erase(m_vsProblemsLogs.begin(),m_vsProblemsLogs.end());
	m_editReport.SetWindowText("");
	m_vSolutions.erase(m_vSolutions.begin(),m_vSolutions.end());
	SetWindowText("1/4 postprocessing in progress...");
	m_timeBegin = m_timeBegin.GetCurrentTime();
	try{
		//the postprocessing itself is done by the kin5ax library
		CProgressCtrlCallback oPostprocessingProgress(m_progressPostprocessing);
		CCLDataPostprocessor oCLDataPostprocessor(oPostprocessorSettings);
		CCLDataPostprocessingResult oResult = oCLDataPostprocessor.Process
			(m_pDoc->m_oChain
			,*m_pDoc->m_pMillingTool
			,m_pDoc->m_oCutterLocationData
			,m_pDoc->m_oSimulationData
			,pFile
			,m_nDetailedReportContents
			,&oPostprocessingProgress);
		m_nTotalProblemsCount = oResult.m_nTotalProblemsCount;
		m_dMaxPositionError = oResult.m_dMaxPositionError;
		m_oMaxPositionErrorSolution = oResult.m_oMaxPositionErrorSolution;
		m_cvMaxPositionErrorInput = oResult.m_cvMaxPositionErrorInput;
		m_dMaxOrientationErrorVectorLength = oResult.m_dMaxOrientationErrorVectorLength;
		m_oMaxOrientationErrorSolution = oResult.m_oMaxOrientationErrorSolution;
		m_cvMaxOrientationErrorInput = oResult.m_cvMaxOrientationErrorInput;
		m_timeEnd = CTime::GetCurrentTime();

		if(pFile){
			fprintf(pFile,"\nErrors summary\n");
			fprintf(pFile,"Maximum position error\n\tinput: XYZ:%s, IJK:%s\n\tparameters: %s\n"
				,m_cvMaxPositionErrorInput.m_p.toString().c_str() 
				,m_cvMaxPositionErrorInput.m_v.toString().c_str()
				,m_oMaxPositionErrorSolution.toString(true,"\n\t").c_str());
			fprintf(pFile,"Maximum orientation error\n\tinput: XYZ:%s, IJK:%s\n\tparameters: %s\n"
				,m_cvMaxOrientationErrorInput.m_p.toString().c_str() 
				,m_cvMaxOrientationErrorInput.m_v.toString().c_str()
				,m_oMaxOrientationErrorSolution.toString(true,"\n\t").c_str());
			fclose(pFile);
			pFile = 0;
		}
		m_progressPostprocessing.SetPos(100);
		SetWindowText("2/4 finding shortest path in progress...");
		CProgressCtrlCallback oProgress(m_progressPostprocessing);
		m_pDoc->m_oSimulationData.FindShortestPath(oPostprocessorSettings,&oProgress);
	} catch (CGeneratorException &e){
		if(pFile)
			fclose(pFile);
		m_progressPostprocessing.SetPos(0);
		m_buttonStart.EnableWindow(TRUE);
		SetWindowText("Postprocessing CL-data");
		MessageBox(e.Message().c_str());
		return;
	}
	m_progressPostprocessing.SetPos(100);
	m_buttonStart.EnableWindow(TRUE);
	m_buttonSaveGCode.EnableWindow(TRUE);

	SetWindowText("4/4 report generating in progress...");
	GenerateAndShowReport();
	if(m_pDoc->m_oSimulationData.GetEmptySolutionsCount()){
		MessageBox((std::string("There is "+size_to_string(m_pDoc->m_oSimulationData.GetEmptySolutionsCount())+" problem(s) without any valid solution\nProblems without solution are ignored during simulation/saving G-code output")).c_str());
	}
	SetWindowText("Postprocessing CL-data");
	if(m_bDetailedReport){
		OnBnClickedCheckDetailedReport();
		Invalidate(TRUE);
	}
}

std::string CPostprocessingDialog::GenerateReportHead() const
{
	assert(m_pDoc);
	std::string sReportHead;
	if(m_pDoc){
		CCutterLocationData &oCLData = m_pDoc->m_oCutterLocationData;
		sReportHead="";
		sReportHead+=("Postprocessing report \t");
		CTime t = CTime::GetCurrentTime();
		CString sDate = t.Format( "%c");
		sReportHead+=(sDate.GetBuffer(sDate.GetLength())+sNL);
		sReportHead+=("Solving method: "+oPostprocessorSettings.GetComputationMethod("\n"));
		sReportHead+=("Kinematic chain definition file: "+m_pDoc->m_oChain.GetFileName()+sNL);
		sReportHead+=("Cutter location data file name: "+oCLData.GetFileName()+sNL);
		sReportHead+=("\tlines in file: "+size_to_string(oCLData.GetLinesCount())+sNL);
		sReportHead+=("\tlines with cutter location: "+size_to_string(oCLData.GetLocationsCount())+sNL);
		sReportHead+=("\tlines with error: "+size_to_string(oCLData.GetErrorsCount())+sNL);
		sReportHead+=("\tempty lines: "+size_to_string(oCLData.GetLinesCount()-oCLData.GetLocationsCount()-oCLData.GetErrorsCount())+sNL);

		sReportHead+=("\nPostprocessing started at "+std::string(m_timeBegin.Format("%c").GetBuffer(0))+sNL);
		sReportHead+=("Postprocessing finished at "+std::string(m_timeEnd.Format("%c").GetBuffer(0))+sNL);
		CTimeSpan interval(m_timeEnd-m_timeBegin);
		sReportHead+=("Consumed time: "
			+double_to_string((double)interval.GetDays())+"days, "
			+double_to_string((double)interval.GetHours())+"h, "
			+double_to_string((double)interval.GetMinutes())+"min, "
			+double_to_string((double)interval.GetSeconds())+"s "+sNL);
		if(interval.GetTotalSeconds() && m_nTotalProblemsCount){
			double d = (double)interval.GetTotalSeconds()/(((double)m_nTotalProblemsCount/1000.0));
			sReportHead+=("Normalized time consumption: "+double_to_string(d)+" second per 1K problems"+sNL);
			sReportHead+=(" (with '"+size_to_string(2-m_nDetailedReportContents)+" solution(s)' detailed report generation)"+sNL);

		}


		sReportHead+=(sNL+"Linear interpolation settings"+sNL);
		sReportHead+=("\tmax position difference: "+double_to_string(m_dActualMaxPositionDifference)+sNL);
		sReportHead+=("\tmax orientation difference: "+double_to_string(m_dActualMaxOrientationDifferenceDegrees)+" degrees\n"+sNL);
		switch(m_nDetailedReportContents){
			case 0:
				sReportHead+=("Logs of processed problems:\n");
				break;
			case 1:
				sReportHead+=("Logs of processed problems with 1 or 0 solutions:\n");
				break;
			default:
				sReportHead+=("Logs of processed problems without solution:\n");		
		}				
		sReportHead+=sNL;

	}
	return sReportHead;
}
void CPostprocessingDialog::GenerateAndShowReport(void)
{
	assert(m_pDoc);
	if(m_pDoc){
	
		CCutterLocationData &oCLData = m_pDoc->m_oCutterLocationData;
		m_sReport="";
		m_sReport+=("Postprocessing report \t");
		CTime t = CTime::GetCurrentTime();
		CString sDate = t.Format( "%c");
		m_sReport+=(sDate.GetBuffer(0)+sRNL);
		m_sReport+=(oPostprocessorSettings.GetComputationMethod(sRNL));
		m_sReport+=("Kinematic chain definition file: "+m_pDoc->m_oChain.GetFileName()+sRNL);
		m_sReport+=("Cutter location data file: "+oCLData.GetFileName()+sRNL);
		m_sReport+=("\tlines in file: "+size_to_string(oCLData.GetLinesCount(),true));
		m_sReport+=("\tlines with cutter location: "+size_to_string(oCLData.GetLocationsCount(),true));
		m_sReport+=("\tlines with error: "+size_to_string(oCLData.GetErrorsCount(),true));
		m_sReport+=("\tempty lines: "+size_to_string(oCLData.GetLinesCount()-oCLData.GetLocationsCount()-oCLData.GetErrorsCount(),true));

		m_sReport+=(sRNL+"Postprocessing started at "+m_timeBegin.Format("%c").GetBuffer(0)+sRNL);
		m_sReport+=(std::string("Postprocessing finished at ")+m_timeEnd.Format("%c").GetBuffer(0)+sRNL);
		CTimeSpan interval(m_timeEnd-m_timeBegin);
		m_sReport+=("Consumed time: "
			+double_to_string((double)interval.GetDays())+"days, "
			+double_to_string((double)interval.GetHours())+"h, "
			+double_to_string((double)interval.GetMinutes())+"min, "
			+double_to_string((double)interval.GetSeconds())+"s "+sRNL);
		if(interval.GetTotalSeconds() && m_nTotalProblemsCount){
			double d = (double)interval.GetTotalSeconds()/(((double)m_nTotalProblemsCount)/1000.0);
			m_sReport+=("Normalized time consumption: "+double_to_string(d)+" second per 1K problems");
			if(m_bDetailedReport)
				m_sReport+=" (with '"+size_to_string(2-m_nDetailedReportContents)+" solution(s)' detailed report generation)";
			m_sReport+=sRNL;
		}

	
		m_sReport+=(sRNL+"Linear interpolation settings"+sRNL);
		m_sReport+=("\tmax position difference: "+double_to_string(m_dActualMaxPositionDifference,true));
		m_sReport+=("\tmax orientation difference: "+double_to_string(m_dActualMaxOrientationDifferenceDegrees)+" degrees"+sRNL);


		m_sReport+=(sRNL+"Interpolation results"+sRNL);
		m_sReport+=("\ttotal problems number: "+size_to_string(m_nTotalProblemsCount)+sRNL);
		m_sReport+=("\tintermediate locations number: "+size_to_string(m_nTotalProblemsCount-oCLData.GetLocationsCount())+sRNL);
		m_sReport+=("\taverage position difference: "+double_to_string(oCLData.GetTrajectoryLength()/(double)m_nTotalProblemsCount,true));
		m_sReport+=("\taverage orientation difference: "+double_to_string(radians_to_degrees( oCLData.GetTotalOrientationChangeRadians())/(double)m_nTotalProblemsCount)+" degrees"+sRNL);

		m_sReport+=(sRNL+"Computation errors"+sRNL);
		m_sReport+=("\tMaximum position error: "+sRNL);
		m_sReport+=("\t\t input: XYZ="+m_cvMaxPositionErrorInput.m_p.toString());
		m_sReport+=(" IJK="+m_cvMaxPositionErrorInput.m_v.toString()+sRNL);
		m_sReport+=("\t\tparameters: "+m_oMaxPositionErrorSolution.toString(true,"\r\n\t\t")+sRNL);

		m_sReport+=("\tMaximum orientation error "+sRNL);
		m_sReport+=("\t\t input: XYZ="+m_cvMaxOrientationErrorInput.m_p.toString());
		m_sReport+=(" IJK="+m_cvMaxOrientationErrorInput.m_v.toString()+sRNL);
		m_sReport+=("\t\tparameters: "+m_oMaxOrientationErrorSolution.toString(true,"\r\n\t")+sRNL);

		m_editReport.SetWindowText(m_sReport.c_str());
	}

}

void CPostprocessingDialog::OnBnClickedButtonProgramSettings()
{
	CSettingsDialog dlg;
	dlg.DoModal();
}

/*
void CPostprocessingDialog::OnBnClickedButtonSaveReport()
{
	CFileDialog dlg(FALSE,"txt","report.txt",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"Text files (*.txt)|*.txt|All Files (*.*)|*.*||");
	if(dlg.DoModal()==IDOK){
		FILE *pFile = fopen(dlg.GetPathName().GetString(),"wt");
		if(!pFile){
			MessageBox("Cannot open file: "+dlg.GetPathName()+"\nReport not saved");
			return;
		}
		while(1){
			size_t n = m_sReport.find_first_of('\r');
			if(m_sReport[n]=='\r')
				m_sReport[n]=' ';
			else
				break;
		}
		if(fprintf(pFile,"%s",m_sReport.c_str())<0)
			MessageBox("Cannot write to file: "+dlg.GetPathName()+"\nReport not saved");
		fclose(pFile);			
	} 
}
//*/
BOOL CPostprocessingDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	assert(m_pDoc);
	if(m_pDoc){
		m_pDoc->m_oChain.CheckAndCorrectSegmentsNames();
		size_t nFirst=0,nSecond=0,nThird=0;
		m_pDoc->m_oChain.GetTranslationsIndexes(nFirst,nSecond,nThird);
		m_vTranslationsIndexes.push_back(nFirst);
		m_vTranslationsIndexes.push_back(nSecond);		
		m_vTranslationsIndexes.push_back(nThird);
		for(size_t i=0;i<3;i++){
			std::string s = m_pDoc->m_oChain[(int)m_vTranslationsIndexes[i]].GetName();
			if(s.length() && (s[0]=='x' || s[0]=='X'))
				m_nXNameIndex=(int)i;
			if(s.length() && (s[0]=='y' || s[0]=='Y'))
				m_nYNameIndex=(int)i;
			if(s.length() && (s[0]=='z' || s[0]=='Z'))
				m_nZNameIndex=(int)i;
			m_comboXName.AddString(s.c_str());
			m_comboYName.AddString(s.c_str());
			m_comboZName.AddString(s.c_str());
		}
		m_pDoc->m_oChain.GetRotationsIndexes(nFirst,nSecond);
		m_vRotationsIndexes.push_back(nFirst);
		m_vRotationsIndexes.push_back(nSecond);
		for(size_t i=0;i<2;i++){
			m_comboAngle1Name.AddString(m_pDoc->m_oChain[(int)m_vRotationsIndexes[i]].GetName().c_str());
			m_comboAngle2Name.AddString(m_pDoc->m_oChain[(int)m_vRotationsIndexes[i]].GetName().c_str());
		}
		m_nAngle1NameIndex=0;
		m_nAngle2NameIndex=1;
		UpdateData(FALSE);

	} 
	m_comboAngle1Char.SetCurSel(0);
	m_comboAngle2Char.SetCurSel(1);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPostprocessingDialog::OnBnClickedButtonSaveGcode()
{

	CFileDialog dlg(FALSE,"nc",m_pDoc->m_oCutterLocationData.GetFileTitle().c_str() ,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"NC files (*.nc)|*.nc|All Files (*.*)|*.*||");
	if(dlg.DoModal()==IDOK){
		FILE *pFile = fopen(dlg.GetPathName().GetBuffer(0),"wt");
		if(!pFile){
			MessageBox("Cannot open file: "+dlg.GetPathName()+"\nNC-file not saved");
			return;
		}
		UpdateData(TRUE);
		int res = m_pDoc->m_oSimulationData.ExportToGCode
			(pFile
			,m_vTranslationsIndexes[m_nXNameIndex]
			,m_vTranslationsIndexes[m_nYNameIndex]
			,m_vTranslationsIndexes[m_nZNameIndex]
			,m_sAngle1GCodeName[0]
			,m_vRotationsIndexes[m_nAngle1NameIndex]
			,m_sAngle2GCodeName[0]
			,m_vRotationsIndexes[m_nAngle2NameIndex]
			,m_bRadians!=FALSE);
		if(res<0)
			MessageBox("Cannot write to file: "+dlg.GetPathName()+"\nNC-file not saved (or partially saved)");
		fclose(pFile);			
	} 
}

void CPostprocessingDialog::OnBnClickedCheckDetailedReport()
{
	UpdateData(TRUE);
	if(m_bDetailedReport){
		if(m_sReportFile.length()==0){
			CFileDialog dlg(FALSE,"txt","report.txt",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"Text files (*.txt)|*.txt|All Files (*.*)|*.*||");
			if( dlg.DoModal()==IDOK){
				m_sReportFile = dlg.GetPathName().GetBuffer(0);
			} else {
				m_bDetailedReport=FALSE;
				UpdateData(FALSE);
				m_radioDetailedReportContents0.EnableWindow(FALSE);
				m_radioDetailedReportContents1.EnableWindow(FALSE);
				m_radioDetailedReportContents2.EnableWindow(FALSE);
				m_buttonBrowseForDetailedReportFileName.EnableWindow(FALSE);
				m_editDetailedReportFileName.EnableWindow(FALSE);
				return;
			}
		}
		m_editDetailedReportFileName.SetWindowText(m_sReportFile.c_str());
		m_radioDetailedReportContents0.EnableWindow(TRUE);
		m_radioDetailedReportContents1.EnableWindow(TRUE);
		m_radioDetailedReportContents2.EnableWindow(TRUE);
		m_buttonBrowseForDetailedReportFileName.EnableWindow(TRUE);
		m_editDetailedReportFileName.EnableWindow(TRUE);
	}
	if(!m_bDetailedReport){
		m_radioDetailedReportContents0.EnableWindow(FALSE);
		m_radioDetailedReportContents1.EnableWindow(FALSE);
		m_radioDetailedReportContents2.EnableWindow(FALSE);
		m_buttonBrowseForDetailedReportFileName.EnableWindow(FALSE);
		m_editDetailedReportFileName.EnableWindow(FALSE);
	}
}

void CPostprocessingDialog::OnBnClickedButtonBrowseForDetailedReportFileName()
{
			CFileDialog dlg(FALSE,"txt","report.txt",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"Text files (*.txt)|*.txt|All Files (*.*)|*.*||");
			if( dlg.DoModal()==IDOK){
				m_sReportFile = dlg.GetPathName().GetBuffer(0);
				m_editDetailedReportFileName.SetWindowText(m_sReportFile.c_str());
			} 
}
