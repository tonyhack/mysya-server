#ifndef MYSYA_TIMESTAMP_H
#define MYSYA_TIMESTAMP_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

namespace mysya {

class Timestamp {
 public:
  Timestamp();
  Timestamp(const Timestamp &t);
  ~Timestamp();

  void SetNow();

  time_t GetSecond() const;
  int64_t GetMillisecond() const;
  int64_t GetNanosecond() const;

  // Absolute distance.
  int64_t DistanceSecond(const Timestamp &other) const;
  int64_t DistanceMillisecond(const Timestamp &other) const;

  Timestamp &operator=(const Timestamp &other);
  bool operator<(const Timestamp &other) const;

  Timestamp &operator=(int64_t millisecond);
  Timestamp &operator+=(int64_t millisecond);
  Timestamp operator+(int64_t millisecond) const;

 private:
  time_t second_;
  int64_t nanosecond_;
};

}  // namespace mysya

#endif  // MYSYA_TIMESTAMP_H
