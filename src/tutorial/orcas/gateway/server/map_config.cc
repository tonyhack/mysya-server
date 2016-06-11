#include "tutorial/orcas/gateway/server/map_config.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/deps/tinyxml/tinyxml.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

MYSYA_SINGLETON_IMPL(MapConfig);

MapConfig::MapConfig() {}
MapConfig::~MapConfig() {}

bool MapConfig::LoadBattleBuilding(const std::string &file) {
  TiXmlDocument doc;
  if (doc.LoadFile(file.data()) == false) {
    MYSYA_ERROR("Load config(%s) failed.", file.data());
    return false;
  }

  TiXmlElement *resultset_node = doc.FirstChildElement("resultset");
  if (resultset_node == NULL) {
    MYSYA_ERROR("Config(%s) /resultset not found.", file.data());
    return false;
  }

  const char *attr = NULL;
  const char *attr_name = NULL;

  TiXmlElement *row_node = resultset_node->FirstChildElement("row");
  while (row_node != NULL) {
    BattleBuildingConf conf;

    TiXmlElement *field_node = row_node->FirstChildElement("field");
    while (field_node != NULL) {
      attr_name = field_node->Attribute("name");
      if (attr_name == NULL) {
        MYSYA_ERROR("Config(%s) /resultset/row/field[name] not found.",
            file.data());
        return false;
      }

      attr = field_node->GetText();
      if (attr == NULL) {
        MYSYA_ERROR("Config(%s) /resultset/row/field[text] not found.",
            file.data());
        return false;
      }
      std::string attr_name_str = attr_name;

      if (attr_name_str == "id") {
        conf.type_ = atoi(attr);
      } else if (attr_name_str == "food_recovery") {
        conf.food_add_ = atoi(attr);
      } else if (attr_name_str == "supply") {
        conf.supply_ = atoi(attr);
      } else if (attr_name_str == "mana_recovery") {
        conf.elixir_add_ = atoi(attr);
      } else {
      }

      field_node = field_node->NextSiblingElement("field");
    }

    this->battle_buildings_.insert(std::make_pair(conf.type_, conf));

    row_node = row_node->NextSiblingElement("row");
  }

  return true;
}

bool MapConfig::LoadBuilding(const std::string &file) {
  TiXmlDocument doc;
  if (doc.LoadFile(file.data()) == false) {
    MYSYA_ERROR("Load config(%s) failed.", file.data());
    return false;
  }

  TiXmlElement *resultset_node = doc.FirstChildElement("resultset");
  if (resultset_node == NULL) {
    MYSYA_ERROR("Config(%s) /resultset not found.", file.data());
    return false;
  }

  const char *attr = NULL;
  const char *attr_name = NULL;

  TiXmlElement *row_node = resultset_node->FirstChildElement("row");
  while (row_node != NULL) {
    BuildingConf conf;

    TiXmlElement *field_node = row_node->FirstChildElement("field");
    while (field_node != NULL) {
      attr_name = field_node->Attribute("name");
      if (attr_name == NULL) {
        MYSYA_ERROR("Config(%s) /resultset/row/field[name] not found.",
            file.data());
        return false;
      }

      attr = field_node->GetText();
      if (attr == NULL) {
        MYSYA_ERROR("Config(%s) /resultset/row/field[text] not found.",
            file.data());
        return false;
      }
      std::string attr_name_str = attr_name;

      if (attr_name_str == "id") {
        conf.id_ = atoi(attr);
      } else if (attr_name_str == "map_build_id") {
        conf.map_id_ = atoi(attr);
      } else if (attr_name_str == "init_side") {
        conf.camp_id_ = atoi(attr);
      } else if (attr_name_str == "type_id") {
        conf.type_ = atoi(attr);
      } else if (attr_name_str == "pos_x") {
        conf.x_ = atoi(attr);
      } else if (attr_name_str == "pos_y") {
        conf.y_ = atoi(attr);
      } else if (attr_name_str == "hp") {
        conf.hp_ = atoi(attr);
      } else if (attr_name_str == "hp_recovery") {
        conf.hp_recovery_ = atoi(attr);
      } else {
      }

      field_node = field_node->NextSiblingElement("field");
    }

    BattleBuildingHashmap::const_iterator battle_building_iter =
      this->battle_buildings_.find(conf.type_);
    if (battle_building_iter == this->battle_buildings_.end()) {
      MYSYA_ERROR("Config(%s) error type_id(%d).", file.data(), conf.type_);
      return false;
    }

    conf.food_add_ = battle_building_iter->second.food_add_;
    conf.supply_ = battle_building_iter->second.supply_;
    conf.elixir_add_ = battle_building_iter->second.elixir_add_;

    this->buildings_.insert(std::make_pair(conf.id_, conf));

    row_node = row_node->NextSiblingElement("row");
  }

  return true;
}

bool MapConfig::LoadMap(const std::string &file) {
  TiXmlDocument doc;
  if (doc.LoadFile(file.data()) == false) {
    MYSYA_ERROR("Load config(%s) failed.", file.data());
    return false;
  }

  TiXmlElement *resultset_node = doc.FirstChildElement("resultset");
  if (resultset_node == NULL) {
    MYSYA_ERROR("Config(%s) /resultset not found.", file.data());
    return false;
  }

  const char *attr = NULL;
  const char *attr_name = NULL;

  TiXmlElement *row_node = resultset_node->FirstChildElement("row");
  while (row_node != NULL) {
    MapConf conf;

    TiXmlElement *field_node = row_node->FirstChildElement("field");
    while (field_node != NULL) {
      attr_name = field_node->Attribute("name");
      if (attr_name == NULL) {
        MYSYA_ERROR("Config(%s) /resultset/row/field[name] not found.",
            file.data());
        return false;
      }

      attr = field_node->GetText();
      if (attr == NULL) {
        MYSYA_ERROR("Config(%s) /resultset/row/field[text] not found.",
            file.data());
        return false;
      }
      std::string attr_name_str = attr_name;

      if (attr_name_str == "id") {
        conf.id_ = atoi(attr);
      } else if (attr_name_str == "map_build_id") {
        conf.map_id_ = atoi(attr);
      } else {
      }

      field_node = field_node->NextSiblingElement("field");
    }

    for (BuildingHashmap::iterator iter = this->buildings_.begin();
        iter != this->buildings_.end(); ++iter) {
      if (iter->second.map_id_ != conf.map_id_) {
        continue;
      }

      MapConf::BuildingVectorMap::iterator building_iter = conf.buildings_.find(
          iter->second.camp_id_);
      if (building_iter == conf.buildings_.end()) {
        MapConf::BuildingVector building_vector;
        building_iter = conf.buildings_.insert(std::make_pair(iter->second.camp_id_,
              building_vector)).first;
      }

      building_iter->second.push_back(&iter->second);
    }

    this->maps_.insert(std::make_pair(conf.id_, conf));

    row_node = row_node->NextSiblingElement("row");
  }

  return true;
}

const MapConf *MapConfig::GetMapConf(int id) const {
  const MapConf *conf = NULL;

  MapHashmap::const_iterator iter = this->maps_.find(id);
  if (iter != this->maps_.end()) {
    conf = &iter->second;
  }

  return conf;
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
