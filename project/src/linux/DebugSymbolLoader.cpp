#include "DebugSymbolLoader.h"
#include "SymbolCache.h"

#include <cxxabi.h>

#include <iterator>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <execinfo.h>
#include <string.h>

#include <sys/stat.h>
#include <fstream>
#include <memory>
#include <ext/stdio_filebuf.h>

#include <QThreadStorage>

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


	bool bidirectional_popen(std::vector<const char*>& command, int* in, int*out) {

		int pipein[2];
		int pipeout[2];

		if (pipe(pipein) == -1 || pipe(pipeout) == -1) {
			return false;
		}

		int pid = fork();

		if (pid == 0) {
			// sou o filho, o pipein é meu output

			close(pipein[0]);
			close(pipeout[1]);

			if (dup2(pipein[1],STDOUT_FILENO) == -1 || dup2(pipeout[0],STDIN_FILENO)) {
				close(pipein[1]);
				close(pipeout[0]);
				return false;
			}

			command.push_back(NULL);
			if (execv(command[0], const_cast<char**>(&command[1])) == -1) {
				close(pipein[1]);
				close(pipeout[0]);
				return false;
			}
			return true;
		} else {
			// sou o pai
			close(pipein[1]);
			close(pipeout[0]);
			*in = pipein[0];
			*out = pipeout[1];
			return true;
		}

		return false;
	}
}


namespace Backtrace {
	using namespace BacktracePrivate;

	class Addr2LineSymbolLoader: public IDebugSymbolLoader {

		ostream a2lout;
		istream a2lin;

		auto_ptr<__gnu_cxx::stdio_filebuf<char> > outfb;
		auto_ptr<__gnu_cxx::stdio_filebuf<char> > infb;

	public:

		Addr2LineSymbolLoader() : a2lout(NULL), a2lin(NULL) {
			//initializePipes();
		}
		~Addr2LineSymbolLoader() {
		}

		void initializePipes() {
			if (executable_found) {
				std::vector<const char*> command;
				command.push_back("/usr/bin/addr2line");
				command.push_back("addr2line");
				command.push_back("-Cife");
				command.push_back(executable.c_str());

				int in, out;

				if (bidirectional_popen(command, &in, &out)) {
					outfb.reset(new  __gnu_cxx::stdio_filebuf<char>(out, ios_base::out));
					infb.reset(new __gnu_cxx::stdio_filebuf<char>(in, ios_base::in));

					a2lout.rdbuf(outfb.get());
					a2lin.rdbuf(infb.get());
				}
			}
		}


		virtual bool findDebugInfo(StackFrame* frames, int nFrames) {

			if (executable_found && nFrames > 0) {
				if (!a2lout.good() || !a2lin.good()) {
					initializePipes();
				}

				std::vector<size_t> misses;
				misses.reserve(nFrames);

				for (int i = 0; i < nFrames; i++) {
					misses.push_back(i);
					a2lout << frames[i].addr << "\n";

					const SymbolCache::CachedFrame* frame = SymbolCache::instance().cachedFor(frames[i].addr);
					if (frame && frame->state == SymbolCache::SymbolsLoaded) {
						frames[i] = *frame;
					} else {
						misses.push_back(i);
						a2lout << frames[i].addr << "\n";
					}
				}
				a2lout.flush();

				if (misses.size() == 0) {
					return true;
				}
				bool status = false;
				for (size_t i = 0; a2lin.good() && i < misses.size(); ++i) {
					try {
						StackFrame& frame = frames[misses[i]];

						string function;
						getline(a2lin, function);

						string lineinfo;
						getline(a2lin, lineinfo);
						int colon = lineinfo.find_last_of(':');
						lineinfo.replace(colon, 1, 1, ' ');
						stringstream ss(lineinfo);

						ss >> frame.sourceFile;
						ss >> frame.line;
						std::swap(function, frame.function);
						frame.imageFile = executable;
						SymbolCache::instance().updateCache(&frame, SymbolCache::SymbolsLoaded);
						status = true;
					} catch (...) {
						// o stream pode ter fechado nesse frame
					}
				}
				return status;

			} else {
				return true;
			}
			return false;
		}
	};


	IDebugSymbolLoader& getPlatformDebugSymbolLoader()
	{
		static QThreadStorage<Addr2LineSymbolLoader*> storage;

		if (!storage.hasLocalData()) {
			storage.setLocalData(new Addr2LineSymbolLoader());
		}
		return *storage.localData();
	}

	void initializeExecutablePath(const char* argv0) {
		Finder finder;
		string rel_path(argv0);
		executable_found = finder.whereis(rel_path, executable);
	}
}


