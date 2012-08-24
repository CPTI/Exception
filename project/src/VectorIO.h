#ifndef VECTORIO_H
#define VECTORIO_H

#include <stddef.h>
#include <stdio.h>
#include <QIODevice>

namespace VectorIO
{
	struct out_elem {
		const void* begin;
		size_t size;
	};

	int write_vec(QIODevice* dev, out_elem* data, size_t n);
	int write_vec(FILE* dev, out_elem* data, size_t n);
}

#endif // VECTORIO_H
