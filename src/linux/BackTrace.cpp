#include "../BackTrace.h"
#include "../Exception.h"

#include <errno.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <cxxabi.h>

#include <iterator>
#include <vector>
#include <sstream>

#ifndef NO_ADDR2LINE
#include <sys/stat.h>
#endif

using namespace std;
using namespace ExceptionLib;

namespace {
	static bool executable_found = false;
	static string executable;

#ifndef NO_ADDR2LINE


	char * opt_getcwd (char * stack_buf, int stack_size)
	{
		if (getcwd (stack_buf, stack_size) == stack_buf)
			return stack_buf;
		size_t size = stack_size;

		while (1)
		{
			char *buffer = (char *) malloc (size);
			if (getcwd (buffer, size) == buffer)
				return buffer;
			free (buffer);
			if (errno != ERANGE)
				return 0;
			size *= 2;
		}
	}

	class Finder {
	private:
		vector<string> pathv;
	public:
		Finder() {
			string path(getenv("PATH"));
			const int size = path.size();
			for (int i = 0; i < size; ++i) {
				if (path[i] == ':') path[i] = ' ';
			}
			stringstream splitter;
			splitter << path;

			istream_iterator<string> it(splitter);
			istream_iterator<string> _end;
			for (; it != _end; ++it) {
				pathv.push_back(*it + "/");
			}
		}
		inline bool check_exe_file(const string& path) {
			struct stat statbuf;
			if(!stat(path.c_str(), &statbuf)) {
				if ((statbuf.st_mode & S_IXUSR) || (statbuf.st_mode & S_IXGRP) || (statbuf.st_mode & S_IXOTH)) {
					return true;
				}
			}
			return false;
		}
		bool whereis(const string& file, string& resolved) {

			if (file.size() > 0 && file[0] == '/') {
				if (check_exe_file(file)) {
					resolved = file;
					return true;
				}
				return false;
			}

			char stack_buf[100];
			char * cwd = opt_getcwd(stack_buf, 100);
			string local_path(cwd);
			if (cwd != stack_buf)
				free(cwd);
			local_path += '/';
			local_path += file;
			if (check_exe_file(local_path)) {
				resolved = local_path;
				return true;
			}

			vector<string>::iterator it = pathv.begin();
			vector<string>::iterator end = pathv.end();

			for (; it != end; ++it) {
				string name(*it + file);
				if (check_exe_file(name)) {
					resolved = name;
					return true;
				}
			}

			return false;
		}
	};
#endif


	class StackTraceLinux: public ::Backtrace::StackTrace {
	private:
		void * m_bt[50];
		ssize_t m_size;

	public:
		StackTraceLinux() {
			m_size = backtrace(m_bt, sizeof(m_bt)/sizeof(void*));
		}

		static string resolveSymbols(void* const addresses[], ssize_t size) {
			stringstream stack;
			bool first_way_worked = false;

#ifndef NO_ADDR2LINE

			if (executable_found) {

				stringstream command;
				command << "addr2line -Cife ";
				command << executable;

				for (int i = 0; i < size; i++) {
					command << " " << addresses[i];
				}
				command << " 2>/dev/null";

				FILE *p = popen(command.str().c_str(), "r");

				if (p) {
					// workaround
					int c = fgetc(p);
					if (c != EOF) ungetc(c, p);

					if (!feof(p)) {
						size_t lsize = 256;
						char * line = (char*) malloc(lsize);
						ssize_t read;

						bool func_name = true;

						int i = 0;
						while ((read = getline(&line, &lsize, p)) > 0) {
							if (func_name) {
								line[read-1] = ' ';
								func_name = false;
								stack << addresses[i++] << ":\t" << line << "\tat ";
							} else {
								func_name = true;
								stack << line;
							}
						}

						fclose(p);
						free(line);
						first_way_worked = true;
					}
				}
			}
#endif /* NO_ADDR2LINE */

			if (!first_way_worked) {
				size_t length = 50;
				int status;
				char * demangled = (char*)malloc(length);

				char **strings = backtrace_symbols (addresses, size);
				for (int i = 0; i < size; i++) {
					bool success = false;
					char * begin = strstr(strings[i], "_Z");
					if (begin) {
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
							demangled = abi::__cxa_demangle(begin, demangled, &length, &status);
							if (status == 0 ) {
								*pos = c;
								// vamos substituir a string mangled
								string s;
								s.insert(s.end(), strings[i], begin);
								s.append(demangled);
								s.append(pos);

								stack << s << endl;
								success = true;



							} else {
								*pos = c;
							}
						}
					}
					if (!success) {
						stack << strings[i] << endl;
					}

				}

				free (strings);
				if (demangled) {
					free(demangled);
				}

			}
			return stack.str();
		}

		string getTrace() const {
			return resolveSymbols(m_bt, m_size);
		}
	};

	void segfaulthandler(int signum, siginfo_t * info, void*)
	{
		stringstream ss;

		void * addr = info->si_addr;
		int code = info->si_code;

		switch(signum) {
			case SIGSEGV:
			{
				ss << "Segmentation fault at: " << addr;
				if (code & SEGV_MAPERR) {
					ss << ", address not mapped to object";
				} else if (code & SEGV_ACCERR) {
					ss << ", invalid permissions for mapped object";
				}
				throw SegmentationFault(ss.str());
				break;
			}
			case SIGBUS:
			{
				ss << "Bus error at: " << addr;
				if (code & BUS_ADRALN) {
					ss << ", invalid address alignment";
				} else if (code & BUS_ADRERR) {
					ss << ", nonexistent physical address";
				} else if (code & BUS_OBJERR) {
					ss << ", object-specific hardware error";
				}
				throw SegmentationFault(ss.str());
				break;
			}
			case SIGFPE:
			{
				ss << "Floating point error at: " << addr;
				if (code & FPE_INTDIV) {
					ss << ", integer divide by zero";
				} else if (code & FPE_INTOVF) {
					ss << ", integer overflow";
				} else if (code & FPE_FLTDIV) {
					ss << ", floating-point divide by zero";
				} else if (code & FPE_FLTOVF) {
					ss << ", floating-point overflow";
				} else if (code & FPE_FLTUND) {
					ss << ", floating-point underflow";
				} else if (code & FPE_FLTRES) {
					ss << ", floating-point inexact result";
				} else if (code & FPE_FLTINV) {
					ss << ", floating-point invalid operation";
				} else if (code & FPE_FLTSUB) {
					ss << ", subscript out of range";
				}
				throw FloatingPointException(ss.str());
				break;
			}
			case SIGILL:
			{
				ss << "Illegal instruction at: " << addr;
				if (code & ILL_ILLOPC) {
					ss << ", illegal opcode";
				} else if (code & ILL_ILLOPN) {
					ss << ", illegal operand";
				} else if (code & ILL_ILLADR) {
					ss << ", illegal addressing mode";
				} else if (code & ILL_ILLTRP) {
					ss << ", illegal trap";
				} else if (code & ILL_PRVOPC) {
					ss << ", privileged opcode";
				} else if (code & ILL_PRVREG) {
					ss << ", privileged register";
				} else if (code & ILL_COPROC) {
					ss << ", coprocessor error";
				} else if (code & ILL_BADSTK) {
					ss << ", internal stack error";
				}
				throw IllegalInstruction(ss.str());
				break;
			}
			default:
			{
				ss << "Caught unexpected signal: " << info->si_signo << addr;
				throw Exception(ss.str());
				break;
			}
		}
	}

}

#include <iostream>
using namespace std;

namespace Backtrace {
	void initialize(const char* argv0)
	{
#ifndef NO_ADDR2LINE
		Finder finder;
		string rel_path(argv0);
		executable_found = finder.whereis(rel_path, executable);
#endif

		struct sigaction action;

		action.sa_handler = 0;
		action.sa_sigaction = segfaulthandler;
		sigemptyset (&action.sa_mask);
		sigaddset(&action.sa_mask, SIGSEGV);
		sigaddset(&action.sa_mask, SIGBUS);
		sigaddset(&action.sa_mask, SIGILL);
		sigaddset(&action.sa_mask, SIGFPE);
		action.sa_flags = SA_SIGINFO;

		sigaction(SIGSEGV, &action, NULL);
		sigaction(SIGBUS, &action, NULL);
		sigaction(SIGILL, &action, NULL);
		sigaction(SIGFPE, &action, NULL);
	}


	StackTrace* trace()
	{
		return new StackTraceLinux();
	}

	bool backtraceSupported()
	{
		return true;
	}
}
