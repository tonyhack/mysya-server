#include <libgen.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <zlib.h>

#include <string>
#include <vector>

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/scene/scene.h"
#include "tutorial/orcas/combat/server/scene/scene_manager.h"

class CommandLineOption {
 public:
  CommandLineOption() {}
  ~CommandLineOption() {}

  bool Parse(int argc, char *argv[]);

  std::string error_;
  std::string conf_file_;
  std::string cmd_dirname_;
  std::string cmd_basename_;
};

bool CommandLineOption::Parse(int argc, char *argv[]) {
  int opt = -1;

  std::vector<char> buffer(argv[0], argv[0] + strlen(argv[0]) + 1);
  this->cmd_dirname_ = ::dirname(&buffer[0]);
  this->cmd_basename_ = ::dirname(&buffer[0]);

  opterr = 0;

  while ((opt = ::getopt(argc, argv, "c:")) != -1) {
    switch (opt) {
      case 'c':
        this->conf_file_ = optarg;
        break;
      default:
        this->error_ = "invalid option";
        return false;
    }
  }

  if (this->conf_file_.empty() == true) {
    this->error_ = "invalid option c";
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  CommandLineOption option;
  if (option.Parse(argc, argv) == false) {
    MYSYA_ERROR("Option::Parse failed(%s).", option.error_.data());
    return -1;
  }

  using namespace ::tutorial::orcas::combat::server::scene;

  if (SceneManager::GetInstance()->LoadConfig(option.conf_file_) == false) {
    MYSYA_ERROR("SceneManager::LoadConfig(%s) failed.", option.conf_file_.data());
    return -1;
  }

  Scene *scene = SceneManager::GetInstance()->Allocate(1);
  if (scene == NULL) {
    MYSYA_ERROR("SceneManager::Allocate(1) failed.");
    return -1;
  }

  if (scene->Initialize(NULL) == false) {
    MYSYA_ERROR("Scene::Initialize() failed.");
    return -1;
  }

  ::protocol::Position begin_pos;
  ::protocol::Position end_pos;
  Scene::PositionVector paths;

  begin_pos.set_x(9);
  begin_pos.set_y(62);
  end_pos.set_x(113);
  end_pos.set_y(59);
  scene->PrintSearchPath(begin_pos, end_pos);

  return 0;
}
