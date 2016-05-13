#ifndef TUTORIAL_ORCAS_COMBAT_BASE64_H
#define TUTORIAL_ORCAS_COMBAT_BASE64_H

#include <cstddef>
#include <string>

namespace tutorial {
namespace orcas {
namespace combat {

int base64_encode(const char *in, size_t in_size,
                  char *out, size_t out_size);
std::string base64_encode(const std::string &str);
std::string base64_encode(const char *buffer, size_t size);

int base64_decode(const char *in, size_t in_size,
                  char *out, size_t out_size);
std::string base64_decode(const std::string &str);
std::string base64_decode(const char *buffer, size_t size);

}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_BASE64_H
