#ifndef MYSYA_UTIL_EXCEPTION_H
#define MYSYA_UTIL_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace mysya {
namespace util {

class SystemErrorException : public std::runtime_error {
 public:
  SystemErrorException(const std::string &arg)
    : std::runtime_error(arg) {}
};

void ThrowSystemErrorException(const char *format, ...);

}  // namespace util
}  // namespace mysya

#endif  // MYSYA_UTIL_EXCEPTION_H
