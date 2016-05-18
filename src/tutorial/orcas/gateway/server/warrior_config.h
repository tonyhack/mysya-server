#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_WARRIOR_CONFIG_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_WARRIOR_CONFIG_H

#include <string>
#include <unordered_map>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

struct WarriorConf {
  int id_;
  std::string name_;
  int type_;
  int num_;
  int food_need_;
  int hp_;
  int attack_;
  int defence_;
  int move_speed_;
  int attack_speed_;
  int attack_range_;
  int sight_;
};

class WarriorConfig {
 public:
  typedef std::unordered_map<int, WarriorConf> WarriorHashmap;

  bool Load(const std::string &file);

  const WarriorConf *GetWarriorConf(int id) const;
  const WarriorHashmap &GetWarriors() const;

 private:
  WarriorHashmap warriors_;

  MYSYA_SINGLETON(WarriorConfig);
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_WARRIOR_CONFIG_H
