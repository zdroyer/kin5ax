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
	//asked between the steps of the operation: false stops it at the next opportunity (the operation
	//then says so in its result or return value, nothing is thrown). Default: go on.
	virtual bool Continue() { return true; }
};

} // namespace kin5ax
