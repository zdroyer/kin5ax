#pragma once
#include "millingtool.h"



// CMillingToolPreview

class CMillingToolPreview : public CWnd
{
	DECLARE_DYNAMIC(CMillingToolPreview)

public:
	CMillingToolPreview();
	virtual ~CMillingToolPreview();
	CMillingTool *m_pMillingTool;

protected:
	DECLARE_MESSAGE_MAP()
public:
	void SetMillingTool(CMillingTool *pMillingTool);
	afx_msg void OnPaint();
};


