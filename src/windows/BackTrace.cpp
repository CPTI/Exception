#include "../Bactrace.h"

namespace {
	class BackTraceWindows: public StackWalker, public ::Backtrace::StackTrace
{
public:
	BackTraceWindows() : m_foundMe(false) {
		this->ShowCallstack();
	}

	std::string getTrace() const {
		return m_trace;
	}

protected:
	virtual void OnOutput(LPCSTR szText)
	{
		std::string text(szText);

		// Elimina um monte de lixo que ninguem quer ver
		if (!m_foundMe) {
			if (text.find("StackTrace") != std::string::npos) {
				m_foundMe = true;
			}
		} else {
			m_trace += szText;
		}
	}
private:
	bool m_foundMe;
	std::string m_trace;
};
}

namespace Backtrace {
	void initialize(const char*) {}

	StackTrace* trace()
	{
		return new BackTraceWindows();
	}

	bool backtraceSupported()
	{
		return true;
	}
}