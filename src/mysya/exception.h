#ifndef MYSYA_EXCEPTION_H
#define MYSYA_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace mysya {

class SystemErrorException : public std::runtime_error {
 public:
  SystemErrorException(const std::string &arg)
    : std::runtime_error(arg) {}
};

}  // namespace mysya

#endif  // MYSYA_EXCEPTION_H
