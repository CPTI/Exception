#include "StackAddressLoader.h"
#include "SymbolCache.h"

#include <algorithm>
#include <string.h>
#include <string>
#include <execinfo.h>
#include "Demangling.h"
using namespace std;

namespace Backtrace {
	using namespace BacktracePrivate;

	class LinuxStacktraceLoader: public Backtrace::IStackAddresLoader {

		virtual int getStack(int depth, StackFrame* frames) {
			const int MAX_STACK = 32;

			depth = std::min(MAX_STACK, depth);
			void* addrs[MAX_STACK];
			void* addrs_to_load[MAX_STACK];
			int index[MAX_STACK];

			const int effDepth = backtrace(addrs, depth);

			// heuristica: vou de baixo para cima até achar o primeiro simbolo desconhecido
			int symbolLoadDepth=0;

			// pula o frame deste metodo
			for (int i = 1; i < effDepth; ++i) {
				const SymbolCache::CachedFrame* frame = SymbolCache::instance().cachedFor(addrs[i]);
				if (frame == NULL || frame->state == SymbolCache::NothingLoaded) {
					addrs_to_load[symbolLoadDepth] = addrs[i];
					index[symbolLoadDepth] = i-1;
					symbolLoadDepth++;
				} else {
					frames[i-1] = *frame;
				}
			}

			char **strings = backtrace_symbols (addrs_to_load, symbolLoadDepth);

			for (int i = 0; i < symbolLoadDepth; i++) {

				StackFrame& frame = frames[index[i]];

				frame.addr = addrs_to_load[i];

				bool success = false;
				char * begin = strstr(strings[i], "(");

				if (begin) {

					frame.imageFile.clear();
					frame.imageFile.append(strings[i], begin-strings[i]);

					++begin;
					if (strncmp(begin, "_Z", 2) != 0) {
						continue;
					}

					char * pos = 0;

					for (char * c = begin+1; *c != '\0'; ++c) {
						if (!(isalnum(*c) || *c == '_')) {
							pos = c;
							break;
						}
					}
					if (pos) {
						char c = *pos;
						*pos = 0;

                        string demangled;

                        bool dem_success = Demangling::demangle(begin, demangled);

						if (dem_success) {
							frame.function = demangled;
							success = true;
						}
						*pos = c;
					}
				}
				if (!success) {
					frame.function = strings[i];
				}
				SymbolCache::instance().updateCache(&frame, SymbolCache::AddressLoaded);
            }
			free (strings);

			return effDepth - 1;
		}
	};

	IStackAddresLoader& getPlatformStackLoader()
	{
		static LinuxStacktraceLoader instance;
		return instance;
	}

}
