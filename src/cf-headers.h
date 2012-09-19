#ifndef _SRC_CF_HEADERS_H_
#define _SRC_CF_HEADERS_H_

namespace CF {

#include <CoreFoundation/CoreFoundation.h>

#ifdef MAC_OS_X_VERSION_10_6
#include "headers/cf-10-6.h"
#else
#error "Unknown mac os version"
#endif

} // namespace CF

#endif // _SRC_CF_HEADERS_H_
