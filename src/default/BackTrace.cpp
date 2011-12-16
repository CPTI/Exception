#include "../BackTrace.h"

namespace {
    class StackTraceDefault : public ::Backtrace::StackTrace {
	public:
        virtual std::string getTrace() const {
			return "stacktrace not supported";
		} 
    };
}

namespace Backtrace {

    void initialize(const char*) {}
    StackTrace* trace() { return new StackTraceDefault(); }
    bool backtraceSupported() { return false; }
}
