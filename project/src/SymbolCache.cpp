#include "SymbolCache.h"


namespace BacktracePrivate {
	SymbolCache::SymbolCache()
	{
	}


	SymbolCache& SymbolCache::instance()
	{
		static SymbolCache inst;
		return inst;
	}

	const SymbolCache::CachedFrame* SymbolCache::cachedFor(void* address) const {

        read_locker_t locker(&m_lock);
		Cache::const_iterator it = m_cache.find(address);
		if (it == m_cache.end()) {
			return NULL;
		} else {
#ifdef USE_CXX11
            return &(it->second);
#elif defined USE_QT
            return &(*it);
#endif
		}
	}

	void SymbolCache::updateCache(StackFrame* frame, CacheState state) {
        write_locker_t locker(&m_lock);
		CachedFrame& cframe = m_cache[frame->addr];

		if (state > cframe.state) {
			cframe.state = state;

			cframe.addr = frame->addr;
			cframe.function = frame->function;
			cframe.imageFile = frame->imageFile;

			if (state == SymbolsLoaded) {
				cframe.line = frame->line;
				cframe.sourceFile = frame->sourceFile;
			}
		}
	}

}
