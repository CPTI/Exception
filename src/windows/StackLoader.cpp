#include "IStackAddressLoader.h"

#include <windows.h>
#include <winnt.h>
#include <imagehlp.h>

class WindowsStacktraceLoader: public Backtrace::IStackAddresLoader {

    virtual int getStack(int depth, void** addrs) {

        CONTEXT context;
        memset(&context, 0, sizeof(CONTEXT));
        context.ContextFlags = CONTEXT_FULL;

        HANDLE thread = GetCurrentThread();
        bool success = GetThreadContext(thread, &context);

        if (!success) {
            return 0;
        }

        STACKFRAME frame;
        memset(&frame,0,sizeof(frame));

        frame.AddrPC.Offset = context.Eip;
        frame.AddrPC.Mode = AddrModeFlat;
        frame.AddrStack.Offset = context.Esp;
        frame.AddrStack.Mode = AddrModeFlat;
        frame.AddrFrame.Offset = context.Ebp;
        frame.AddrFrame.Mode = AddrModeFlat;

        HANDLE process = GetCurrentProcess();

        int i = 0;
        int skip = 2;

        while(StackWalk(IMAGE_FILE_MACHINE_I386,
            process,
            thread,
            &frame,
            &context,
            0,
            SymFunctionTableAccess,
            SymGetModuleBase, 0)) {

            if (skip-- > 0) {
                continue;
            }
            addrs[i++] = reinterpret_cast<void*>(frame.AddrPC.Offset);

            if (i >= depth)
                break;
        }
        return i;
    }
};

namespace Backtrace {
    IStackAddresLoader& getPlatformStackLoader()
    {
        static WindowsStacktraceLoader instance;
        return instance;
    }
}
