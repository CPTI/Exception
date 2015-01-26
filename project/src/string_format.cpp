#include "string_format.h"


#ifdef USE_CXX11
#elif defined USE_QT
namespace string_format {
    const QString adapt(const std::string& t) { return QString::fromStdString(t); }
}
#endif
