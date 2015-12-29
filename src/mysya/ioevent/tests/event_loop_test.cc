#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include <mysya/ioevent/event_channel.h>
#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/dynamic_buffer.h>

namespace mysya {
namespace ioevent {
namespace test {

class CommandLine {
 public:
  typedef std::vector<char> BufferVector;

  CommandLine() : buffer_(16, 8) {}
  ~CommandLine() {}

  void Run() {
    this->channel_.SetFileDescriptor(0);
    this->channel_.SetNonblock();
    this->channel_.SetReadCallback(
        std::bind(&CommandLine::OnStdinCallback, this, std::placeholders::_1));

    this->channel_.AttachEventLoop(&this->loop_);

    this->PrintPrompt();
    this->loop_.Loop();

    this->channel_.ResetReadCallback();
  }

 private:
  void OnStdinCallback(void *channel_data) {
    EventChannel *channel = (EventChannel *)(channel_data);

    for (;;) {
      this->buffer_.ReserveWritableBytes(8);
      int bytes = ::read(channel->GetFileDescriptor(), this->buffer_.WriteBegin(),
          this->buffer_.WritableBytes());
      if (bytes == 0) {
        return;
      } else if (bytes == -1) {
        if (errno != EAGAIN) {
          this->loop_.Quit();
          ::printf("recv error.");
          return;
        }
        return;
      }

      this->buffer_.WrittenBytes(bytes);

      while (true) {
        const char *read_begin = this->buffer_.ReadBegin();
        const char *new_line = NULL;

        for (size_t i = 0; i < this->buffer_.ReadableBytes(); ++i) {
          if (read_begin[i] == '\n') {
            new_line = &read_begin[i];
            break;
          }
        }

        if (new_line == NULL) {
          break;
        }

        size_t line_size = new_line - read_begin;
        std::string user_command(read_begin, line_size);

        if (user_command == "q" ||
            user_command == "quit" ||
            user_command == "exit") {
          this->loop_.Quit();
          return;
        } else {
          ::printf("%s: command not found\n", user_command.data());
        }

        this->buffer_.ReadBytes(line_size + 1);

        this->PrintPrompt();
      }
    }
  }

  void PrintPrompt() {
    ::printf("\033[;36m");
    ::printf("mysya> ");
    ::printf("\033[0m");
    fflush(stdout);
  }

  EventChannel channel_;
  EventLoop loop_;

  DynamicBuffer buffer_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CommandLine);
};

void CommandLineTest() {
  CommandLine cl;
  cl.Run();
}

}  // namespace test
}  // namespace ioevent
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::ioevent::test::CommandLineTest();

  return 0;
}
