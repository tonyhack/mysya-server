#include <mysya/util/exception.h>

#include <stdarg.h>

namespace mysya {
namespace util {

void ThrowSystemErrorException(const char *format, ...) {
  char buffer[4096];

  va_list args;
  ::va_start(args, format);
  ::vsprintf(buffer, format, args);
  ::va_end(args);

  throw SystemErrorException(buffer);
}

}  // namespace util
}  // namespace mysya
