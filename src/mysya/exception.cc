#include <mysya/exception.h>

#include <stdarg.h>

namespace mysya {

void ThrowSystemErrorException(const char *format, ...) {
  char buffer[4096];

  va_list args;
  ::va_start(args, format);
  ::vsprintf(buffer, format, args);
  ::va_end(args);

  throw SystemErrorException(buffer);
}

}  // namespace mysya
