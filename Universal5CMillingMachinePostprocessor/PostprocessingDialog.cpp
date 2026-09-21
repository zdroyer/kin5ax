// PostprocessingDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Universal5CMillingMachinePostprocessor.h"
#include "postprocessingdialog.h"
#include "settingsdialog.h"
#include "string_tools.h"
#include ".\postprocessingdialog.h"
// CPostprocessingDialog dialog

extern CApplicationConfiguration oConfiguration;

IMPLEMENT_DYNAMIC(CPostprocessingDialog, CDialog)
CPostprocessingDialog::CPostprocessingDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPostprocessingDialog::IDD, pParent)
	, m_pDoc(0)
	, m_dPositionDifference(oConfiguration.m_dMaxPositionDifference)
	, m_dOrientationDifference(oConfiguration.m_dMaxOrientationDifferenceDegrees)
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
	s.Format("%d",n);
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
		SetText(m_editTrajectoryLength,m_pDoc->m_oCutterLocationData.GetTrajectoryLenght());
		UpdateProblemsCountControl();
		m_vSolutions.reserve(m_pDoc->m_oCutterLocationData.GetLocationsCount());
		m_pDoc->m_oSimulationData.ReserveMemory(m_pDoc->m_oCutterLocationData.GetLocationsCount());
	}
}

void CPostprocessingDialog::OnEnChangeEditOrientationDifference()
{
	UpdateData();
	oConfiguration.m_dMaxOrientationDifferenceDegrees = fabs(m_dOrientationDifference);
	oConfiguration.m_dMaxOrientationDifferenceRadians = degrees_to_radians(oConfiguration.m_dMaxOrientationDifferenceDegrees);
	UpdateProblemsCountControl();
}

void CPostprocessingDialog::OnEnChangeEditPositionDifference()
{
	UpdateData();
	oConfiguration.m_dMaxPositionDifference = fabs(m_dPositionDifference);
	UpdateProblemsCountControl();
}

void CPostprocessingDialog::UpdateProblemsCountControl()
{
	assert(m_pDoc);
	if(m_pDoc){
		double dLocationsCount = (double) m_pDoc->m_oCutterLocationData.GetLocationsCount();
		double dAvgDistance = m_pDoc->m_oCutterLocationData.GetTrajectoryLenght()/dLocationsCount;
		double dProblemsForPosition = dLocationsCount*max(1.0,dAvgDistance/oConfiguration.m_dMaxPositionDifference);

		double dAvgAngle = radians_to_degrees( m_pDoc->m_oCutterLocationData.GetTotalOrientationChangeRadians())/dLocationsCount;
		double dProblemsForOrientation = dLocationsCount*max(1.0,dAvgAngle/oConfiguration.m_dMaxOrientationDifferenceDegrees);
		
		double  dProblemsCount = max(dProblemsForOrientation,dProblemsForPosition);
		SetText(m_editProblemsCount,(size_t)(1.2*dProblemsCount));
		m_vSolutions.reserve(size_t(1.2*dProblemsCount));
		m_pDoc->m_oSimulationData.ReserveMemory(size_t(1.2*dProblemsCount));
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
		
	m_buttonSaveGCode.EnableWindow(FALSE);

	m_buttonStart.EnableWindow(FALSE);
	m_progressPostprocessing.SetRange(0,100);
	m_progressPostprocessing.SetPos(0);

	m_dActualMaxOrientationDiferenceDegrees = oConfiguration.m_dMaxOrientationDifferenceDegrees;
	m_dActualMaxPositionDifference = oConfiguration.m_dMaxPositionDifference;
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
		fprintf(pFile,GenerateReportHead().c_str());
	}
	
	m_vsProblemsLogs.erase(m_vsProblemsLogs.begin(),m_vsProblemsLogs.end());
	C5CPostprocessor oPostporcessor(m_pDoc->m_oChain);
	CCutterLocationData &data = m_pDoc->m_oCutterLocationData;
	CR3Point p1,p2,pi;
	CR3Vector v1,v2,vi;
	m_editReport.SetWindowText("");
	m_vSolutions.erase(m_vSolutions.begin(),m_vSolutions.end());

	size_t n = max(1,data.GetLocationsCount()/300);
	vector<pair<double,CR3CaughtVector> > vIntermediateLocations;

	data.ResetLocationIterator();
	data.GetNextLocation(p1,v1);
	data.ResetLocationIterator();
	m_pDoc->m_oSimulationData.DeleteContents();
	SetWindowText("1/4 postprocessing in progress...");
	string sLog;
	bool bCollectingAmbiguousSollutions=false;
	vector<CR3CaughtVector> vAmbiguousCollection;
	vAmbiguousCollection.reserve(50);
	CR3CaughtVector lastInput(CR3Vector(0.0),CR3Point(0.0));
	m_timeBegin = m_timeBegin.GetCurrentTime();
	for(int i=0; data.GetNextLocation(p2,v2);i++){
		InterpolateLoaction(p1,v1,p2,v2,vIntermediateLocations);
		for(size_t j=0;j<vIntermediateLocations.size();j++){
			sLog="";
			tChainConfigurationVector vSolutions = oPostporcessor.Solve
				(vIntermediateLocations[j].second.m_p
				,vIntermediateLocations[j].second.m_v
				,m_pDoc->m_oChain
				,*m_pDoc->m_pMillingTool
				,sLog);
			if(vSolutions.size() && vSolutions[0].IsAmbiguous()){
				if(!bCollectingAmbiguousSollutions){
					vAmbiguousCollection.erase(vAmbiguousCollection.begin(),vAmbiguousCollection.end());
					vAmbiguousCollection.push_back(lastInput);
					bCollectingAmbiguousSollutions=true;
				}
				vAmbiguousCollection.push_back(vIntermediateLocations[j].second);
				if(j+1>=vIntermediateLocations.size() && !data.HasMoreLocations()){
					string sSingularSolutionLog="";
					vector<tChainConfigurationVector> vUnambiguous = oPostporcessor.SolveSingularOrientation(vAmbiguousCollection,m_pDoc->m_oChain,*m_pDoc->m_pMillingTool,sSingularSolutionLog);
					for(size_t q=0;q<vUnambiguous.size();q++)
						m_pDoc->m_oSimulationData.AddConfigurationSet(vUnambiguous[q]);
					if(m_bDetailedReport)
						fprintf(pFile,"%s\n",sSingularSolutionLog.c_str());
					bCollectingAmbiguousSollutions=false;
				}
			} else {
				if(bCollectingAmbiguousSollutions){
					vAmbiguousCollection.push_back(vIntermediateLocations[j].second);
					string sSingularSolutionLog="";
					vector<tChainConfigurationVector> vUnambiguous = oPostporcessor.SolveSingularOrientation(vAmbiguousCollection,m_pDoc->m_oChain,*m_pDoc->m_pMillingTool,sSingularSolutionLog);
					for(size_t q=0;q<vUnambiguous.size();q++)
						m_pDoc->m_oSimulationData.AddConfigurationSet(vUnambiguous[q]);
					if(m_bDetailedReport)
						fprintf(pFile,"%s\n",sSingularSolutionLog.c_str());
					bCollectingAmbiguousSollutions=false;
				} 
                m_pDoc->m_oSimulationData.AddConfigurationSet(vSolutions);
			}
			lastInput.m_p = vIntermediateLocations[j].second.m_p;
			lastInput.m_v = vIntermediateLocations[j].second.m_v;
			m_nTotalProblemsCount++;

			for(size_t k=0;k<vSolutions.size();k++){
				m_pDoc->m_oChain.ComputeAndSetError(vSolutions[k],lastInput);
				if(m_dMaxOrientationErrorVectorLength<vSolutions[k].GetOrientationErrorInDistanceUnit()){
					m_dMaxOrientationErrorVectorLength = vSolutions[k].GetOrientationErrorInDistanceUnit();
					m_oMaxOrientationErrorSolution = vSolutions[k];
					m_cvMaxOrientationErrorInput = lastInput;
				}
				if(m_dMaxPositionError < vSolutions[k].GetPositionError()){
					m_dMaxPositionError = vSolutions[k].GetPositionError();
					m_oMaxPositionErrorSolution = vSolutions[k];
					m_cvMaxPositionErrorInput = lastInput;
				}
			}
			
			if(m_bDetailedReport){
				string s = "XYZ:"+vIntermediateLocations[j].second.m_p.toString()+", IJK:"+vIntermediateLocations[j].second.m_v.toString()+", location no. "+size_to_string(i+1);
				string s2="";

				for(size_t k=0;k<vSolutions.size();k++){
					s2+=(""+vSolutions[k].toString()+"\n; ");
				}
				if(vSolutions.size()){
					s2+=sNL;
				}
				switch(m_nDetailedReportContents){
					case 0:
						fprintf(pFile,"%s\n%s%s\n",s.c_str(),sLog.c_str(),s2.c_str());
						break;
					case 1:
						if(vSolutions.size()<2)
							fprintf(pFile,"%s\n%s%s\n",s.c_str(),sLog.c_str(),s2.c_str());
						break;
					default:
						if(vSolutions.size()<2)
							fprintf(pFile,"%s\n%s",s.c_str(),sLog.c_str());
				}				

			}
		}
		if(i%n==0){
			m_progressPostprocessing.SetPos((int)(100*i/data.GetLocationsCount()));
		}
		p1=p2;
		v1=v2;
	}
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
	}
	m_progressPostprocessing.SetPos(100);
	SetWindowText("2/4 finding shortest path in progres...");
	m_pDoc->m_oSimulationData.FindShortestPath(m_progressPostprocessing);
	m_progressPostprocessing.SetPos(100);
	m_buttonStart.EnableWindow(TRUE);
	m_buttonSaveGCode.EnableWindow(TRUE);

	SetWindowText("4/4 report generating in progress...");
	GenerateAndShowReport();
	if(m_pDoc->m_oSimulationData.GetEmptySolutionsCount()){
		MessageBox((string("There is "+size_to_string(m_pDoc->m_oSimulationData.GetEmptySolutionsCount())+" problem(s) without any valid solution\nProblems without solution are ignored during simulation/saving G-code output")).c_str());
	}
	SetWindowText("Postprocessing CL-data");
	if(m_bDetailedReport){
		OnBnClickedCheckDetailedReport();
		Invalidate(TRUE);
	}
}

void CPostprocessingDialog::InterpolateLoaction
	( const CR3Point &p1
	, const CR3Vector &v1
	, const CR3Point &p2
	, const CR3Vector &v2
	, vector<pair<double,CR3CaughtVector> > & result)
{
	result.erase(result.begin(),result.end());
	if(p1==p2 && v1==v2){
		result.push_back(pair<double,CR3CaughtVector>(1.0,CR3CaughtVector(v2,p2)));	
		return;
	}
	double dAngle = (v1.UndirectedVersorsAngle(v2)/oConfiguration.m_dMaxOrientationDifferenceRadians);
	double dDistance = (d_E(p1,p2)/oConfiguration.m_dMaxPositionDifference);
	double dInv = ceil(max(dAngle,dDistance));
	if(dInv<=1.0){
		result.push_back(pair<double,CR3CaughtVector>(1.0,CR3CaughtVector(v2,p2)));
		return;
	} 
	double t;
	result.reserve((size_t)fabs(dInv));
	for(double i=1.0;i<dInv-0.001; i++){
		t = i/dInv;
		result.push_back(pair<double,CR3CaughtVector>(t,CR3CaughtVector(((v2*t)+(v1*(1.0-t))).GetVersor(),(p2*t)+(p1*(1.0-t)))));
	}
	result.push_back(pair<double,CR3CaughtVector>(1.0,CR3CaughtVector(v2,p2)));
}
string CPostprocessingDialog::GenerateReportHead() const
{
	assert(m_pDoc);
	string sReportHead;
	if(m_pDoc){
		CCutterLocationData &oCLData = m_pDoc->m_oCutterLocationData;
		sReportHead="";
		sReportHead+=("Postprocessing report \t");
		CTime t = CTime::GetCurrentTime();
		CString sDate = t.Format( "%c");
		sReportHead+=(sDate.GetBuffer(sDate.GetLength())+sNL);
		sReportHead+=("Solving method: "+oConfiguration.GetComputationMethod("\n"));
		sReportHead+=("Kinematic chain definition file: "+m_pDoc->m_oChain.GetFileName()+sNL);
		sReportHead+=("Cutter location data file name: "+oCLData.GetFileName()+sNL);
		sReportHead+=("\tlines in file: "+size_to_string(oCLData.GetLinesCount())+sNL);
		sReportHead+=("\tlines with cutter location: "+size_to_string(oCLData.GetLocationsCount())+sNL);
		sReportHead+=("\tlines with error: "+size_to_string(oCLData.GetErrorsCount())+sNL);
		sReportHead+=("\tempty lines: "+size_to_string(oCLData.GetLinesCount()-oCLData.GetLocationsCount()-oCLData.GetErrorsCount())+sNL);

		sReportHead+=("\nPostprocessing started at "+string(m_timeBegin.Format("%c").GetBuffer(0))+sNL);
		sReportHead+=("Postoprocessing finished at "+string(m_timeEnd.Format("%c").GetBuffer(0))+sNL);
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
		sReportHead+=("\tmax orientation diference: "+double_to_string(m_dActualMaxOrientationDiferenceDegrees)+" degrees\n"+sNL);
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
		m_sReport+=(oConfiguration.GetComputationMethod(sRNL));
		m_sReport+=("Kinematic chain definition file: "+m_pDoc->m_oChain.GetFileName()+sRNL);
		m_sReport+=("Cutter location data file: "+oCLData.GetFileName()+sRNL);
		m_sReport+=("\tlines in file: "+size_to_string(oCLData.GetLinesCount(),true));
		m_sReport+=("\tlines with cutter location: "+size_to_string(oCLData.GetLocationsCount(),true));
		m_sReport+=("\tlines with error: "+size_to_string(oCLData.GetErrorsCount(),true));
		m_sReport+=("\tempty lines: "+size_to_string(oCLData.GetLinesCount()-oCLData.GetLocationsCount()-oCLData.GetErrorsCount(),true));

		m_sReport+=(sRNL+"Postprocessing started at "+m_timeBegin.Format("%c").GetBuffer(0)+sRNL);
		m_sReport+=(string("Postoprocessing finished at ")+m_timeEnd.Format("%c").GetBuffer(0)+sRNL);
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
		m_sReport+=("\tmax orientation diference: "+double_to_string(m_dActualMaxOrientationDiferenceDegrees)+" degrees"+sRNL);


		m_sReport+=(sRNL+"Interpolation results"+sRNL);
		m_sReport+=("\ttotal problems number: "+size_to_string(m_nTotalProblemsCount)+sRNL);
		m_sReport+=("\tintermediate locations number: "+size_to_string(m_nTotalProblemsCount-oCLData.GetLocationsCount())+sRNL);
		m_sReport+=("\taverage position difference: "+double_to_string(oCLData.GetTrajectoryLenght()/(double)m_nTotalProblemsCount,true));
		m_sReport+=("\taverage orientation diference: "+double_to_string(radians_to_degrees( oCLData.GetTotalOrientationChangeRadians())/(double)m_nTotalProblemsCount)+" degrees"+sRNL);

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
			string s = m_pDoc->m_oChain[(int)m_vTranslationsIndexes[i]].GetName();
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
			,m_bRadians);
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
