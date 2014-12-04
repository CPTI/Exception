#include "VectorIO.h"

#ifdef LINUX
#include <sys/uio.h>
#include <unistd.h>
#endif

#include <vector>
#include <stdio.h>
#include <string.h>

namespace {
    int fallback_write(FILE* dev, VectorIO::out_elem* data, size_t n) {

        size_t total = 0;
        for (size_t i = 0; i < n; ++i) {
            total += data[i].size;
        }
        std::vector<char> single_buffer(total);

        size_t pos = 0;
        for (size_t i = 0; i < n; ++i) {
            memcpy(&single_buffer[pos], reinterpret_cast<const char*>(data[i].begin), data[i].size);
            pos += data[i].size;
        }
        return fwrite(&single_buffer[0], 1, single_buffer.size(), dev);
	}

#ifdef LINUX
	int filedes_write(int fd, VectorIO::out_elem* data, size_t n) {

		// Aparentemente quando o fd é um terminal a saida pode ficar misturada no linux
		// http://stackoverflow.com/questions/9336572/how-does-list-i-o-writev-internally-work

		if (isatty(fd)) {
            size_t total = 0;
            for (size_t i = 0; i < n; ++i) {
                total += data[i].size;
            }
            std::vector<char> single_buffer(total);

            size_t pos = 0;
			for (size_t i = 0; i < n; ++i) {
                memcpy(&single_buffer[pos], reinterpret_cast<const char*>(data[i].begin), data[i].size);
                pos += data[i].size;
			}
            return write(fd, &single_buffer[0], single_buffer.size());
		} else {
			// meu out_elem é binariamente igual ao iovec;
			return writev(fd, reinterpret_cast<iovec*>(data), n);
		}
	}
#endif

}

namespace VectorIO
{
	int write_vec(FILE* dev, out_elem* data, size_t n) {
#ifdef LINUX
        return filedes_write(fileno(dev), data, n);
#endif
        return fallback_write(dev, data, n);
	}
}
