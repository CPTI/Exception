#include "StackAddressLoader.h"

#include <windows.h>
#include <winnt.h>
#include <imagehlp.h>

// Substituir por std::mutex
#include <QMutex>

#include <string>
using namespace std;

namespace Backtrace {

    class WindowsStacktraceLoader: public Backtrace::IStackAddresLoader {
        QMutex m_mutex;
    public:
        WindowsStacktraceLoader() {
            SymInitialize(GetCurrentProcess(), 0, TRUE);
        }

        virtual ~WindowsStacktraceLoader() {
            SymCleanup(GetCurrentProcess());
        }

        virtual int getStack(int depth, Backtrace::StackFrame* frames) {
            QMutexLocker locker(&m_mutex);

            char procname[MAX_PATH];
            GetModuleFileNameA(NULL, procname, sizeof procname);

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

            const int SYMBUF = 512;
            char symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + SYMBUF];
            char module_name_raw[MAX_PATH];

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

                DWORD module_base = SymGetModuleBase(process, frame.AddrPC.Offset);

                GetModuleFileNameA((HINSTANCE)module_base, module_name_raw, MAX_PATH);

                IMAGEHLP_SYMBOL* symbol = reinterpret_cast<IMAGEHLP_SYMBOL*>(symbol_buffer);
                symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
                symbol->MaxNameLength = SYMBUF-1;
                DWORD dummy = 0;

                if (SymGetSymFromAddr(process, frame.AddrPC.Offset, &dummy, symbol)) {
                    frames[i].function = symbol->Name;
                } else {
                    frames[i].function.clear();
                }

                frames[i].addr = reinterpret_cast<void*>(frame.AddrPC.Offset);
                frames[i].imageFile = module_name_raw;

                i++;
                if (i >= depth)
                    break;
            }
            return i;
        }
    };

    IStackAddresLoader& getPlatformStackLoader()
    {
        static WindowsStacktraceLoader instance;
        return instance;
    }
}
