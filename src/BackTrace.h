#ifndef BACKTRACE_H
#define BACKTRACE_H

/* Arquivo com a declaracao de funcoes de backtrace.
 *
 */

#include <string>

namespace Backtrace {

	class StackTrace {
	public:
		virtual ~StackTrace() {}
		virtual std::string getTrace() const = 0;

	};

	void initialize(const char* argv0);
	StackTrace* trace();
	bool backtraceSupported();
}

#endif /* BACKTRACE_H */
