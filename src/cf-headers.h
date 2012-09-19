#ifndef _SRC_CF_HEADERS_H_
#define _SRC_CF_HEADERS_H_

#include <CoreFoundation/CoreFoundation.h>

#ifdef MAC_OS_X_VERSION_10_7
# include "headers/cf-10-7.h"
#elif MAC_OS_X_VERSION_10_6
# include "headers/cf-10-6.h"
#else
# error "Unknown mac os version"
#endif

#endif // _SRC_CF_HEADERS_H_
