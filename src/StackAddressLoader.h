#ifndef ISTACKADDRESSLOADER_H
#define ISTACKADDRESSLOADER_H

#include <stdint.h>
#include "BackTrace.h"

namespace Backtrace {

	// Interface for the stack provide backend.

	class IStackAddresLoader {
	public:
		virtual ~IStackAddresLoader() {}

		// Returns the current callstack starting at the calling function
		// At most depth addresses are loaded. The return value is the
        // actual number of stack addresses loaded. Only the addresses are
        // guaranteed to be loaded, But implementations can load the function
        // name and the module file name.
		virtual int getStack(int depth, StackFrame* frames) = 0;

	};

	IStackAddresLoader& getPlatformStackLoader();

}

#endif // ISTACKADDRESSLOADER_H
