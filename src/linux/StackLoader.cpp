#include "IStackAddressLoader.h"

#include  <execinfo.h>
#include <algorithm>
#include <string.h>

class LinuxStacktraceLoader: public Backtrace::IStackAddresLoader {

	static const int MAX_STACK = 32;

	virtual int getStack(int depth, void** addrs) {
		int effDepth = backtrace(addrs, depth);
		// remove o endereço desta função.
		memmove(addrs, addrs+1, (effDepth-1)*sizeof(void*));
		return effDepth - 1;
	}
};

namespace Backtrace {
	IStackAddresLoader& getPlatformStackLoader()
	{
		static LinuxStacktraceLoader instance;
		return instance;
	}

}
