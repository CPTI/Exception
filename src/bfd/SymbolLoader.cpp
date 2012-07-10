#include "SymbolLoader.h"

#include "bfd.h"
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <vector>
#include <sstream>
#include <iterator>
#include <sys/stat.h>
#include <map>

#include <iostream>
using namespace std;

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

}


namespace Backtrace {

	class BFDSymbolLoader: public ISymbolLoader {
		typedef std::map<string, bfd*> context_map;
	public:

		~BFDSymbolLoader() {
			context_map::iterator it = m_contexts.begin();
			for (; it != m_contexts.end(); ++it) {
				bfd_close(it->second);
			}
		}

		virtual void findSymbol(StackFrame* frames, int nFrames)
		{
			if (executable_found) {


			}

		}

		virtual void findSymbolAndDebugInfo(StackFrame* frames, int nFrames)
		{

		}

	private:
		context_map m_contexts;


	};

	ISymbolLoader& getPlatformSymbolLoader()
	{
		static BFDSymbolLoader instance;
		return instance;
	}

	void initializeExecutablePath(const char* argv0) {
	#ifndef NO_ADDR2LINE
			Finder finder;
			string rel_path(argv0);
			executable_found = finder.whereis(rel_path, executable);
	#endif
	}

}
