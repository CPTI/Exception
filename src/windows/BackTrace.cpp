#include "BackTrace.h"
#include "StackAddressLoader.h"
#include "DebugSymbolLoader.h"

#include <memory>

namespace Backtrace {
    void initialize(const char* argv0) {
        initializeExecutablePath(argv0);
	}

	StackTrace* trace()
	{
		const int MAX_STACK = 32;
		std::auto_ptr<StackTrace> trace(new StackTrace());
		trace->getFrames().resize(MAX_STACK);
		const int num = getPlatformStackLoader().getStack(MAX_STACK, &trace->getFrames()[0]);
		trace->getFrames().resize(num);
		getPlatformDebugSymbolLoader().findDebugInfo(&trace->getFrames()[0], num);

		return trace.release();
	}

	bool backtraceSupported()
	{
		return true;
	}
}
