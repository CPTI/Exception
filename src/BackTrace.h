#ifndef BACKTRACE_H
#define BACKTRACE_H

/* Arquivo com a declaracao de funcoes de backtrace.
 *
 */

#include <string>
#include <stdint.h>

namespace Backtrace {

	struct StackFrame {
		void* addr;
		std::string function;
		int line;
		std::string sourceFile;
		std::string imageFile;
		StackFrame() : addr(0), function(""), line(-1), sourceFile(""), imageFile("") {}
	};

	class StackTrace {
	public:

		StackTrace() : m_referenceCount(1) {}

		virtual ~StackTrace() {}

		//virtual std::string getDetailedTrace() const = 0;
		virtual std::string getTrace() const = 0;

		void increaseCount() {
			++m_referenceCount;
		}

		void decreaseCount() {
			--m_referenceCount;
			if (m_referenceCount <= 0) {
				delete this; // o destrutor é virtual
			}
		}

	private:
		int m_referenceCount;
	};

	void initialize(const char* argv0);
	StackTrace* trace();
	bool backtraceSupported();

}

#endif /* BACKTRACE_H */
