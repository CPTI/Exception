#ifndef CONFIG_H
#define CONFIG_H

#if __cplusplus >= 201103L
#define USE_CXX11
#elif defined QT_CORE_LIB
#define USE_QT
#endif

#ifdef QT_CORE_LIB
#define SUPPORT_QT
#endif


#endif /* CONFIG_H */
