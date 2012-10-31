#include "VectorIO.h"

#include <QByteArray>
#ifdef QT_NETWORK_LIB
#include <QAbstractSocket>
#endif
#include <QFile>
#include <QIODevice>

#ifdef LINUX
#include <sys/uio.h>
#endif


namespace {
	int fallback_write(QIODevice* dev, VectorIO::out_elem* data, size_t n) {

		QByteArray bytes;
		for (size_t i = 0; i < n; ++i) {
			bytes.append(reinterpret_cast<const char*>(data[i].begin), data[i].size);
		}
		return dev->write(bytes);
	}

#ifdef LINUX
	int filedes_write(int fd, VectorIO::out_elem* data, size_t n) {

		// meu out_elem é binariamente igual ao iovec;
		return writev(fd, reinterpret_cast<iovec*>(data), n);
	}
#endif

}

namespace VectorIO
{

	int write_vec(QIODevice* dev, out_elem* data, size_t n)
	{
#ifdef LINUX
		QFile* file = qobject_cast<QFile*>(dev);
		if (file) {
			return filedes_write(file->handle(), data, n);
		}
#ifdef QT_NETWORK_LIB
		QAbstractSocket* socket = qobject_cast<QAbstractSocket*>(dev);
		if (socket) {
			return filedes_write(socket->socketDescriptor(), data, n);
		}
#endif
#endif
		return fallback_write(dev, data, n);
	}

	int write_vec(FILE* dev, out_elem* data, size_t n) {
		QFile out;
		out.open(dev, QIODevice::WriteOnly);
		return write_vec(&out, data, n);
	}

}
