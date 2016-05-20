#ifndef MYSYA_UTIL_HEX_DUMP_H
#define MYSYA_UTIL_HEX_DUMP_H

#include <stddef.h>

#include <algorithm>

namespace mysya {
namespace util {

void Hexdump(const char *buffer, size_t size, const char *desc = "");

}  // namespace util
}  // namespace mysya

#endif  // MYSYA_UTIL_HEX_DUMP_H
