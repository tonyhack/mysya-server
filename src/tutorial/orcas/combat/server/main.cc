#include <libgen.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <functional>
#include <map>
#include <string>

#include <google/protobuf/stubs/common.h>
#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/socket_address.h>

#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/configs.h"

class CommandLineOption {
 public:
  CommandLineOption() : log_level_(0), daemon_(0) {}
  ~CommandLineOption() {}

  bool Parse(int argc, char *argv[]);

  std::string error_;
  std::string cmd_dirname_;
  std::string cmd_basename_;
  std::string conf_file_;
  std::string pid_file_;
  std::string log_dir_;
  std::string stderr_file_;
  std::string stdout_file_;
  int log_level_;
  int daemon_;
};

bool CommandLineOption::Parse(int argc, char *argv[]) {
  int opt = -1;

  std::vector<char> buffer(argv[0], argv[0] + strlen(argv[0]) + 1);
  this->cmd_dirname_ = ::dirname(&buffer[0]);
  this->cmd_basename_ = ::dirname(&buffer[0]);

  opterr = 0;

  while ((opt = ::getopt(argc, argv, "c:p:L:e:o:l:d:")) != -1) {
    switch (opt) {
      case 'c':
        this->conf_file_ = optarg;
        break;
      case 'p':
        this->pid_file_ = optarg;
        break;
      case 'L':
        this->log_dir_ = optarg;
        break;
      case 'e':
        this->stderr_file_ = optarg;
        break;
      case 'o':
        this->stdout_file_ = optarg;
        break;
      case 'l':
        this->log_level_ = atoi(optarg);
        break;
      case 'd':
        this->daemon_ = atoi(optarg);
        break;
      default:
        this->error_ = "invalid option";
        return false;
    }
  }

  if (this->conf_file_.empty() == true) {
    this->error_ += "missing option -c;";
  }
  if (this->pid_file_.empty() == true) {
    this->error_ += "missing option -p;";
  }
  if (this->log_dir_.empty() == true) {
    this->log_dir_ = ".log/";
  }
  if (this->stderr_file_.empty() == true) {
    this->stderr_file_ = "./stderr.log";
  }
  if (this->stdout_file_.empty() == true) {
    this->stdout_file_ = "./stdout.log";
  }

  return true;
}

int main(int argc, char *argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  CommandLineOption option;
  if (option.Parse(argc, argv) == false) {
    MYSYA_ERROR("Option::Parse failed(%s).", option.error_.data());
    return -1;
  }

  if (option.daemon_ > 0) {
    ::freopen("/dev/null", "r", stdin);
    ::freopen(option.stdout_file_.data(), "a", stdout);
    ::freopen(option.stderr_file_.data(), "a", stderr);

    ::setvbuf(stdout, NULL, _IONBF, 0);
    ::setvbuf(stderr, NULL, _IONBF, 0);
    ::setvbuf(stdin, NULL, _IONBF, 0);

    if (daemon(1, 1) != 0) {
      MYSYA_ERROR("daemon() failed.");
      return -1;
    }
  }

  if (::tutorial::orcas::combat::server::Configs::GetInstance()->Load(
        option.conf_file_) == false) {
      MYSYA_ERROR("Configs::Load(%s) failed.", option.conf_file_.data());
      return -1;
  }

  ::mysya::ioevent::SocketAddress listen_addr(
      ::tutorial::orcas::combat::server::Configs::GetInstance()->listen_host_,
      ::tutorial::orcas::combat::server::Configs::GetInstance()->listen_port_);

  ::mysya::ioevent::EventLoop event_loop;
  std::unique_ptr< ::tutorial::orcas::combat::server::AppServer> app_server(
      new (std::nothrow) ::tutorial::orcas::combat::server::AppServer(&event_loop, 128));
  if (app_server.get() == NULL) {
    MYSYA_ERROR("Allocate AppServer failed.");
    return -1;
  }

  if (app_server->Listen(listen_addr) == false) {
    MYSYA_ERROR("AppServer::Listen(%s:%d) failed.",
        ::tutorial::orcas::combat::server::Configs::GetInstance()->listen_host_.data(),
        ::tutorial::orcas::combat::server::Configs::GetInstance()->listen_port_);
    return -1;
  }

  event_loop.Loop();

  ::google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
