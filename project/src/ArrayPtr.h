#ifndef ARRAYPTR_H
#define ARRAYPTR_H

#include "config.h"
#include <algorithm>
#include <stddef.h>

#include "Typelist.h"
#include "TypelistMacros.h"
#include "TypeManip.h"


#ifdef USE_CXX11
        #include <atomic>
#elif defined USE_QT
        #include <QAtomicInt>
#endif

namespace ArrayPrivate {
	template<typename T>
	class ArrayPtrTemporary {
	public:
		ArrayPtrTemporary(T* buffer) : m_buffer(buffer) {}
		T* m_buffer;
	};
}

template<typename T>
class ArrayPtr {
public:
	explicit ArrayPtr(int size)
		: m_buffer(NULL)
	{
		if (size > 0) {
			m_buffer = new T[size];
		}
	}

	ArrayPtr(T* t) : m_buffer(t) {}

	~ArrayPtr() {
		reset(NULL);
	}

	ArrayPtr(const ArrayPrivate::ArrayPtrTemporary<T> &t)
		: m_buffer(t.m_buffer)
	{}

	operator ArrayPrivate::ArrayPtrTemporary<T>()
	{
		ArrayPrivate::ArrayPtrTemporary<T> t(m_buffer);
		m_buffer = NULL;
		return t;
	}

	ArrayPtr(ArrayPtr& that)
		: m_buffer(that.m_buffer)
	{
		that.m_buffer = NULL;
	}


	void swap(ArrayPtr& that)
	{
		std::swap(m_buffer, that.m_buffer);
	}

	ArrayPtr& operator=(ArrayPtr ptr)
	{
		// A passagem por copia ja se apropriou do ponteiro
		swap(ptr);
		return *this;
	}

	T* get() const {
		return m_buffer;
	}

	T* release() {
		T* tmp = m_buffer;
		m_buffer = NULL;
		return tmp;
	}

	void reset(T* buffer) {
		if (m_buffer) {
			delete[] m_buffer;
		}
		m_buffer = buffer;
	}

	T& operator[](size_t pos) {
		return m_buffer[pos];
	}

	const T& operator[](size_t pos) const {
		return m_buffer[pos];
	}

private:
	T* m_buffer;
};


namespace ArrayPrivate {

	typedef LOKI_TYPELIST_11(
			signed short,
			signed int,
			signed long,
			signed long long,
			unsigned short,
			unsigned int,
			unsigned long,
			unsigned long long,
			float,
			double,
			size_t
			) Primitives;

	template<class T>
	struct is_pointer {
		enum { value = false };
	};

	template<class T>
	struct is_pointer<T*> {
		enum { value = true };
	};

	template<class T>
	struct is_primitive {
		enum { value = (Loki::TL::IndexOf<Primitives, T>::value != -1) };
	};

	template<class T>
	struct is_scalar {
		enum { value = (is_primitive<T>::value || is_pointer<T>::value) };
	};


	template<class T, bool ThreadSafe>
	struct alloc_count {

		struct buffer_header {
			size_t count;
			size_t size;
		};

		static void incrementCount(T* t) {
			buffer_header* header = reinterpret_cast<buffer_header*>(t) - 1;
			++header->count;
		}

		static bool decrementCount(T* t) {
			buffer_header* header = reinterpret_cast<buffer_header*>(t) - 1;
			bool ret = (header->count == 1);
			--header->count;
			return ret;
		}
	};

	template<class T>
	struct alloc_count<T, true> {

        struct buffer_header {
#ifdef USE_CXX11
            std::atomic_int count;
#elif defined USE_QT
            QAtomicInt count;
#endif
			size_t size;

            buffer_header(): count(0), size(0) {}
		};

		static void incrementCount(T* t) {
			buffer_header* header = reinterpret_cast<buffer_header*>(t) - 1;
#ifdef USE_CXX11
            ++header->count;
#elif defined USE_QT
            header->count.ref();
#endif
		}

		static bool decrementCount(T* t) {
			buffer_header* header = reinterpret_cast<buffer_header*>(t) - 1;
#ifdef USE_CXX11
            return --header->count == 0;
#elif defined USE_QT
            return !header->count.deref();
#endif
		}
	};


	template<class T, bool Scalar>
	struct initializer {

		static void initialize(T* ptr, size_t size) {
			T* end = ptr+size;
			for (; ptr < end ; ++ptr) {
				new(ptr) T();
			}
		}

		static void finalize(T* begin, size_t size) {
			T* ptr = begin+size-1;
			for (; ptr >= begin; --ptr) {
				ptr->~T();
			}
		}
	};

	template<class T>
	struct initializer<T, true> {
		static void initialize(T*, size_t) {}
		static void finalize(T*, size_t) {}
	};


	template<class T, bool ThreadSafe = false>
	struct buffer_handler: public alloc_count<T, ThreadSafe>, public initializer<T, is_scalar<T>::value >  {
	private:
		typedef typename alloc_count<T, ThreadSafe>::buffer_header buffer_header;
		typedef initializer<T, is_scalar<T>::value > init;
	public:
		static T* allocate(size_t size) {
			size_t totalSize = sizeof(buffer_header) + size*sizeof(T);
			char* buffer  = new char[totalSize];
			buffer_header* header   = reinterpret_cast<buffer_header*>(buffer);
			header->count = 0;
			header->size  = size;

			T* begin = reinterpret_cast<T*>(buffer+sizeof(buffer_header));
			init::initialize(begin, size);
			return begin;
		}

		static void deallocate(T* t) {
			char* buffer = reinterpret_cast<char*>(t) - sizeof(buffer_header);
			buffer_header* header = reinterpret_cast<buffer_header*>(buffer);

			init::finalize(t, header->size);
			delete[] buffer;
		}

		static size_t size(T* t) {
			char* buffer = reinterpret_cast<char*>(t) - sizeof(buffer_header);
			return reinterpret_cast<buffer_header*>(buffer)->size;
		}
	};
}


template<typename T, bool ThreadSafe=true>
class SharedArray {
	typedef ArrayPrivate::buffer_handler<T, ThreadSafe> alloc;
public:
	explicit SharedArray(size_t size)
		: m_buffer(NULL)
	{
		if (size > 0) {
			m_buffer = alloc::allocate(size);
			alloc::incrementCount(m_buffer);
		}
	}

	~SharedArray() {
		if (m_buffer) {
			if (alloc::decrementCount(m_buffer)) {
				alloc::deallocate(m_buffer);
			}
		}
		m_buffer = NULL;
	}

	SharedArray(const SharedArray& t)
		: m_buffer(t.m_buffer)
	{
		if (m_buffer) {
			alloc::incrementCount(m_buffer);
		}
	}

	void swap(SharedArray& that)
	{
		std::swap(m_buffer, that.m_buffer);
	}

	SharedArray& operator=(SharedArray ptr)
	{
		// A passagem por copia ja se apropriou do ponteiro
		swap(ptr);
		return *this;
	}

	typedef T* iterator;
	typedef const T* const_iterator;

	T* get() {
		return m_buffer;
	}

	const T* get() const {
		return m_buffer;
	}

	iterator begin() {
		return get();
	}

	const_iterator begin() const {
		return get();
	}

	iterator end() {
		return begin() + size();
	}

	const_iterator end() const {
		return begin() + size();
	}

	T* release() {
		T* tmp = m_buffer;
		m_buffer = NULL;
		return tmp;
	}

	void reset(T* buffer) {
		if (m_buffer) {
			delete[] m_buffer;
		}
		m_buffer = buffer;
	}

	T& operator[](size_t pos) {
		return m_buffer[pos];
	}

	const T& operator[](size_t pos) const {
		return m_buffer[pos];
	}

	size_t size() const {
		if (m_buffer == NULL) {
			return 0;
		}
		return alloc::size(m_buffer);
	}

private:
	T* m_buffer;
};

namespace std {
	template<class T>
	void swap(ArrayPtr<T>& a1, ArrayPtr<T>& a2) {
		a1.swap(a2);
	}
}

#endif // ARRAYPTR_H
