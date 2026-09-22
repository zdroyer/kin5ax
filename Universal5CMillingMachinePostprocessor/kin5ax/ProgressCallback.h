#pragma once

//Receiver of the progress of a long running operation of the library.
//Implemented on the GUI side (see CProgressCtrlCallback); the library itself knows no GUI.
namespace kin5ax {

class CProgressCallback
{
public:
	virtual ~CProgressCallback(void){}
	//dFraction: from 0.0 (just started) to 1.0 (done)
	virtual void SetProgress(double dFraction) = 0;
};

} // namespace kin5ax
