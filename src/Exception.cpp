#include "Exception.h"

#include <cstddef>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>

#include "BackTrace.h"


using namespace std;

namespace ExceptionLib {

	static bool stackEnabled = true;


	void terminate_handler()
	{
		cerr << "Caught unhandled exception" << endl;

		if (stackEnabled && ::Backtrace::backtraceSupported()) {
			::Backtrace::StackTrace* trace = ::Backtrace::trace();
			cerr << trace->getTrace() << endl;
			delete trace;
		}
	}

	void init(char *argv0)
	{
		::Backtrace::initialize(argv0);
		set_terminate(terminate_handler);
	}


	Exception::Exception(std::string errorMsg, bool trace) :
		error(errorMsg),
		st(0)
	{
		if (stackEnabled && trace) {
			st = ::Backtrace::trace();
		}
	}

	Exception::Exception(const Exception& that) :
		error(that.error),
		st(that.st)
	{
		that.st = 0;
	}

	Exception::~Exception() throw ()
	{
		try {
			if (st) delete st;
		} catch (...) {}
	}

	string Exception::stacktrace() const
	{
		if (st) {
			return st->getTrace();
		} else if (!stackEnabled) {
			return "stacktrace deactivated";
		} else {
			return "stacktrace not available";
		}
	}

	void stacktraceEnabled(bool enable) {
		stackEnabled = enable;
	}

}
