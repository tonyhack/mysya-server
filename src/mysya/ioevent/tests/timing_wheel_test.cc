#include <stdio.h>

#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/timing_wheel.h>

namespace mysya {
namespace ioevent {
namespace tests {

EventLoop g_event_loop;

namespace timer {

class TimerHandler {
 public:
  TimerHandler(EventLoop *event_loop, int expire_ms,
      int call_times, const std::string &debug_info)
    : event_loop_(event_loop), expire_ms_(expire_ms),
      call_times_(call_times), timer_id_(0),
      debug_info_(debug_info) {
    this->event_loop_->StartTimer(1000,
        std::bind(&TimerHandler::OnStartTimer, this, std::placeholders::_1), 1);
  }
  ~TimerHandler() {}

 private:
  bool StartTimer(int expire_ms, int call_times) {
    this->timer_id_ = this->event_loop_->StartTimer(expire_ms,
        std::bind(&TimerHandler::OnExpired, this, std::placeholders::_1),
        call_times);
    if (timer_id_ < 0) {
      return false;
    }

    // printf("Start timer[%s] expire_ms(%d)...\n",
    //     this->debug_info_.data(), expire_ms);

    this->timestamp_ = this->event_loop_->GetTimestamp();

#ifndef _MYSYA_DEBUG_
    this->event_loop_->SetTimerDebugTickCounts(0);
#endif  // _MYSYA_DEBUG_

    return true;
  }

  void OnStartTimer(int64_t timer_id) {
    this->StartTimer(this->expire_ms_, this->call_times_);
  }

  void OnExpired(int64_t timer_id) {
    ::mysya::util::Timestamp timestamp;
    timestamp.SetNow();

#ifndef _MYSYA_DEBUG_
    printf("[%s] timer_id[%ld] expired[%ld,%ld] tick_counts[%ld].\n", this->debug_info_.data(), timer_id,
        this->timestamp_.DistanceMicroSecond(this->event_loop_->GetTimestamp()),
        this->timestamp_.DistanceMillisecond(this->event_loop_->GetTimestamp()),
        this->event_loop_->GetTimerDebugTickCounts());
#else
    printf("[%s] timer_id[%ld] expired[%ld,%ld].\n", this->debug_info_.data(), timer_id,
        this->timestamp_.DistanceMicroSecond(this->event_loop_->GetTimestamp()),
        this->timestamp_.DistanceMillisecond(this->event_loop_->GetTimestamp()));
#endif  // _MYSYA_DEBUG_

    // exit(0);
  }

  EventLoop *event_loop_;
  int expire_ms_;
  int call_times_;
  int64_t timer_id_;
  ::mysya::util::Timestamp timestamp_;
  std::string debug_info_;
};

void TestFunc() {
  try {
    TimerHandler handler0(&g_event_loop, 5, 1, "5 msec timer");
    TimerHandler handler1(&g_event_loop, 30, 1, "30 msec timer");
    TimerHandler handler2(&g_event_loop, 100, 1, "100 msec timer");
    TimerHandler handler3(&g_event_loop, 255, 1, "255 msec timer");
    TimerHandler handler4(&g_event_loop, 257, 1, "257 msec timer");
    TimerHandler handler5(&g_event_loop, 500, 1, "500 msec timer");
    TimerHandler handler6(&g_event_loop, 1 * 1000, 1, "1 sec timer");
    TimerHandler handler7(&g_event_loop, 10 * 1000, 1, "10 sec timer");
    TimerHandler handler8(&g_event_loop, 20 * 1000, 1, "20 sec timer");
    TimerHandler handler9(&g_event_loop, 1 * 60 * 1000, 1, "1 min timer");
    TimerHandler handler10(&g_event_loop, 3 * 60 * 1000, 1, "3 min timer");
    TimerHandler handler11(&g_event_loop, 10 * 60 * 1000, 1, "10 min timer");
    TimerHandler handler12(&g_event_loop, 20 * 60 * 1000, 1, "20 min timer");
    TimerHandler handler13(&g_event_loop, 1 * 60 * 60 * 1000, 1, "1 hour timer");
    TimerHandler handler14(&g_event_loop, 2 * 60 * 60 * 1000, 1, "2 hour timer");
    TimerHandler handler15(&g_event_loop, 5 * 60 * 60 * 1000, 1, "5 hour timer");
    TimerHandler handler16(&g_event_loop, 10 * 60 * 60 * 1000, 1, "10 hour timer");
    TimerHandler handler17(&g_event_loop, 24 * 60 * 60 * 1000, 1, "24 hour timer");
    TimerHandler handler18(&g_event_loop, 30 * 60 * 60 * 1000, 1, "30 hour timer");
    g_event_loop.Loop();
  } catch (...) {
    printf("error.\n");
  }
}

}  // namespace timer

}  // namespace tests
}  // namespace ioevent
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::ioevent::tests::timer::TestFunc();

  return 0;
}
