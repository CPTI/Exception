#include "StackAddressLoader.h"

namespace Backtrace {

    // Interface for the stack provide backend.

    class DefaultStackLoader: public IStackAddresLoader {
    public:

        // Returns the current callstack starting at the calling function
        // At most depth addresses are loaded. The return value is the
        // actual number of stack addresses loaded. The complete filesystem path
        // for each module is loaded as well, if possible
        virtual int getStack(int, StackFrame*) { return 0; }

    };

    IStackAddresLoader& getPlatformStackLoader()
    {
        static DefaultStackLoader instance;
        return instance;
    }

}
