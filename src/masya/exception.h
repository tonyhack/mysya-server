#ifndef MASYA_EXCEPTION_H
#define MASYA_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace masya {

class SystemErrorException : public std::runtime_error {
 public:
  SystemErrorException(const std::string &arg)
    : std::runtime_error(arg) {}
};

}  // namespace masya

#endif  // MASYA_EXCEPTION_H
