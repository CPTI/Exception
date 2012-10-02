#include "Exception.h"

#include "BackTrace.h"
#include "Demangling.h"
#include "StackAddressLoader.h"
#include "DebugSymbolLoader.h"
#include "Logger.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typeinfo>

using namespace std;


static bool initialized = false;

#if __GNUC__

#include <cxxabi.h>

struct padding {
	void* ptr;
};

static const int MAX_FRAMES = 16;
static const int INTERCEPT_SKIP = 1;

struct frames {
	typedef void (*destructor)(void*);
	int size;
	Backtrace::StackFrame* frms;
	destructor dtor;
	union {
		char buffer[sizeof(Backtrace::StackFrame)*MAX_FRAMES];
		padding p;
	};
};

static __thread frames localFrames = { 0 , 0, 0, {{0}}};

namespace {
	void create_frames()
	{
		Backtrace::StackFrame* addr = reinterpret_cast<Backtrace::StackFrame*>(localFrames.buffer);
		if (localFrames.frms != addr) {
			// só um acaso muito grande ia fazer o ponteiro apontar exatamente para o lugar certo.
			localFrames.size = 0;
			localFrames.frms = addr;
			for (int i = 0; i < MAX_FRAMES;  ++i) {
				new(addr + i) Backtrace::StackFrame();
			}
		}
	}

	void destroy_frames(void *thrown_exception)
	{
		Backtrace::StackFrame* addr = reinterpret_cast<Backtrace::StackFrame*>(localFrames.buffer);
		if (localFrames.frms == addr) {
			for (int i = 0; i < MAX_FRAMES;  ++i) {
				addr[i].~StackFrame();
			}
			localFrames.dtor(thrown_exception);
		}
		localFrames.size = -1;
		localFrames.frms = NULL;
		localFrames.dtor = NULL;
	}
}

namespace ExceptionLib {
	const Backtrace::StackFrame* getBT(const std::exception& ex, size_t* depth, bool loadDebugSyms)
	{
		const ExceptionBase* base = dynamic_cast<const ExceptionBase*>(&ex);
		if (base) {
			if (base->stacktrace()) {

				if (loadDebugSyms) {
					base->stacktrace()->loadDebug();
				}
				if (depth) *depth = base->stacktrace()->getFrames().size();
				if (*depth > 0) {
					return &base->stacktrace()->getFrames()[0];
				}
			}
		} else {
			Backtrace::StackFrame* addr = reinterpret_cast<Backtrace::StackFrame*>(localFrames.buffer);
			if (localFrames.frms != addr) {
				if (depth) *depth = 0;
				return NULL;
			}
			if (depth) *depth = localFrames.size - INTERCEPT_SKIP;
			if (*depth > 0) {

				if (loadDebugSyms) {
					Backtrace::getPlatformDebugSymbolLoader().findDebugInfo(localFrames.frms, localFrames.size);
				}

				return localFrames.frms+INTERCEPT_SKIP;
			}
		}
		return NULL;
	}
}


extern "C" void * __cxa_allocate_exception(size_t size);
extern "C" void __cxa_free_exception(void *thrown_exception);

namespace {
	bool find_base(const abi::__class_type_info* actual, const abi::__class_type_info* target, ptrdiff_t* offset);

	bool recursive_find_base(const abi::__vmi_class_type_info* actual, const abi::__class_type_info* target, ptrdiff_t* offset)
	{

		for (size_t i = 0; i < actual->__base_count; ++i) {
			const abi::__class_type_info* base = actual->__base_info[i].__base_type;
			if (*base == *target) {
				if (offset) *offset += actual->__base_info[i].__offset();
				return true;
			}
			int _offset = 0;
			if (find_base(base, target, offset)) {
				if (offset) *offset += actual->__base_info[i].__offset() + _offset;
				return true;
			}
		}
		return false;
	}

	bool recursive_find_base(const abi::__si_class_type_info* actual, const abi::__class_type_info* target, ptrdiff_t* offset)
	{
		if (*actual->__base_type == *target) {
			return true;
		}
		return find_base(actual->__base_type, target, offset);
	}

	bool find_base(const abi::__class_type_info* actual, const abi::__class_type_info* target, ptrdiff_t* offset)
	{
		if (offset) *offset = 0;
		if (*actual == *target) {
			return true;
		}
		const abi::__vmi_class_type_info* vmactual = dynamic_cast<const abi::__vmi_class_type_info*>(actual);
		if (vmactual) {
			return recursive_find_base(vmactual, target, offset);
		}
		const abi::__si_class_type_info* sactual = dynamic_cast<const abi::__si_class_type_info*>(actual);
		if (sactual) {
			return recursive_find_base(sactual, target, offset);
		}
		return false;
	}

}

extern "C" void __real___cxa_throw( void* thrown_exception,
									const std::type_info* tinfo, void ( *dest )( void* ) )
__attribute__(( noreturn ));

extern "C" void __wrap___cxa_throw( void* thrown_exception,
									const std::type_info* tinfo, void ( *dest )( void* ) )
{
	const abi::__class_type_info* cinfo = dynamic_cast<const abi::__class_type_info*>(tinfo);

	if (cinfo != NULL) {
		const abi::__class_type_info& exclass = dynamic_cast<const abi::__class_type_info&>(typeid(ExceptionLib::ExceptionBase));

		if ( find_base(cinfo, &exclass, NULL)) {
			__real___cxa_throw( thrown_exception, tinfo, dest );
			return;
		}

		const abi::__class_type_info& stdexclass = dynamic_cast<const abi::__class_type_info&>(typeid(std::exception));

		if ( find_base(cinfo, &stdexclass, NULL)) {
			create_frames();
			localFrames.size = Backtrace::getPlatformStackLoader().getStack(MAX_FRAMES, localFrames.frms);
			localFrames.dtor = dest;
			__real___cxa_throw( thrown_exception, tinfo, destroy_frames );
			return;
		}
	}
	__real___cxa_throw( thrown_exception, tinfo, dest );
}

#else

namespace ExceptionLib {
	const Backtrace::StackFrame* getBT(const std::exception&, size_t* depth, bool)
	{
		if (depth) * depth = 0;
		return NULL;
	}
}

#endif

namespace ExceptionLib {

	static const size_t SKIP_FRAMES = 4;

	static bool stackEnabled = true;


	void terminate_handler()
	{
#ifdef __GNUC__
		static bool tried_throw = false;

		try {
			if (!tried_throw) {
				tried_throw = true;
				throw;
			}
			cerr << "no active exception" << endl;
		}
		catch (const std::exception &ex) {
			Log::LoggerFactory::getLogger("root").log(Log::LERROR, "Unhandled exception:\n%1", ex);
		}
		catch (...) {
			Log::LoggerFactory::getLogger("root").log(Log::LERROR, "Unknown unhandled exception");
		}
#else
		Log::LoggerFactory::getLogger("root").log(Log::LERROR, "Unhandled exception");
#endif
	}

	void init(char *argv0)
	{
		::Backtrace::initialize(argv0);
		set_terminate(terminate_handler);
		initialized = true;
	}

	ExceptionBase::ExceptionBase(const ExceptionBase& that)
		: BaseExceptionType()
		, m_raiser(that.m_raiser)
		, m_cloner(that.m_cloner)
		, st(NULL)
		, m_what(that.m_what)
		, m_nested(NULL)
	{
		if (that.m_nested) {
			m_nested = that.m_nested->clone();
		}
		if (that.st) {
			st = that.st;
			st->increaseCount();
		}
	}

	ExceptionBase::~ExceptionBase() throw ()
	{
		try {
			if (m_nested) {
				delete m_nested;
			}
			if (st) st->decreaseCount();
			st = NULL;
		} catch (...) {}
	}


	void ExceptionBase::raise() const
	{
		m_raiser(this);
	}

	ExceptionBase* ExceptionBase::clone() const
	{
		return m_cloner(this);
	}

	const char* ExceptionBase::what() const throw()
	{
		return m_what.c_str();
	}

	const ExceptionBase* ExceptionBase::nested() const
	{
		return m_nested;
	}

	Backtrace::StackTrace* ExceptionBase::stacktrace() const
	{
		if (st) {
			return st;
		} else {
			return NULL;
		}
	}

	void NOINLINE ExceptionBase::setup(bool enableTrace, const ExceptionBase* nested)
	{
		if (nested) {
			m_nested = nested->clone();
		}
		if (stackEnabled && enableTrace) {
			st = ::Backtrace::trace();
			if (st) {
				std::vector<Backtrace::StackFrame>& frames = st->getFrames();
				if (frames.size() > SKIP_FRAMES) {
					rotate(frames.begin(), frames.begin()+SKIP_FRAMES, frames.end());
					frames.resize(frames.size()-SKIP_FRAMES);
				} else {
					frames.resize(0);
				}
			}
		}
	}

	Exception::Exception(const Exception& that) :
		ExceptionBase(that)
	{	}


	void stacktraceEnabled(bool enable) {
		stackEnabled = enable;
	}
}
