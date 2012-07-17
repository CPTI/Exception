#ifndef DEMANGLING_H
#define DEMANGLING_H

#include <string>

namespace Demangling {
    bool demangle(const char* input, std::string& out);
}


#endif // DEMANGLING_H
