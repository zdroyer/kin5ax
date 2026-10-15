#pragma once

// The boundary of the C interface: the one place an exception is allowed to die, and the message it
// leaves behind for k5_last_error. Shared by the sources of the interface - kin5axc.cpp and the .fcm
// bridge in kin5axfcm.cpp - so that both answer a host the same way and both keep the same promise,
// which is that nothing of C++ crosses the header, not even an exception.
#include "stdafx.h"			//the prelude of the kin5ax library, which its headers expect first
#include "kin5ax.h"
#include "ArgsMyException.h"
#include "GeneratorException.h"
#include <exception>
#include <string>

namespace k5c {

//stores the message and gives the status back, so that a failing call is one line. The message is
//kept per thread: a host that uses two machines on two threads gets its own answer on each.
k5_status Fail(k5_status nStatus, const std::string &sMessage);
//every call starts by forgetting the message of the one before it
void Clear();
//what k5_last_error hands out; empty when the last call succeeded
const char *LastError();

} // namespace k5c

//the boundary: nothing of C++ escapes through the header, not even an exception
#define K5_BEGIN	k5c::Clear(); try {
#define K5_END(nDefault)	} \
	catch(const kin5ax::CArgsMyException &e){ return k5c::Fail(K5_ERROR_ARGUMENT,e.Message()); } \
	catch(const kin5ax::CGeneratorException &e){ return k5c::Fail(nDefault,e.Message()); } \
	catch(const std::exception &e){ return k5c::Fail(K5_ERROR_UNKNOWN,e.what()); } \
	catch(...){ return k5c::Fail(K5_ERROR_UNKNOWN,"an exception of a kind this interface does not know"); }

#define K5_NEED(bCondition,szMessage)	if(!(bCondition)) return k5c::Fail(K5_ERROR_ARGUMENT,szMessage);
