#ifndef MYSYA_CLASS_UTIL_H
#define MYSYA_CLASS_UTIL_H

#include <cstdarg>

#define MYSYA_SAFE_BOOL_TYPE(_T)              \
 private:                                     \
    typedef void (_T::*SafeBoolType)() const; \
    void SafeBoolTypeNotNull() const {}       \
 public:                                      \


#define MYSYA_GET_VAR_PARAMS(str, str_len, pat) \
  do {                                          \
    va_list ap;                                 \
    bzero(str, str_len);                        \
    va_start(ap, pat);                          \
    vsnprintf(str, str_len - 1, pat, ap);       \
    va_end(ap);                                 \
  } while (false);                              \


#define MYSYA_DISALLOW_COPY_AND_ASSIGN(_T)    \
  _T(const _T &);                             \
  void operator=(const _T &);                 \


#define MYSYA_SINGLETON(_T)                   \
 public:                                      \
  static _T *GetInstance() {                  \
    if (g_instance_ == NULL) {                \
      g_instance_ = new _T;                   \
    }                                         \
    return g_instance_;                       \
  }                                           \
                                              \
  static void RelaseInstance() {              \
    delete g_instance_;                       \
    g_instance_ = NULL;                       \
  }                                           \
                                              \
 private:                                     \
  static _T *g_instance_;                     \
  _T();                                       \
  ~_T();                                      \
  _T(const _T &);                             \
  void operator=(const _T &);                 \


#define MYSYA_SINGLETON2(_T)                  \
 public:                                      \
  static _T *GetInstance() {                  \
    static _T t;                              \
    return &t;                                \
  }                                           \
                                              \
  static void RelaseInstance() {              \
  }                                           \
                                              \
 private:                                     \
  _T();                                       \
  ~_T();                                      \
  _T(const _T &);                             \
  void operator=(const _T &);                 \


#endif  // MYSYA_CLASS_UTIL_H
