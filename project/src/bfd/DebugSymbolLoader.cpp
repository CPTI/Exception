#include "DebugSymbolLoader.h"

#include "SymbolCache.h"

#include <bfd.h>
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>

#if __cplusplus >= 201103L
#include <mutex>
#elif defined QT_CORE_LIB
#include <QMutex>
#include <QMutexLocker>
#endif

#include "Demangling.h"


using namespace std;

namespace Backtrace {
	using namespace BacktracePrivate;

	class BFDSymbolLoader: public IDebugSymbolLoader {

		struct BFD_context {
			bfd* handle;
			vector<bfd_symbol*> symbols;

			bool valid;
			BFD_context() : handle(NULL), symbols(0), valid(false) {}
		};

		typedef std::map<string, BFD_context> context_map;
#if __cplusplus >= 201103L
        typedef std::mutex mutex_t;
        struct mutex_locker_t {
            std::lock_guard<mutex_t> guard;
            mutex_locker_t(mutex_t* l) : guard(*l) {}
        };
#elif defined QT_CORE_LIB
        typedef QMutex mutex_t;
        typedef QMutexLocker mutex_locker_t;
#endif
	public:

		BFDSymbolLoader()
        {
		}

		~BFDSymbolLoader() {
			context_map::iterator it = m_contexts.begin();
			for (; it != m_contexts.end(); ++it) {
				BFD_context& ctx = it->second;
				if (ctx.valid) {
					bfd_close(ctx.handle);
				}
			}
		}

		virtual bool findDebugInfo(StackFrame* frames, int nFrames)
		{
			for (int i = 0; i < nFrames; ++i) {

				const SymbolCache::CachedFrame* frame = SymbolCache::instance().cachedFor(frames[i].addr);

				if (frame && frame->state == SymbolCache::SymbolsLoaded) {
					frames[i] = *frame;
				} else {
					BFD_context& ctx = getBFD(frames[i].imageFile);
					if (ctx.valid) {
						string source;
						string function;
						int line;

						find(ctx, frames[i].addr, source, function, line);
						if (function != ""){
							frames[i].sourceFile = source;
							frames[i].line = line;

							if (!Demangling::demangle(function.c_str(), frames[i].function)) {
								frames[i].function = function;
							}
							SymbolCache::instance().updateCache(&frames[i], SymbolCache::SymbolsLoaded);
						}
					}
				}
			}
			return false;
		}

	private:
		context_map m_contexts;
        mutex_t m_mutex;

		BFD_context& getBFD(const std::string& moduleName) {
            mutex_locker_t locker(&m_mutex);
			context_map::iterator it = m_contexts.find(moduleName);
			if (it != m_contexts.end()) {
				return it->second;
			} else {

				BFD_context& ctx = m_contexts[moduleName]; // chama o ctor default

				bfd* context = bfd_openr(moduleName.c_str(), NULL);

				if (context) {

					const int r1 = bfd_check_format(context, bfd_object);
					const int r2 = bfd_check_format_matches(context, bfd_object, NULL);
					const int r3 = bfd_get_file_flags(context) & HAS_SYMS;

					if (r1 && r2 && r3) {
						uint64_t storage_needed = bfd_get_symtab_upper_bound(context);

						if (storage_needed > 0) {


							ctx.symbols.resize(storage_needed);
							int number_of_symbols = bfd_canonicalize_symtab(context, &ctx.symbols[0]);
							if (number_of_symbols <= 0) {
								ctx.symbols.clear();
							} else {
								ctx.handle = context;
								ctx.valid = true;
							}
						}
					}
					if (!ctx.valid) {
						bfd_close(context);
					}
				}
				return ctx;
			}
		}


		struct find_info {
			bfd_symbol** symbols;
			bfd_vma counter;
			const char *file;
			const char *func;
			unsigned line;
		};

		static void lookup_section(bfd *abfd, asection *sec, void *opaque_data)
		{
			find_info* data = reinterpret_cast<find_info*>(opaque_data);

			if (data->func)
				return;

			if (!(bfd_get_section_flags(abfd, sec) & SEC_ALLOC))
				return;

			bfd_vma vma = bfd_get_section_vma(abfd, sec);
			if (data->counter < vma || vma + bfd_get_section_size(sec) <= data->counter)
				return;

			bfd_find_nearest_line(abfd, sec, data->symbols, data->counter - vma, &(data->file), &(data->func), &(data->line));
		}


		void find(BFD_context b, void* offset, string& file, string& func, int& line)
		{
			struct find_info data;
			data.func = NULL;
			data.symbols = &b.symbols[0];
			data.counter = reinterpret_cast<bfd_vma>(offset);
			data.file = NULL;
			data.func = NULL;
			data.line = 0;

			bfd_map_over_sections(b.handle, &lookup_section, &data);
			if (data.file) {
				file = data.file;
			}
			if (data.func) {
				func = data.func;
			}
			line = data.line;
		}

	};

	IDebugSymbolLoader& getPlatformDebugSymbolLoader()
	{
		static BFDSymbolLoader instance;
		return instance;
	}

	void initializeExecutablePath(const char*) {
	}

}
