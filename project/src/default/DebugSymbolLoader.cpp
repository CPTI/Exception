#include "DebugSymbolLoader.h"

namespace Backtrace {

    class DefaultDebugSymbolLoader: public Backtrace::IDebugSymbolLoader {
    public:
        virtual bool findDebugInfo(StackFrame* frames, int nFrames) { return false; }
    };

    IDebugSymbolLoader& getPlatformDebugSymbolLoader()
    {
        static DefaultDebugSymbolLoader instance;
        return instance;
    }

    void initializeExecutablePath(const char* argv0) {}
}
