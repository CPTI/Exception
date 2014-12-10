#ifndef SYMBOLCACHE_H
#define SYMBOLCACHE_H

#include "BackTrace.h"


#if __cplusplus >= 201103L
    #include <unordered_map>
    #include <mutex>
#elif defined QT_CORE_LIB
    #include <QHash>
    #include <QReadWriteLock>
#endif


namespace BacktracePrivate {
	using namespace Backtrace;

	class SymbolCache
	{
	public:

		enum CacheState {
			NothingLoaded = 0,
			AddressLoaded = 1,
			SymbolsLoaded = 2
		};

		struct CachedFrame: public StackFrame {
			CacheState state;

			CachedFrame() : StackFrame(), state(NothingLoaded) {}
			CachedFrame(const StackFrame& that) : StackFrame(that), state(NothingLoaded) {}
			CachedFrame(const CachedFrame& that) : StackFrame(that), state(that.state) {}
		};

		const CachedFrame* cachedFor(void* address) const;
		void updateCache(StackFrame* frame, CacheState state);

		static SymbolCache& instance();

	private:
		SymbolCache();

#if __cplusplus >= 201103L
        typedef std::unordered_map<void*, CachedFrame> Cache;
        // we'll have to wait for c++14 to use de shared_timed_mutex for read and write locks
        typedef std::mutex lock_t;
        struct Locker {
            std::lock_guard<lock_t> guard;
            Locker(lock_t* l) : guard(*l) {}
        };
        typedef Locker read_locker_t;
        typedef Locker write_locker_t;
#elif defined QT_CORE_LIB
        typedef QHash<void*, CachedFrame> Cache;
        typedef QReadWriteLock lock_t;
        typedef QReadLocker read_locker_t;
        typedef QReadLocker write_locker_t;
#endif

		Cache m_cache;
        mutable lock_t m_lock;
	};

}

#endif // SYMBOLCACHE_H
