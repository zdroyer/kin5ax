#pragma once

#include "ProgressCallback.h"

//Shows the progress of a library operation on a progress bar (the whole range of the control)
class CProgressCtrlCallback : public CProgressCallback
{
public:
	CProgressCtrlCallback(CProgressCtrl &oProgressCtrl)
		:m_oProgressCtrl(oProgressCtrl)
	{}
	virtual void SetProgress(double dFraction)
	{
		int nLower,nUpper;
		m_oProgressCtrl.GetRange(nLower,nUpper);
		m_oProgressCtrl.SetPos(nLower+(int)(dFraction*(double)(nUpper-nLower)));
	}
protected:
	CProgressCtrl &m_oProgressCtrl;
};
