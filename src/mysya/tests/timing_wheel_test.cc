#include <stdio.h>

#include <mysya/event_loop.h>
#include <mysya/timing_wheel.h>

namespace mysya {
namespace test {

EventLoop g_event_loop;

namespace timer {

class TimerHandler {
 public:
  TimerHandler(EventLoop *event_loop, const std::string &debug_info)
    : event_loop_(event_loop), debug_info_(debug_info) {}
  ~TimerHandler() {}

  bool StartTimer(int expire_ms, int call_times) {
    this->timer_id_ = this->event_loop_->StartTimer(expire_ms,
        std::bind(&TimerHandler::OnExpired, this, std::placeholders::_1),
        call_times);
    if (timer_id_ < 0) {
      return false;
    }

    this->timestamp_ = this->event_loop_->GetTimestamp();

    return true;
  }

  void OnExpired(int64_t timer_id) {
    printf("[%s] timer_id[%ld] expired[%ld].\n", this->debug_info_.data(), timer_id,
        this->timestamp_.DistanceMillisecond(this->event_loop_->GetTimestamp()));
  }

 private:
  EventLoop *event_loop_;
  Timestamp timestamp_;
  int64_t timer_id_;
  std::string debug_info_;
};

void TestFunc() {
  TimerHandler handler1(&g_event_loop, "10sec timer");
  if (handler1.StartTimer(10 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  TimerHandler handler2(&g_event_loop, "20sec timer");
  if (handler2.StartTimer(20 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  TimerHandler handler3(&g_event_loop, "1min timer");
  if (handler3.StartTimer(1 * 60 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  TimerHandler handler4(&g_event_loop, "20min timer");
  if (handler4.StartTimer(20 * 60 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  TimerHandler handler5(&g_event_loop, "50min timer");
  if (handler5.StartTimer(50 * 60 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  TimerHandler handler6(&g_event_loop, "1hour timer");
  if (handler6.StartTimer(1 * 60 * 60 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  TimerHandler handler7(&g_event_loop, "2hour timer");
  if (handler7.StartTimer(2 * 60 * 60 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  TimerHandler handler8(&g_event_loop, "3hour timer");
  if (handler8.StartTimer(3 * 60 * 60 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  TimerHandler handler9(&g_event_loop, "4hour timer");
  if (handler9.StartTimer(4 * 60 * 60 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  TimerHandler handler10(&g_event_loop, "5hour timer");
  if (handler10.StartTimer(5 * 60 * 60 * 1000, 1) == false) {
    printf("TimerHandler::StartTimer() failed.\n");
    return;
  }

  try {
    g_event_loop.Loop();
  } catch (...) {
    printf("error.\n");
  }
}

}  // namespace timer

}  // namespace test
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::test::timer::TestFunc();

  return 0;
}
