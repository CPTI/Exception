#ifndef BACKTRACE_H
#define BACKTRACE_H

/* Arquivo com a declaracao de funcoes de backtrace.
 *
 */

#include <string>
#include <vector>
#include <stdint.h>
#include <sstream>

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

		~StackTrace() {}

		std::string asString() const {
			std::stringstream ss;
			for (size_t i = 0; i < m_frames.size(); ++i) {
				ss << m_frames[i].addr << ":  " << m_frames[i].function << " in (" << m_frames[i].imageFile << ")";
				if (m_frames[i].line >= 0) {
					ss << " at " << m_frames[i].sourceFile << ": " << m_frames[i].line;
				}
				ss << "\n";
			}
			return ss.str();
		}

		std::vector<StackFrame>& getFrames() { return m_frames; }

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
		std::vector<StackFrame> m_frames;
	};

	void initialize(const char* argv0);
	StackTrace* trace();
	bool backtraceSupported();

}

#endif /* BACKTRACE_H */
