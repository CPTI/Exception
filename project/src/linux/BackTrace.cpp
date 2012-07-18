#include "../BackTrace.h"
#include "../Exception.h"

#include <errno.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <cxxabi.h>

#include <iterator>
#include <vector>
#include <sstream>
#include <memory>

#include "StackAddressLoader.h"
#include "DebugSymbolLoader.h"

using namespace std;
using namespace ExceptionLib;

namespace {

	void segfaulthandler(int signum, siginfo_t * info, void*)
	{
		stringstream ss;

		void * addr = info->si_addr;
		int code = info->si_code;

		switch(signum) {
			case SIGSEGV:
			{
				ss << "Segmentation fault at: " << addr;
				if (code & SEGV_MAPERR) {
					ss << ", address not mapped to object";
				} else if (code & SEGV_ACCERR) {
					ss << ", invalid permissions for mapped object";
				}
				throw SegmentationFault(ss.str());
				break;
			}
			case SIGBUS:
			{
				ss << "Bus error at: " << addr;
				if (code & BUS_ADRALN) {
					ss << ", invalid address alignment";
				} else if (code & BUS_ADRERR) {
					ss << ", nonexistent physical address";
				} else if (code & BUS_OBJERR) {
					ss << ", object-specific hardware error";
				}
				throw SegmentationFault(ss.str());
				break;
			}
			case SIGFPE:
			{
				ss << "Floating point error at: " << addr;
				if (code & FPE_INTDIV) {
					ss << ", integer divide by zero";
				} else if (code & FPE_INTOVF) {
					ss << ", integer overflow";
				} else if (code & FPE_FLTDIV) {
					ss << ", floating-point divide by zero";
				} else if (code & FPE_FLTOVF) {
					ss << ", floating-point overflow";
				} else if (code & FPE_FLTUND) {
					ss << ", floating-point underflow";
				} else if (code & FPE_FLTRES) {
					ss << ", floating-point inexact result";
				} else if (code & FPE_FLTINV) {
					ss << ", floating-point invalid operation";
				} else if (code & FPE_FLTSUB) {
					ss << ", subscript out of range";
				}
				throw FloatingPointException(ss.str());
				break;
			}
			case SIGILL:
			{
				ss << "Illegal instruction at: " << addr;
				if (code & ILL_ILLOPC) {
					ss << ", illegal opcode";
				} else if (code & ILL_ILLOPN) {
					ss << ", illegal operand";
				} else if (code & ILL_ILLADR) {
					ss << ", illegal addressing mode";
				} else if (code & ILL_ILLTRP) {
					ss << ", illegal trap";
				} else if (code & ILL_PRVOPC) {
					ss << ", privileged opcode";
				} else if (code & ILL_PRVREG) {
					ss << ", privileged register";
				} else if (code & ILL_COPROC) {
					ss << ", coprocessor error";
				} else if (code & ILL_BADSTK) {
					ss << ", internal stack error";
				}
				throw IllegalInstruction(ss.str());
				break;
			}
			default:
			{
				ss << "Caught unexpected signal: " << info->si_signo << addr;
				throw Exception(ss.str());
				break;
			}
		}
	}

}


namespace Backtrace {
	void initialize(const char* argv0)
	{
		initializeExecutablePath(argv0);

		struct sigaction action;

		action.sa_handler = 0;
		action.sa_sigaction = segfaulthandler;
		sigemptyset (&action.sa_mask);
		sigaddset(&action.sa_mask, SIGSEGV);
		sigaddset(&action.sa_mask, SIGBUS);
		sigaddset(&action.sa_mask, SIGILL);
		sigaddset(&action.sa_mask, SIGFPE);
		action.sa_flags = SA_SIGINFO;

		sigaction(SIGSEGV, &action, NULL);
		sigaction(SIGBUS, &action, NULL);
		sigaction(SIGILL, &action, NULL);
		sigaction(SIGFPE, &action, NULL);
	}


	StackTrace* trace()
	{
		const int MAX_STACK = 32;
		std::auto_ptr<StackTrace> trace(new StackTrace());
		trace->getFrames().resize(MAX_STACK);
		const int num = getPlatformStackLoader().getStack(MAX_STACK, &trace->getFrames()[0]);
		trace->getFrames().resize(num);

		return trace.release();
	}

	bool backtraceSupported()
	{
		return true;
	}
}
