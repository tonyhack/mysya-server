#include <mysya/ioevent/timing_wheel.h>

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/timerfd.h>

#include <list>
#include <memory>

#include <mysya/ioevent/event_channel.h>
#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/logger.h>
#include <mysya/util/exception.h>
#include <mysya/util/timestamp.h>

namespace mysya {
namespace ioevent {

const int TimingWheel::kTimingWheelBucketNum[TimingWheel::kTimingWheelNum] = {
  256, 64, 64, 64, 64,
};

class TimingWheel::Timer {
 public:
  typedef TimingWheel::ExpireCallback ExpireCallback;

  Timer(int64_t id, const ::mysya::util::Timestamp &expire_timestamp,
      int expire_tick_counts, int call_times, const ExpireCallback &cb)
    : id_(id), expire_timestamp_(expire_timestamp),
      expire_tick_counts_(expire_tick_counts), call_times_(call_times),
      undo_tick_counts_(0), wheel_(0), bucket_(0), expire_cb_(cb) {}
  ~Timer() {}

  int64_t GetID() const { return this->id_; }
  const ::mysya::util::Timestamp &GetExpireTimestamp() const { return this->expire_timestamp_; }
  int GetExpireTickCounts() const { return this->expire_tick_counts_; }
  int GetCallTimes() const { return this->call_times_; }
  int DecCallTimes() { return --this->call_times_; }
  int GetUndoTickCounts() const { return this->undo_tick_counts_; }
  void SetUndoTickCounts(int value) { this->undo_tick_counts_ = value; }
  int GetWheel() const { return this->wheel_; }
  void SetWheel(int value) { this->wheel_ = value; }
  size_t GetBucket() const { return this->bucket_; }
  void SetBucket(size_t value) { this->bucket_ = value; }
  ExpireCallback GetExpireCallback() const { return this->expire_cb_; }

 private:
  int64_t id_;
  ::mysya::util::Timestamp expire_timestamp_;
  int expire_tick_counts_;
  int call_times_;
  int undo_tick_counts_;

  int wheel_;
  size_t bucket_;

  ExpireCallback expire_cb_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Timer);
};

class TimingWheel::TimerIdAllocator {
 public:
  TimerIdAllocator() : id_(0) {}
  ~TimerIdAllocator() {}

  int64_t Allocate() {
    if (++this->id_ <= 0) {
      return -1;
    }

    return this->id_;
  }

 private:
  int64_t id_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(TimerIdAllocator);
};

class TimingWheel::Wheel {
 public:
  typedef std::list<Timer *> TimerList;

  Wheel(TimingWheel *timing_wheel, int index, size_t bucket_num);
  ~Wheel();

  void AddTimer(Timer *timer, size_t tick_counts);
  void RemoveTimer(size_t bucket, int64_t timer_id);

  int OnExpired(const ::mysya::util::Timestamp &now_timestamp);

  size_t GetCurrentBucket() const { return this->current_bucket_; }
  void SetCurrentBucket(size_t value) { this->current_bucket_ = value; }
  size_t GetBucketNum() const { return this->bucket_num_; }

 private:
  TimingWheel *host_;
  int index_;

  size_t current_bucket_;
  size_t bucket_num_;
  TimerList *buckets_;
};

TimingWheel::Wheel::Wheel(TimingWheel *timing_wheel,
    int index, size_t bucket_num)
  : host_(timing_wheel), index_(index),
    current_bucket_(0), bucket_num_(bucket_num),
    buckets_(NULL) {
  this->buckets_ = new (std::nothrow) TimerList[this->GetBucketNum()];
  if (this->buckets_ == NULL) {
    ::mysya::util::ThrowSystemErrorException(
        "Wheel::Wheel(): allocate TimerList failed.");
  }
}

TimingWheel::Wheel::~Wheel() {
  delete [] this->buckets_;
}

void TimingWheel::Wheel::AddTimer(Timer *timer, size_t tick_counts) {
  size_t wheel_step_tick_counts = 1;

  // Total tick counts of this wheel step.
  for (int i = 0; i < this->index_; ++i) {
    Wheel *wheel = this->host_->GetWheel(i);
    wheel_step_tick_counts *= wheel->GetBucketNum();
  }

  assert(tick_counts < wheel_step_tick_counts * this->GetBucketNum());

  // Position to put in.
  size_t bucket =
    (tick_counts / wheel_step_tick_counts + this->GetCurrentBucket()) %
    this->GetBucketNum();
  bucket = bucket > 0 ? bucket - 1 : this->GetBucketNum() - 1;

  assert(bucket < this->GetBucketNum());

  timer->SetBucket(bucket);
  this->buckets_[bucket].push_back(timer);
}

void TimingWheel::Wheel::RemoveTimer(size_t bucket, int64_t timer_id) {
  assert(bucket < this->GetBucketNum());

  for (TimerList::iterator iter = this->buckets_[bucket].begin();
      iter != this->buckets_[bucket].end(); ++iter) {
    Timer *timer = *iter;

    if (timer->GetID() == timer_id) {
      this->buckets_[bucket].erase(iter);
      break;
    }
  }
}

int TimingWheel::Wheel::OnExpired(const ::mysya::util::Timestamp &now_timestamp) {
  size_t bucket = this->GetCurrentBucket();
  this->SetCurrentBucket((bucket + 1) % this->GetBucketNum());

  for (TimerList::iterator iter = this->buckets_[bucket].begin();
      iter != this->buckets_[bucket].end();) {
    Timer *timer = *iter;
    iter = this->buckets_[bucket].erase(iter);

    // Undo tick count.
    if (timer->GetUndoTickCounts() > 0) {
      // undo.
      this->host_->AddWheel(timer, timer->GetUndoTickCounts());
      continue;
    } else if (now_timestamp < timer->GetExpireTimestamp()) {
      // not expired.
      timer->SetUndoTickCounts(1);
      this->host_->AddWheel(timer, timer->GetUndoTickCounts());
      continue;
    } else {
      // expired and callback.
      int64_t timer_id = timer->GetID();
      ExpireCallback callback = timer->GetExpireCallback();

      if (timer->GetCallTimes() < 0 || timer->DecCallTimes() > 0) {
        // Add timer again.
        this->host_->AddWheel(timer, timer->GetExpireTickCounts());
      } else {
        this->host_->RemoveTimer(timer_id);
      }

      callback(timer_id);
    }
  }

  return this->GetCurrentBucket();
}


TimingWheel::TimingWheel(int tick_ms, EventLoop *event_loop)
  : tick_ms_(tick_ms), undo_nsec_(0), event_loop_(event_loop),
    event_channel_(NULL), timer_ids_(NULL) {
#ifndef _MYSYA_DEBUG_
  this->debug_tick_counts_ = 0;
#endif  // _MYSYA_DEBUG_

  // TODO: unique_ptr<> ?
  this->event_channel_ = new (std::nothrow) EventChannel();
  if (this->event_channel_ == NULL) {
    ::mysya::util::ThrowSystemErrorException(
        "TimingWheel::TimingWheel(): allocate EventChannel failed.");
  }

  // TODO: unique_ptr<> ?
  this->timer_ids_ = new (std::nothrow) TimerIdAllocator();
  if (this->timer_ids_ == NULL) {
    ::mysya::util::ThrowSystemErrorException(
        "TimingWheel::TimingWheel(): allocate TimerIdAllocator failed.");
  }

  // Allocate wheels.
  for (int i = 0; i < 5; ++i) {
    // TODO: unique_ptr<> ?
    Wheel *wheel =
      new (std::nothrow) Wheel(this, i, kTimingWheelBucketNum[i]);
    if (wheel == NULL) {
      ::mysya::util::ThrowSystemErrorException(
          "TimingWheel::TimingWheel(): allocate Wheel failed.");
    }

    this->wheels_.push_back(wheel);
  }

  int timer_fd = ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  if (timer_fd == -1) {
    MYSYA_ERROR("timerfd_create failed, errno=%d.", errno);
    ::mysya::util::ThrowSystemErrorException(
        "TimingWheel::TimingWheel(): timerfd_create failed, strerror(%s).",
        ::strerror(errno));
  }

  this->event_channel_->SetFileDescriptor(timer_fd);
  this->event_channel_->SetReadCallback(
      std::bind(&TimingWheel::OnRead, this, std::placeholders::_1));

  if (this->event_channel_->AttachEventLoop(this->event_loop_) == false) {
    ::mysya::util::ThrowSystemErrorException(
          "TimingWheel::TimingWheel(): EventChannel::AttachEventLoop() failed.");
  }

  struct itimerspec new_value;
  bzero(&new_value, sizeof(new_value));
  new_value.it_value.tv_sec = 1;
  new_value.it_interval.tv_sec = this->tick_ms_ / 1000;
  new_value.it_interval.tv_nsec = (this->tick_ms_ % 1000) * 1000000;

  if (::timerfd_settime(this->event_channel_->GetFileDescriptor(), 0, &new_value, NULL) != 0) {
    MYSYA_ERROR("timerfd_settime failed, errno=%d.", errno);
    ::mysya::util::ThrowSystemErrorException(
          "TimingWheel::TimingWheel(): timerfd_settime failed, strerror(%s).",
          ::strerror(errno));
  }
}

TimingWheel::~TimingWheel() {
  int fd = this->event_channel_->GetFileDescriptor();
  if (fd != -1) {
    this->event_channel_->DetachEventLoop();
    ::close(fd);
  }

  delete this->timer_ids_;
  delete this->event_channel_;
}

int64_t TimingWheel::AddTimer(const ::mysya::util::Timestamp &now, int expire_ms,
    const ExpireCallback &cb, int call_times) {
  int expire_tick_counts = expire_ms / this->tick_ms_;
  if (expire_ms % this->tick_ms_ > 0) {
    expire_tick_counts += 1;
  }

  int64_t timer_id = this->timer_ids_->Allocate();
  if (timer_id <= 0) {
    MYSYA_ERROR("TimerIdAllocator::Allocate() failed.");
    return -1;
  }

  if (this->timers_.find(timer_id) != this->timers_.end()) {
    MYSYA_ERROR("Timer id(%d) duplicated.", timer_id);
    return -1;
  }

  std::unique_ptr<Timer> timer(
    new (std::nothrow) Timer(timer_id, now + expire_ms,
        expire_tick_counts, call_times, cb));
  if (timer.get() == NULL) {
    MYSYA_ERROR("Allocate Timer failed.");
    return -1;
  }

  this->AddWheel(timer.get(), expire_tick_counts);
  this->timers_.insert(std::make_pair(timer_id, timer.get()));

  timer.release();

  return timer_id;
}

void TimingWheel::RemoveTimer(int64_t timer_id) {
  TimerHashmap::iterator iter = this->timers_.find(timer_id);
  if (iter == this->timers_.end()) {
    return;
  }

  Timer *timer = iter->second;

  // Remove timer from wheel.
  this->RemoveWheel(timer);

  this->timers_.erase(iter);
  delete timer;
}

TimingWheel::Wheel *TimingWheel::GetWheel(int index) const {
  if (index < 0 || index >= (int)this->wheels_.size()) {
    return NULL;
  }

  return this->wheels_[index];
}

void TimingWheel::AddWheel(Timer *timer, int expire_tick_counts) {
  int64_t undo_tick_counts = 0;
  int64_t wheel_max_tick_counts = 1;
  int64_t wheel_step_tick_counts = 1;

  for (int i = 0; i < kTimingWheelNum; ++i) {
    Wheel *wheel = this->GetWheel(i);

    wheel_max_tick_counts *= kTimingWheelBucketNum[i];
    if (expire_tick_counts < wheel_max_tick_counts) {
      wheel->AddTimer(timer, expire_tick_counts);
      timer->SetWheel(i);
      break;
    }

    undo_tick_counts += wheel->GetCurrentBucket() * wheel_step_tick_counts;

    // The next wheel step tick counts.
    wheel_step_tick_counts = wheel_max_tick_counts;
  }

  undo_tick_counts += expire_tick_counts % wheel_step_tick_counts;

  timer->SetUndoTickCounts(undo_tick_counts);
}

void TimingWheel::RemoveWheel(Timer *timer) {
  Wheel *wheel = this->GetWheel(timer->GetWheel());
  if (wheel == NULL) {
    MYSYA_ERROR("TimingWheel::GetWheel(%d) failed.", timer->GetWheel());
    return;
  }

  wheel->RemoveTimer(timer->GetBucket(), timer->GetID());
}

void TimingWheel::OnRead(EventChannel *event_channel) {
  uint64_t read_value = 0;
  ::read(this->event_channel_->GetFileDescriptor(),
      &read_value, sizeof(read_value));

  EventLoop *event_loop = event_channel->GetEventLoop();
  this->OnExpired(event_loop->GetTimestamp());
}

void TimingWheel::OnExpired(const ::mysya::util::Timestamp &now_timestamp) {
  int undo_tick_counts = 0;
  int tick_ns = this->tick_ms_ * 1000000;

  int64_t distance_ns = now_timestamp.DistanceNanoSecond(this->timestamp_);

  this->undo_nsec_ += distance_ns;
  undo_tick_counts = this->undo_nsec_ / tick_ns;
  this->undo_nsec_ %= tick_ns;

  while (--undo_tick_counts >= 0) {
#ifndef _MYSYA_DEBUG_
    ++this->debug_tick_counts_;
#endif  // _MYSYA_DEBUG_

    for (int i = 0; i < kTimingWheelNum; ++i) {
      if (this->GetWheel(i)->OnExpired(now_timestamp) > 0) {
        break;
      }
    }
  }

  this->timestamp_ = now_timestamp;
}

}  // namespace ioevent
}  // namespace mysya
