#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_MAP_CONFIG_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_MAP_CONFIG_H

#include <map>
#include <string>
#include <vector>
#include <unordered_map>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

struct BattleBuildingConf {
  int type_;                // 类型
  int food_add_;            // 单位时间的粮草供给
  int supply_;              // 人口上限值
  int elixir_add_;          // 单位时间的法力值供给
};

struct BuildingConf {
  int id_;                  // id
  int map_id_;              // 地图id
  int camp_id_;             // 阵营id
  int type_;                // 类型
  int x_;                   // 坐标x
  int y_;                   // 坐标y
  int hp_;                  // 生命值
  int hp_recovery_;         // 生命恢复值
  int food_add_;            // 单位时间的粮草供给
  int supply_;              // 人口上限值
  int elixir_add_;          // 单位时间的法力值供给
};

struct MapConf {
  typedef std::vector<BuildingConf *> BuildingVector;
  typedef std::map<int, BuildingVector> BuildingVectorMap;

  int id_;
  int map_id_;
  BuildingVectorMap buildings_;
};

class MapConfig {
 public:
  typedef std::unordered_map<int, BattleBuildingConf> BattleBuildingHashmap;
  typedef std::unordered_map<int, BuildingConf> BuildingHashmap;
  typedef std::unordered_map<int, MapConf> MapHashmap;

  bool LoadBattleBuilding(const std::string &file);
  bool LoadBuilding(const std::string &file);
  bool LoadMap(const std::string &file);

  const MapConf *GetMapConf(int id) const;

 private:
  BattleBuildingHashmap battle_buildings_;
  BuildingHashmap buildings_;
  MapHashmap maps_;

  MYSYA_SINGLETON(MapConfig);
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_MAP_CONFIG_H
