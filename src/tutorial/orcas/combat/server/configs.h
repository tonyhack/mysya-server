#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_CONFIGS_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_CONFIGS_H

#include <string>
#include <vector>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class Configs {
 public:
  bool Load(const std::string &file);

  int server_id_;
  std::string listen_host_;
  int listen_port_;

  size_t combat_initial_size_;
  size_t combat_extend_size_;
  size_t combat_building_initial_size_;
  size_t combat_building_extend_size_;
  size_t combat_role_initial_size_;
  size_t combat_role_extend_size_;
  size_t combat_warrior_initial_size_;
  size_t combat_warrior_extend_size_;

  std::string conf_path_;

 private:
  MYSYA_SINGLETON(Configs);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_CONFIGS_H
