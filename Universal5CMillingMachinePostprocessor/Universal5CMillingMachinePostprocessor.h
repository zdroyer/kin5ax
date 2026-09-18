// Universal5CMillingMachinePostprocessor.h : main header file for the Universal5CMillingMachinePostprocessor application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CUniversal5CMillingMachinePostprocessorApp:
// See Universal5CMillingMachinePostprocessor.cpp for the implementation of this class
//

class CUniversal5CMillingMachinePostprocessorApp : public CWinApp
{
public:
	CUniversal5CMillingMachinePostprocessorApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CUniversal5CMillingMachinePostprocessorApp theApp;