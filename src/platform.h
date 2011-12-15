#ifndef PLATFORM_H
#define  PLATFORM_H

#if defined(_WIN32)
#define __WINDOWS__ 1
#elif defined(_WIN64)
#define __WINDOWS__ 1
#endif

#if defined(linux)
#define __UNIX__
#endif

#endif /* PLATFORM_H */