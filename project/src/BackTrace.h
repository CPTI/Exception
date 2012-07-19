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

		StackTrace() : m_debugSmbolsLoaded(false), m_referenceCount(1) {}

		~StackTrace() {}

		std::string asString(bool loadDebug = false);

		static std::string asString(int depth, const StackFrame* frames);

		bool isDebugLoaded() { return m_debugSmbolsLoaded; }

		void loadDebug();

		std::vector<StackFrame>& getFrames() { return m_frames; }

		void increaseCount();

		void decreaseCount();

	private:
		bool m_debugSmbolsLoaded;
		int m_referenceCount;
		std::vector<StackFrame> m_frames;
	};

	void initialize(const char* argv0);
	StackTrace* trace();
	bool backtraceSupported();

}

#endif /* BACKTRACE_H */
