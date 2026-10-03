#include "StdAfx.h"
//#include ".\applicationconfiguration.h"
#include "double_tools.h"



CApplicationConfiguration::CApplicationConfiguration(void)
:m_bPerspectiveProjection(false)
//,m_dModelSize(2.0)
,m_dUnit(1.0)
,m_bFixedMaterialFrame(false)
,m_bShowMaterial(false)
,m_bShowMaterialFrame(false)
,m_bShowMaterialContour(true)
,m_bShowMaterialHolder(true)
,m_bShowTool(true)
,m_bShowToolFrame(false)
,m_bShowToolTrajectory(true)
,m_bShowLinksFrames(false)
,m_bShowLinks(true)
,m_bShowBase(false)
,m_bShowParts(true)
,m_bShowBaseFrame(true)
,m_dInputPrecision(dEPSILON)
,m_nRefreshPeriod(20)
,m_nSimulationInterval(0)
,m_bShowToolBoundingBox(false)
{
}

CApplicationConfiguration::~CApplicationConfiguration(void)
{
}


