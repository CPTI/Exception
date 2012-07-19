#include "DebugSymbolLoader.h"


#include <cxxabi.h>

#include <iterator>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <execinfo.h>
#include <string.h>

#include <sys/stat.h>


using namespace std;

namespace {
	static bool executable_found = false;
	static string executable;



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

}


namespace Backtrace {

	class Addr2LineSymbolLoader: public IDebugSymbolLoader {
	public:



		virtual bool findDebugInfo(StackFrame* frames, int nFrames) {

			if (executable_found && nFrames > 0) {

				stringstream command;
				command << "addr2line -Cife ";
				command << executable;

				for (int i = 0; i < nFrames; i++) {
					command << " " << frames[i].addr;
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
								line[read-1] = '\0';
								func_name = false;
								frames[i].function = line;
							} else {
								func_name = true;
								frames[i].imageFile = executable;

								char* pos = strstr(line, ":");
								if (pos) {
									*pos = 0;

									frames[i].sourceFile = line;
									frames[i].line = atoi(pos+1);
								}

								i++;
							}
						}

						fclose(p);
						free(line);
						return true;
					}
				}
			} else {
				return true;
			}
			return false;
		}
	};


	IDebugSymbolLoader& getPlatformDebugSymbolLoader()
	{
		static Addr2LineSymbolLoader instance;
		return instance;
	}

	void initializeExecutablePath(const char* argv0) {
		Finder finder;
		string rel_path(argv0);
		executable_found = finder.whereis(rel_path, executable);
	}

}


