#ifndef MYSYA_IOEVENT_EVENT_LOOP_H
#define MYSYA_IOEVENT_EVENT_LOOP_H

#include <sys/epoll.h>

#include <functional>
#include <vector>

#include <ext/hash_set>

#include <mysya/ioevent/timing_wheel.h>
#include <mysya/util/class_util.h>
#include <mysya/util/timestamp.h>

namespace mysya {
namespace ioevent {

class EventChannel;
class TimingWheel;

class EventLoop {
 public:
  typedef std::vector<struct epoll_event> EventVector;
  typedef TimingWheel::ExpireCallback ExpireCallback;
  typedef __gnu_cxx::hash_set<uint64_t> EventChannelHashset;
  typedef std::function<void ()> WakeupCallback;
  typedef std::vector<WakeupCallback> WakeupCallbackVector;

  EventLoop();
  ~EventLoop();

  bool Looping() const;
  void Loop();

  void Quit();
  void Wakeup();

  bool AddEventChannel(EventChannel *channel);
  bool RemoveEventChannel(EventChannel *channel);
  bool UpdateEventChannel(EventChannel *channel);

  uint64_t AllocateAttachID();

  int64_t StartTimer(int expire_ms, const ExpireCallback &cb,
      int call_times = -1);
  void StopTimer(int64_t timer_id);

  void WakeupCallback(const WakeupCallback &cb);

#ifndef _MYSYA_DEBUG_
  int64_t GetTimerDebugTickCounts() const;
  void SetTimerDebugTickCounts(int64_t value) const;
#endif  // _MYSYA_DEBUG_

  const ::mysya::util::Timestamp &GetTimestamp() const { return this->timestamp_; }

 private:
  bool CheckEventChannelRemoved(EventChannel *channel) const;

  void OnWakeupRead(EventChannel *event_channel);
  void DoWakeupCallback();

  static const int kReadEventMask;
  static const int kWriteEventMask;
  static const int kErrorEventMask;

  bool quit_;
  int epoll_fd_;
  EventVector active_events_;
  EventChannelHashset removed_event_channels_;

  pid_t thread_id_;

  TimingWheel *timing_wheel_;
  ::mysya::util::Timestamp timestamp_;

  int wakeup_fd_;
  Mutex wakeup_mutex_;
  EventChannel *wakeup_event_channel_;
  WakeupCallbackVector wakeup_cbs_;

  class AttachIdAllocator;
  AttachIdAllocator *attach_ids_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(EventLoop);
};

}  // namespace ioevent
}  // namespace mysya

#endif  // MYSYA_EVENT_LOOP_H
