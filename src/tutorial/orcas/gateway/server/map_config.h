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

struct BuildingConf {
  int id_;
  int map_id_;
  int camp_id_;
  int type_;
  int x_;
  int y_;
  int hp_;
};

struct MapConf {
  typedef std::vector<BuildingConf *> BuildingVector;
  typedef std::map<int, BuildingVector> BuildingVectorMap;

  int id_;
  BuildingVectorMap buildings_;
};

class MapConfig {
 public:
  typedef std::unordered_map<int, MapConf> MapHashmap;
  typedef std::unordered_map<int, BuildingConf> BuildingHashmap;

  bool LoadBuilding(const std::string &file);
  bool LoadMap(const std::string &file);

  const MapConf *GetMapConf(int id) const;

 private:
  MapHashmap maps_;
  BuildingHashmap buildings_;

  MYSYA_SINGLETON(MapConfig);
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_MAP_CONFIG_H
