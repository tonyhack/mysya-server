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
  int id_;                        // id
  std::string name_;              // 名称
  int type_;                      // 类型
  int num_;                       // 数量
  int food_need_;                 // 招募粮草需求
  int supply_need_;               // 招募人口需求
  int hp_;                        // 生命值
  int attack_;                    // 攻击力
  int defence_;                   // 防御力
  int move_speed_;                // 移动速度
  int attack_speed_;              // 攻击速度
  int attack_range_;              // 攻击范围
  int sight_;                     // 视野范围
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
