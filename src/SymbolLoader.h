#ifndef SYMBOLLOADER_H
#define SYMBOLLOADER_H

#include "BackTrace.h"

namespace Backtrace {

	class ISymbolLoader {
	public:
		virtual ~ISymbolLoader() {}

		virtual void findSymbol(StackFrame* frames, int nFrames) = 0;
		virtual void findSymbolAndDebugInfo(StackFrame* frames, int nFrames) = 0;
	};

	ISymbolLoader& getPlatformSymbolLoader();

	void initializeExecutablePath(const char* argv0);

}

#endif // SYMBOLLOADER_H
