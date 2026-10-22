// SolidDialog.cpp : a box, a piece of a cylinder or a piece of a sphere, measured in the frame of
// the segment that carries it
//

#include "stdafx.h"
#include "kin5axChainEditorPlus.h"
#include "SolidDialog.h"

IMPLEMENT_DYNAMIC(CSolidDialog, CDialog)

CSolidDialog::CSolidDialog(CWnd *pParent /*=NULL*/)
	: CDialog(CSolidDialog::IDD, pParent)
	,m_nTypeIndex(SOLID_BOX)
	,m_dX1(0.0) ,m_dX2(0.0) ,m_dY1(0.0) ,m_dY2(0.0) ,m_dZ1(0.0) ,m_dZ2(0.0)
	,m_nAxisIndex(R3_AXIS_Z)
	,m_dBase(0.0) ,m_dTop(0.0) ,m_dRadius(0.0)
	,m_dPX(0.0) ,m_dPY(0.0) ,m_dPZ(0.0)
	,m_dAngle1(0.0) ,m_dAngle2(360.0)
	,m_bWholeTurn(TRUE)
	,m_dCX(0.0) ,m_dCY(0.0) ,m_dCZ(0.0)
	,m_dSphereRadius(0.0)
	,m_nCutAxisIndex(R3_AXIS_Z)
	,m_dCutMin(0.0) ,m_dCutMax(0.0)
	,m_bWholeSphere(TRUE)
	,m_pSolid(0)
	,m_pWindow(0)
{
}

CSolidDialog::~CSolidDialog()
{
}

void CSolidDialog::SetSolid(const CSolid &oSolid)
{
	m_nTypeIndex = (int)oSolid.GetType();
	//the numbers of all three kinds are kept, so that changing the kind back costs no typing
	const CR3Box oBox = (oSolid.GetType()==SOLID_BOX) ? oSolid.GetBox() : oSolid.GetBoundingBox();
	m_dX1 = oBox.GetMinPoint().GetX();
	m_dY1 = oBox.GetMinPoint().GetY();
	m_dZ1 = oBox.GetMinPoint().GetZ();
	m_dX2 = oBox.GetMaxPoint().GetX();
	m_dY2 = oBox.GetMaxPoint().GetY();
	m_dZ2 = oBox.GetMaxPoint().GetZ();

	const CR3CylinderSegment &oC = oSolid.GetCylinder();
	if(oSolid.GetType()==SOLID_CYLINDER){
		m_nAxisIndex = (int)oC.GetAxis();
		m_dBase = oC.GetBase();
		m_dTop = oC.GetTop();
		m_dRadius = oC.GetRadius();
		m_dPX = oC.GetAxisPoint().GetX();
		m_dPY = oC.GetAxisPoint().GetY();
		m_dPZ = oC.GetAxisPoint().GetZ();
		m_bWholeTurn = oC.IsWholeTurn() ? TRUE : FALSE;
		m_dAngle1 = oC.GetMinAngleDegrees();
		m_dAngle2 = oC.GetMaxAngleDegrees();
	}
	if(oSolid.GetType()==SOLID_SPHERE){
		const CR3SphereSegment &oS = oSolid.GetSphere();
		m_dCX = oS.GetCentre().GetX();
		m_dCY = oS.GetCentre().GetY();
		m_dCZ = oS.GetCentre().GetZ();
		m_dSphereRadius = oS.GetRadius();
		m_nCutAxisIndex = (int)oS.GetAxis();
		m_bWholeSphere = oS.IsWholeSphere() ? TRUE : FALSE;
		m_dCutMin = oS.GetMin();
		m_dCutMax = oS.GetMax();
	}
}

CSolid CSolidDialog::GetSolid() const
{
	if(m_nTypeIndex==SOLID_CYLINDER){
		const double dFrom = m_bWholeTurn ? 0.0 : m_dAngle1;
		const double dTo = m_bWholeTurn ? 360.0 : m_dAngle2;
		return CSolid(CR3CylinderSegment((R3_AXIS)m_nAxisIndex,CR3Point(m_dPX,m_dPY,m_dPZ)
			,m_dBase,m_dTop,m_dRadius,dFrom,dTo));
	}
	if(m_nTypeIndex==SOLID_SPHERE){
		const CR3Point pCentre(m_dCX,m_dCY,m_dCZ);
		if(m_bWholeSphere)
			return CSolid(CR3SphereSegment(pCentre,m_dSphereRadius));
		return CSolid(CR3SphereSegment(pCentre,m_dSphereRadius,(R3_AXIS)m_nCutAxisIndex
			,m_dCutMin,m_dCutMax));
	}
	return CSolid(CR3Box(m_dX1,m_dX2,m_dY1,m_dY2,m_dZ1,m_dZ2));
}

void CSolidDialog::SetWindowAndSolidForUpdates(CWnd *pWnd, CSolid *pSolid)
{
	m_pSolid = pSolid;
	m_pWindow = pWnd;
	if(pSolid)
		SetSolid(*pSolid);
	UpdateWindowAndSolid();
}

void CSolidDialog::DoDataExchange(CDataExchange *pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBO_SOLID_TYPE, m_nTypeIndex);
	DDX_Text(pDX, IDC_EDIT_SOLID_X1, m_dX1);
	DDX_Text(pDX, IDC_EDIT_SOLID_X2, m_dX2);
	DDX_Text(pDX, IDC_EDIT_SOLID_Y1, m_dY1);
	DDX_Text(pDX, IDC_EDIT_SOLID_Y2, m_dY2);
	DDX_Text(pDX, IDC_EDIT_SOLID_Z1, m_dZ1);
	DDX_Text(pDX, IDC_EDIT_SOLID_Z2, m_dZ2);
	DDX_CBIndex(pDX, IDC_COMBO_SOLID_AXIS, m_nAxisIndex);
	DDX_Text(pDX, IDC_EDIT_SOLID_BASE, m_dBase);
	DDX_Text(pDX, IDC_EDIT_SOLID_TOP, m_dTop);
	DDX_Text(pDX, IDC_EDIT_SOLID_RADIUS, m_dRadius);
	DDX_Text(pDX, IDC_EDIT_SOLID_PX, m_dPX);
	DDX_Text(pDX, IDC_EDIT_SOLID_PY, m_dPY);
	DDX_Text(pDX, IDC_EDIT_SOLID_PZ, m_dPZ);
	DDX_Text(pDX, IDC_EDIT_SOLID_ANGLE1, m_dAngle1);
	DDX_Text(pDX, IDC_EDIT_SOLID_ANGLE2, m_dAngle2);
	DDX_Check(pDX, IDC_CHECK_WHOLE_TURN, m_bWholeTurn);
	DDX_Text(pDX, IDC_EDIT_SOLID_CX, m_dCX);
	DDX_Text(pDX, IDC_EDIT_SOLID_CY, m_dCY);
	DDX_Text(pDX, IDC_EDIT_SOLID_CZ, m_dCZ);
	DDX_Text(pDX, IDC_EDIT_SPHERE_RADIUS, m_dSphereRadius);
	DDX_CBIndex(pDX, IDC_COMBO_SOLID_CUT_AXIS, m_nCutAxisIndex);
	DDX_Text(pDX, IDC_EDIT_SOLID_CUT_MIN, m_dCutMin);
	DDX_Text(pDX, IDC_EDIT_SOLID_CUT_MAX, m_dCutMax);
	DDX_Check(pDX, IDC_CHECK_WHOLE_SPHERE, m_bWholeSphere);
	DDX_Text(pDX, IDC_STATIC_SOLID_PROBLEM, m_sProblem);
	DDX_Control(pDX, IDC_COMBO_SOLID_TYPE, m_comboType);
	DDX_Control(pDX, IDC_COMBO_SOLID_AXIS, m_comboAxis);
	DDX_Control(pDX, IDC_COMBO_SOLID_CUT_AXIS, m_comboCutAxis);
}

BEGIN_MESSAGE_MAP(CSolidDialog, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_SOLID_TYPE, OnCbnSelchangeComboSolidType)
	ON_CBN_SELCHANGE(IDC_COMBO_SOLID_AXIS, OnEnChangeAnySolidField)
	ON_CBN_SELCHANGE(IDC_COMBO_SOLID_CUT_AXIS, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_X1, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_X2, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_Y1, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_Y2, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_Z1, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_Z2, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_BASE, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_TOP, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_RADIUS, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_PX, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_PY, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_PZ, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_ANGLE1, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_ANGLE2, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_CX, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_CY, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_CZ, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SPHERE_RADIUS, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_CUT_MIN, OnEnChangeAnySolidField)
	ON_EN_CHANGE(IDC_EDIT_SOLID_CUT_MAX, OnEnChangeAnySolidField)
	ON_BN_CLICKED(IDC_CHECK_WHOLE_TURN, OnBnClickedCheckWholeTurn)
	ON_BN_CLICKED(IDC_CHECK_WHOLE_SPHERE, OnBnClickedCheckWholeSphere)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CSolidDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_comboType.ResetContent();
	for(int i=0;i<SOLID_TYPE_COUNT;i++)
		m_comboType.AddString(SolidTypeName((SOLID_TYPE)i));		//the order of SOLID_TYPE
	m_comboType.SetCurSel(m_nTypeIndex);
	m_comboAxis.ResetContent();
	m_comboCutAxis.ResetContent();
	for(int i=0;i<R3_AXIS_COUNT;i++){
		m_comboAxis.AddString(R3AxisName((R3_AXIS)i));
		m_comboCutAxis.AddString(R3AxisName((R3_AXIS)i));
	}
	m_comboAxis.SetCurSel(m_nAxisIndex);
	m_comboCutAxis.SetCurSel(m_nCutAxisIndex);
	EnableFields();
	UpdateWindowAndSolid();
	return TRUE;
}

void CSolidDialog::EnableFields()
{
	const int nBox[] = {IDC_EDIT_SOLID_X1,IDC_EDIT_SOLID_X2,IDC_EDIT_SOLID_Y1,IDC_EDIT_SOLID_Y2
		,IDC_EDIT_SOLID_Z1,IDC_EDIT_SOLID_Z2};
	const int nCylinder[] = {IDC_COMBO_SOLID_AXIS,IDC_EDIT_SOLID_BASE,IDC_EDIT_SOLID_TOP
		,IDC_EDIT_SOLID_RADIUS,IDC_EDIT_SOLID_PX,IDC_EDIT_SOLID_PY,IDC_EDIT_SOLID_PZ
		,IDC_CHECK_WHOLE_TURN};
	const int nSphere[] = {IDC_EDIT_SOLID_CX,IDC_EDIT_SOLID_CY,IDC_EDIT_SOLID_CZ
		,IDC_EDIT_SPHERE_RADIUS,IDC_CHECK_WHOLE_SPHERE};
	const int nSector[] = {IDC_EDIT_SOLID_ANGLE1,IDC_EDIT_SOLID_ANGLE2};
	const int nCut[] = {IDC_COMBO_SOLID_CUT_AXIS,IDC_EDIT_SOLID_CUT_MIN,IDC_EDIT_SOLID_CUT_MAX};

	const BOOL bBox = (m_nTypeIndex==SOLID_BOX) ? TRUE : FALSE;
	const BOOL bCylinder = (m_nTypeIndex==SOLID_CYLINDER) ? TRUE : FALSE;
	const BOOL bSphere = (m_nTypeIndex==SOLID_SPHERE) ? TRUE : FALSE;
	for(int i=0;i<6;i++)
		if(GetDlgItem(nBox[i]))
			GetDlgItem(nBox[i])->EnableWindow(bBox);
	for(int i=0;i<8;i++)
		if(GetDlgItem(nCylinder[i]))
			GetDlgItem(nCylinder[i])->EnableWindow(bCylinder);
	for(int i=0;i<5;i++)
		if(GetDlgItem(nSphere[i]))
			GetDlgItem(nSphere[i])->EnableWindow(bSphere);
	//the wedge and the cut are asked for only when they are not the whole shape
	for(int i=0;i<2;i++)
		if(GetDlgItem(nSector[i]))
			GetDlgItem(nSector[i])->EnableWindow(bCylinder && !m_bWholeTurn);
	for(int i=0;i<3;i++)
		if(GetDlgItem(nCut[i]))
			GetDlgItem(nCut[i])->EnableWindow(bSphere && !m_bWholeSphere);
}

void CSolidDialog::UpdateWindowAndSolid()
{
	const CSolid oSolid = GetSolid();
	const std::string sProblem = oSolid.Problem();
	m_sProblem = sProblem.empty() ? CString("") : CString(sProblem.c_str());
	//the caller sets this dialog up before it is shown - the solid has to be in the segment for the
	//3D view to draw it while it is measured - and there are no controls to write to until then
	if(GetSafeHwnd())
		UpdateData(FALSE);
	//a half typed solid is normal, and the segment keeps whatever the fields make of it: the view
	//then shows nothing for an empty one, which is the honest picture
	if(m_pSolid)
		*m_pSolid = oSolid;
	if(m_pWindow)
		m_pWindow->Invalidate();
}

void CSolidDialog::OnCbnSelchangeComboSolidType()
{
	UpdateData(TRUE);
	EnableFields();
	UpdateWindowAndSolid();
}

void CSolidDialog::OnEnChangeAnySolidField()
{
	UpdateData(TRUE);
	UpdateWindowAndSolid();
}

void CSolidDialog::OnBnClickedCheckWholeTurn()
{
	UpdateData(TRUE);
	if(!m_bWholeTurn && m_dAngle2-m_dAngle1<=0.0){
		m_dAngle1 = 0.0;					//a wedge to start from, rather than an empty one
		m_dAngle2 = 90.0;
	}
	EnableFields();
	UpdateWindowAndSolid();
}

void CSolidDialog::OnBnClickedCheckWholeSphere()
{
	UpdateData(TRUE);
	if(!m_bWholeSphere && m_dCutMax-m_dCutMin<=0.0){
		m_dCutMin = 0.0;					//the upper half, which is the usual dome
		m_dCutMax = m_dSphereRadius;
	}
	EnableFields();
	UpdateWindowAndSolid();
}

void CSolidDialog::OnBnClickedOk()
{
	UpdateData(TRUE);
	const CSolid oSolid = GetSolid();
	const std::string sProblem = oSolid.Problem();
	if(sProblem.size()){
		AfxMessageBox((std::string("The fields do not make a solid:\n\n")+sProblem+".").c_str());
		return;
	}
	UpdateWindowAndSolid();
	OnOK();
}
