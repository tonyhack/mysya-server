#include <stdio.h>
#include <string.h>

#include <masya/event_channel.h>
#include <masya/event_loop.h>

namespace masya {
namespace test {

class CommandLine {
 public:
  CommandLine() {}
  ~CommandLine() {}

  void Run() {
    this->channel_.SetFileDescriptor(0);
    this->channel_.SetReadCallback(
        std::bind(&CommandLine::OnStdinCallback, this));

    this->channel_.AttachEventLoop(&this->loop_);

    this->PrintPrompt();
    this->loop_.Loop();
  }

 private:
  void OnStdinCallback() {
    char buffer[2048];
    char *ret = fgets(buffer, sizeof(buffer), stdin);
    if (NULL == ret) {
      this->loop_.Quit();
      return;
    }

    char *new_line = ::strstr(buffer, "\n");
    if (NULL == new_line) {
      this->loop_.Quit();
      return;
    }

    std::string user_command(buffer, new_line - buffer);

    if (user_command == "q" ||
        user_command == "quit" ||
        user_command == "exit") {
      this->loop_.Quit();
      return;
    } else {
      ::printf("%s: command not found\n", user_command.data());
    }

    this->PrintPrompt();
  }

  void PrintPrompt() {
    ::printf("\033[;36m");
    ::printf("robot> ");
    ::printf("\033[0m");
    fflush(stdout);
  }

  EventChannel channel_;
  EventLoop loop_;

  MASYA_DISALLOW_COPY_AND_ASSIGN(CommandLine);
};

void CommandLineTest() {
  CommandLine cl;
  cl.Run();
}

}  // namespace test
}  // namespace masya

int main(int argc, char *argv[]) {
  ::masya::test::CommandLineTest();

  return 0;
}
