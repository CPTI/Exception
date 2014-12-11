#include "string_format.h"


#if __cplusplus >= 201103L
#elif defined QT_CORE_LIB
namespace string_format {
    const QString adapt(const std::string& t) { return QString::fromStdString(t); }
}
#endif
