#ifndef SYMBOLCACHE_H
#define SYMBOLCACHE_H

#include "BackTrace.h"

#include <QHash>
#include <QReadWriteLock>

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

		typedef QHash<void*, CachedFrame> Cache;

		Cache m_cache;
		mutable QReadWriteLock m_lock;
	};

}

#endif // SYMBOLCACHE_H
