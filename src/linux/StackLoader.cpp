#include "StackAddressLoader.h"

#include <algorithm>
#include <string.h>
#include <cxxabi.h>
#include "Demangling.h"

namespace Backtrace {

	class LinuxStacktraceLoader: public Backtrace::IStackAddresLoader {


		virtual int getStack(int depth, StackFrame* frames) {
			const int MAX_STACK = 32;

			depth = std::min(MAX_STACK, depth);
			void* addrs[MAX_STACK];

			int effDepth = backtrace(addrs, depth);

			///////////////////////////////////////////////////////

			char **strings = backtrace_symbols (addrs, effDepth);



			// pula o frame deste metodo
			for (int i = 1; i < effDepth; i++) {

				frames[i-1].addr = addrs[i];

				bool success = false;
				char * begin = strstr(strings[i], "(");

				if (begin) {

					frames[i-1].imageFile.clear();
					frames[i-1].imageFile.append(strings[i], begin-strings[i]);

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

                        if (dem_success == 0 ) {
							frames[i-1].function = demangled;
							success = true;
						}
						*pos = c;
					}
				}
				if (!success) {
					frames[i-1].function = strings[i];
				}
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
