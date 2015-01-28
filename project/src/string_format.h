#ifndef STRING_FORMAT_H
#define STRING_FORMAT_H

#include "config.h"
#ifdef USE_CXX11
#include "str_conversion2.h"
#elif defined USE_QT
#include <QString>
#endif

namespace string_format {


#ifdef USE_CXX11

template <class... T>
std::string format(const char* fmt, const T&... t) {
    return std::move(strconv2::fmt_str(fmt, t...));
}

#elif defined USE_QT

template<class T>
const T& adapt(const T& t) { return t; }

const QString adapt(const std::string& t);

inline std::string format(const char* fmt) {
    return std::string(fmt);
}

template <class T1>
std::string format(const char* fmt, const T1& t1) {
    return QString(fmt).arg(adapt(t1)).toStdString();
}

template <class T1, class T2>
std::string format(const char* fmt, const T1& t1, const T2& t2) {
    return QString(fmt).arg(adapt(t1)).arg(adapt(t2)).toStdString();
}

template <class T1, class T2, class T3>
std::string format(const char* fmt, const T1& t1, const T2& t2, const T3& t3) {
    return QString(fmt).arg(adapt(t1)).arg(adapt(t2)).arg(adapt(t3)).toStdString();
}

template <class T1, class T2, class T3, class T4>
std::string format(const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4) {
    return QString(fmt).arg(adapt(t1)).arg(adapt(t2)).arg(adapt(t3)).arg(adapt(t4)).toStdString();
}

template <class T1, class T2, class T3, class T4, class T5>
std::string format(const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5) {
    return QString(fmt).arg(adapt(t1)).arg(adapt(t2)).arg(adapt(t3)).arg(adapt(t4)).arg(adapt(t5)).toStdString();
}

template <class T1, class T2, class T3, class T4, class T5, class T6>
std::string format(const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6) {
    return QString(fmt).arg(adapt(t1)).arg(adapt(t2)).arg(adapt(t3)).arg(adapt(t4)).arg(adapt(t5)).arg(adapt(t6)).toStdString();
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
std::string format(const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7) {
    return QString(fmt).arg(adapt(t1)).arg(adapt(t2)).arg(adapt(t3)).arg(adapt(t4)).arg(adapt(t5)).arg(adapt(t6)).arg(adapt(t7)).toStdString();
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
std::string format(const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8) {
    return QString(fmt).arg(adapt(t1)).arg(adapt(t2)).arg(adapt(t3)).arg(adapt(t4)).arg(adapt(t5)).arg(adapt(t6)).arg(adapt(t7)).arg(adapt(t8)).toStdString();
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
std::string format(const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9) {
    return QString(fmt).arg(adapt(t1)).arg(adapt(t2)).arg(adapt(t3)).arg(adapt(t4)).arg(adapt(t5)).arg(adapt(t6)).arg(adapt(t7)).arg(adapt(t8)).arg(adapt(t9)).toStdString();
}

template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
std::string format(const char* fmt, const T1& t1, const T2& t2, const T3& t3, const T4& t4, const T5& t5, const T6& t6, const T7& t7, const T8& t8, const T9& t9, const T10& t10) {
    return QString(fmt).arg(adapt(t1)).arg(adapt(t2)).arg(adapt(t3)).arg(adapt(t4)).arg(adapt(t5)).arg(adapt(t6)).arg(adapt(t7)).arg(adapt(t8)).arg(adapt(t9)).arg(adapt(t10))
            .toStdString();
}

#endif


}

#endif // STRING_FORMAT_H
