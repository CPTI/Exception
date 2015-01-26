#ifndef SYMBOLLOADER_H
#define SYMBOLLOADER_H

#include "config.h"
#include "BackTrace.h"

namespace Backtrace {

	class IDebugSymbolLoader {
	public:
		virtual ~IDebugSymbolLoader() {}

		/* This method assumes that the stack addresses and the imageFile fields
		  are set. If successful, this method fills the source file and line fields. */
		virtual bool findDebugInfo(StackFrame* frames, int nFrames) = 0;
	};

	IDebugSymbolLoader& getPlatformDebugSymbolLoader();

	void initializeExecutablePath(const char* argv0);
}

#endif // SYMBOLLOADER_H
