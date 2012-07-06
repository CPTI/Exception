#ifndef ISTACKADDRESSLOADER_H
#define ISTACKADDRESSLOADER_H

#include <stdint.h>

namespace Backtrace {

	// Interface for the stack provide backend.

	class IStackAddresLoader {
	public:
		virtual ~IStackAddresLoader() {}

		// Returns the current callstack starting at the calling function
		// At most depth addresses are loaded. The return value is the
		// actual number of stack addresses. The function may overwrite the
		// area of memory between addrs[ret_val-1] and addres[depth-1]
		virtual int getStack(int depth, void** addrs) = 0;

	};

	IStackAddresLoader& getPlatformStackLoader();

}

#endif // ISTACKADDRESSLOADER_H
