
#include <vector>

#ifndef SVECTOR_H
#define SVECTOR_H
#include "string_format.h"


#if defined(DEBUG) && !defined(__4WIN__)

#include "Error.h"

template<typename T>
class svector : public std::vector<T>
{
public:
	explicit svector() : std::vector<T>() {}

	explicit svector(size_t __n, const T &__value = T())
	:
	std::vector<T>(__n, __value) {}

    T &operator[](size_t n) {
		ERR_ASSERT_MESSAGE(Error::mainSoftware(),
				n < std::vector<T>::size(),
                string_format::format("operator [] | n = %1 | size = %2", n, std::vector<T>::size()));
		return static_cast<std::vector<T> &>(*this)[n];
	}
	const T &operator[](size_t n) const {
		ERR_ASSERT_MESSAGE(Error::mainSoftware(),
				n < std::vector<T>::size(),
                string_format::format("operator [] | n = %1 | size = %2", n, std::vector<T>::size()));
		return static_cast<const std::vector<T> &>(*this)[n];
	}
};


template <>
class svector<bool> : public std::vector<int>
{
public:
	explicit svector()
	:
	std::vector<int>() {}

	explicit svector(size_t __n, const bool &__value = bool())
	:
	std::vector<int>(__n, (int)__value) {}

	int& operator[](size_t n) {
		ERR_ASSERT_MESSAGE(Error::mainSoftware(), n < std::vector<int>::size(), "operator [] " );
		return static_cast<std::vector<int> &>(*this)[n];
	}
	const int& operator[](size_t n) const {
		ERR_ASSERT_MESSAGE(Error::mainSoftware(), n < std::vector<int>::size(), "operator [] " );
		return static_cast<const std::vector<int> &>(*this)[n];
	}
};
#else // DEBUG
	#define svector std::vector
#endif// DEBUG

#endif // SVECTOR_H

