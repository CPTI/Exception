#include "Demangling.h"

#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <imagehlp.h>
#endif

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace {

#ifdef WIN32
    bool canBeMSName(const char* input) {
        return *input == '?';
    }

    bool demangleMS(const char* input, std::string& out) {
        char buffer[1024];
        DWORD result = UnDecorateSymbolName(input, buffer, sizeof(buffer), UNDNAME_COMPLETE);
        if (result != 0) {
            out.clear();
            out.append(buffer, result);
            return true;
        }
        return false;
    }

#endif

#ifdef __GNUC__
    bool canBeGNUName(const char* input) {
        return (strncmp(input, "_Z", 2)==0);
    }

    bool demangleGNU(const char* input, std::string& out) {
        int status = 0;

        char * demangled = abi::__cxa_demangle(input, NULL, NULL, &status);

        if (status == 0) {
            out.clear();
            out.append(demangled);
        }
        if (demangled) {
            free(demangled);
        }
        return (status == 0);
    }

#endif

}

namespace Demangling {
    bool demangle(const char* input, std::string& out)
    {
#ifdef WIN32
        if (canBeMSName(input)) {
            return demangleMS(input, out);
        }
#endif

#ifdef __GNUC__
        if (canBeGNUName(input)) {
            return demangleGNU(input, out);
        }
#endif

        return false;
    }
}
