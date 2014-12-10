#include "string_format.h"


#ifdef QT_CORE_LIB
namespace string_format {
    const QString adapt(const std::string& t) { return QString::fromStdString(t); }
}
#endif
