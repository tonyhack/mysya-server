#ifndef MASYA_CLASS_UTIL_H
#define MASYA_CLASS_UTIL_H

#include <cstdarg>

#define MASYA_SAFE_BOOL_TYPE(_T)              \
private:                                      \
    typedef void (_T::*SafeBoolType)() const; \
    void SafeBoolTypeNotNull() const {}       \
public:                                       \


#define GET_VAR_PARAMS(str, str_len, pat)     \
  do {                                        \
    va_list ap;                               \
    bzero(str, str_len);                      \
    va_start(ap, pat);                        \
    vsnprintf(str, str_len - 1, pat, ap);     \
    va_end(ap);                               \
  } while (false)


#define MASYA_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName &);                    \
  void operator=(const TypeName &);

#endif  // MASYA_CLASS_UTIL_H
