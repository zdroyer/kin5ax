// kin5axChainEditorPlus.h : main header file for the kin5axChainEditorPlus application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CKin5axChainEditorPlusApp:
// See kin5axChainEditorPlus.cpp for the implementation of this class
//

class CKin5axChainEditorPlusApp : public CWinApp
{
public:
	CKin5axChainEditorPlusApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	void OpenWith(int nReader, LPCTSTR lpszPathName);
	afx_msg void OnFileSaveAsJson();
	afx_msg void OnFileImportFcm();
	afx_msg void OnFileExportFcm();
	DECLARE_MESSAGE_MAP()
};

extern CKin5axChainEditorPlusApp theApp;