#include "BackTrace.h"
#include "StackAddressLoader.h"
#include "DebugSymbolLoader.h"

#include <memory>

namespace Backtrace {
    void initialize(const char* argv0) {
        initializeExecutablePath(argv0);
	}

	bool backtraceSupported()
	{
		return true;
	}
}
