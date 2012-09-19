#include "BackTrace.h"
#include "DebugSymbolLoader.h"

// This file contains the platform independent parts of Backtrace.h's implementation

namespace Backtrace {

	std::string StackTrace::asString(bool loadDebugSyms, int skip)
	{
		if (loadDebugSyms && !m_debugSmbolsLoaded) {
			loadDebug();
		}
		return asString(m_frames.size(), &m_frames[0], skip);
	}

	std::string StackTrace::asString(int depth, const StackFrame* frames, int skip)
	{
		std::stringstream ss;
		for (int i = skip; i < depth; ++i) {
			ss << frames[i].addr << ":  " << frames[i].function << " in (" << frames[i].imageFile << ")";
			if (frames[i].line >= 0) {
				ss << " at " << frames[i].sourceFile << ": " << frames[i].line;
			}
			ss << "\n";
		}
		return ss.str();
	}

	void StackTrace::loadDebug()
	{
		if (!m_debugSmbolsLoaded) {
			m_debugSmbolsLoaded = true;
			getPlatformDebugSymbolLoader().findDebugInfo(&m_frames[0], m_frames.size());
		}
	}


	void StackTrace::increaseCount()
	{
		++m_referenceCount;
	}

	void StackTrace::decreaseCount()
	{
		--m_referenceCount;
		if (m_referenceCount <= 0) {
			delete this; // o destrutor é virtual
		}
	}

}
