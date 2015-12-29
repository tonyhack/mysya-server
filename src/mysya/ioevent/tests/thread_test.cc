#include <unistd.h>

#include <vector>

#include <mysya/ioevent/thread.h>

namespace mysya {
namespace ioevent {
namespace test {

void ThreadFunc() {
}

void SimpleTestFunc() {
  ::mysya::ioevent::Thread t1;
  ::mysya::ioevent::Thread t2;

  t1.Start(std::bind(ThreadFunc));
  t2.Start(std::bind(ThreadFunc));

  if (t1.Joinable() == true) {
    t1.Join();
  }
  if (t2.Joinable() == true) {
    t2.Join();
  }
}



class JoinThread {
 public:
  JoinThread() : quit_(false) {}
  ~JoinThread() {}

  void Start(bool join = true) {
    this->thread_.Start(std::bind(&JoinThread::Loop, this), join);
  }

  void Quit() {
    this->quit_ = true;
  }

  void Loop() {
    while (this->quit_ == false) {
      usleep(10000);
    }
  }

  ::mysya::ioevent::Thread *GetThread() {
    return &this->thread_;
  }

 private:
  bool quit_;
  ::mysya::ioevent::Thread thread_;
};

class JoinTest {
 public:
  JoinTest() : join_count_(0), detach_count_(0) {}
  ~JoinTest() {}

  void JoinThreadFunc() {
    usleep(100);

    for (size_t i = 0; i < threads_.size(); ++i) {
      threads_[i]->Quit();
      if (threads_[i]->GetThread()->Join() == true) {
        ++this->join_count_;
      }
    }

    for (size_t i = 0; i < detach_threads_.size(); ++i) {
      detach_threads_[i]->Quit();
      if (detach_threads_[i]->GetThread()->Join() == true) {
        ++this->detach_count_;
      }
    }
  }

  bool Start(size_t thread_num) {
    for (size_t i = 0; i < thread_num; ++i) {
      JoinThread *t = new JoinThread();
      t->Start();
      this->threads_.push_back(t);
    }

    for (size_t i = 0; i < thread_num; ++i) {
      JoinThread *t = new JoinThread();
      t->Start(false);
      this->detach_threads_.push_back(t);
    }

    join_thread1_.Start(std::bind(&JoinTest::JoinThreadFunc, this));
    join_thread2_.Start(std::bind(&JoinTest::JoinThreadFunc, this));

    return true;
  }

  size_t join_count_;
  size_t detach_count_;

  Thread join_thread1_;
  Thread join_thread2_;

 private:
  std::vector<JoinThread *> threads_;
  std::vector<JoinThread *> detach_threads_;
};



void JoinTestFunc() {
  JoinTest jt;
  jt.Start(10);

  jt.join_thread1_.Join();
  jt.join_thread2_.Join();

  if (jt.join_count_ != 10) {
    printf("Failed JoinTestFunc, join_count_(%ld).\n", jt.join_count_);
  }

  if (jt.detach_count_ != 0) {
    printf("Failed JoinTestFunc, detach_count_(%ld).\n", jt.detach_count_);
  }
}

}  // namespace test
}  // namespace ioevent
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::ioevent::test::SimpleTestFunc();
  ::mysya::ioevent::test::JoinTestFunc();

  return 0;
}
