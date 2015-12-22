#include <mysya/timestamp.h>

#include <sys/times.h>

namespace mysya {

Timestamp::Timestamp()
  : second_(0), nanosecond_(0) {}

Timestamp::Timestamp(const Timestamp &t) {
  this->second_ = t.second_;
  this->nanosecond_ = t.nanosecond_;
}

Timestamp::~Timestamp() {}

void Timestamp::SetNow() {
  struct timespec ts;
  ::clock_gettime(CLOCK_REALTIME, &ts);

  this->second_ = ts.tv_sec;
  this->nanosecond_ = ts.tv_nsec;
}

time_t Timestamp::GetSecond() const {
  return this->second_;
}

int64_t Timestamp::GetMillisecond() const {
  return this->nanosecond_ / 1000000;
}

int64_t Timestamp::GetNanosecond() const {
  return this->nanosecond_;
}

// Absolute distance.
int64_t Timestamp::DistanceSecond(const Timestamp &other) const {
  if (this->second_ > other.second_) {
    return this->second_ - other.second_;
  } else {
    return other.second_ - this->second_;
  }
}

int64_t Timestamp::DistanceMillisecond(const Timestamp &other) const {
  const Timestamp *bigger = NULL;
  const Timestamp *smaller = NULL;

  if (*this < other) {
    bigger = &other;
    smaller = this;
  } else {
    bigger = this;
    smaller = &other;
  }

  return (bigger->second_ - smaller->second_) * 1000 +
    (bigger->nanosecond_ - smaller->nanosecond_) / 1000000;
}

int64_t Timestamp::DistanceMicroSecond(const Timestamp &other) const {
  const Timestamp *bigger = NULL;
  const Timestamp *smaller = NULL;

  if (*this < other) {
    bigger = &other;
    smaller = this;
  } else {
    bigger = this;
    smaller = &other;
  }

  return (bigger->second_ - smaller->second_) * 1000000 +
    (bigger->nanosecond_ - smaller->nanosecond_) / 1000;
}

int64_t Timestamp::DistanceNanoSecond(const Timestamp &other) const {
  const Timestamp *bigger = NULL;
  const Timestamp *smaller = NULL;

  if (*this < other) {
    bigger = &other;
    smaller = this;
  } else {
    bigger = this;
    smaller = &other;
  }

  return (bigger->second_ - smaller->second_) * 1000000000 +
    (bigger->nanosecond_ - smaller->nanosecond_);
}

Timestamp &Timestamp::operator=(const Timestamp &other) {
  this->second_ = other.second_;
  this->nanosecond_ = other.nanosecond_;

  return *this;
}

bool Timestamp::operator<(const Timestamp &other) const {
  if (this->second_ != other.second_) {
    return this->second_ < other.second_;
  } else {
    return this->nanosecond_ < other.nanosecond_;
  }
}

Timestamp &Timestamp::operator=(int64_t millisecond) {
  this->second_ = millisecond / 1000;
  this->nanosecond_ = millisecond % 1000 * 1000000;

  return *this;
}

Timestamp &Timestamp::operator+=(int64_t millisecond) {
  this->second_ += millisecond / 1000;
  this->nanosecond_ += millisecond % 1000 * 1000000;

  if (this->nanosecond_ > 1000000000) {
    this->second_ += 1;
    this->nanosecond_ -= 1000000000;
  }

  return *this;
}

Timestamp Timestamp::operator+(int64_t millisecond) const {
  Timestamp timestamp(*this);
  timestamp += millisecond;

  return timestamp;
}

}  // namespace mysya
