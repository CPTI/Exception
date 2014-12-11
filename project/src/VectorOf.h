#ifndef VECTOROF_H
#define VECTOROF_H

#include "stdlib.h"

/*!
 * Este template funciona como um contrato, ele obriga que a classe derivada
 * possua uma interface mínima de vetor.
 */
template<class Element> class VectorOf
{
public:
	/*! Default constructor. */
	VectorOf() {}

	/*! Copy constructor. */
	VectorOf(const VectorOf &) {}

	virtual ~VectorOf() {}
	virtual Element &operator[](size_t pos) = 0;
	virtual const Element operator[](size_t pos) const = 0;
	virtual size_t size() const = 0;
	virtual void resize(size_t size, Element ini = Element()) = 0;
};

#endif // VECTOROF_H
