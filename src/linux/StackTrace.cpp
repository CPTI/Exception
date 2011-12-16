#include "StackTrace.h"

#include <errno.h>
#include <execinfo.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../Exception.h"

using namespace ExceptionLib;

void segfaulthandler(int signum)
{
	signal(SIGSEGV, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
	signal(SIGILL, SIG_DFL);
	signal(SIGFPE, SIG_DFL);

	switch(signum) {
		case SIGSEGV:
			throw SegmentationFault();
			break;
		case SIGBUS:
			throw SegmentationFault();
			break;
		case SIGFPE:
			throw FloatingPointException();
			break;
		case SIGILL:
			throw IllegalInstruction();
			break;
		default:
			throw Exception("Caught unexpected signal");
			break;
	}

	signal(SIGSEGV, segfaulthandler);
	signal(SIGBUS, segfaulthandler);
	signal(SIGILL, segfaulthandler);
	signal(SIGFPE, segfaulthandler);
}
