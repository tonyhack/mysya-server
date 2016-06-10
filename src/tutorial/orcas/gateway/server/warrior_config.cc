#include "tutorial/orcas/gateway/server/warrior_config.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/deps/tinyxml/tinyxml.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

MYSYA_SINGLETON_IMPL(WarriorConfig);

WarriorConfig::WarriorConfig() {}
WarriorConfig::~WarriorConfig() {}

bool WarriorConfig::Load(const std::string &file) {
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
    WarriorConf conf;

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
        MYSYA_ERROR("Config(%s) /resultset/row/field(%s)[text] not found.",
            file.data(), attr_name);
        return false;
      }
      std::string attr_name_str = attr_name;

      if (attr_name_str == "id") {
        conf.id_ = atoi(attr);
      } else if (attr_name_str == "name") {
        conf.name_ = attr;
      } else if (attr_name_str == "series_id") {
        conf.type_ = atoi(attr);
      } else if (attr_name_str == "number") {
        conf.num_ = atoi(attr);
      } else if (attr_name_str == "food_need") {
        conf.food_need_ = atoi(attr);
      } else if (attr_name_str == "supply_need") {
        conf.supply_need_ = atoi(attr);
      } else if (attr_name_str == "hp") {
        conf.hp_ = atoi(attr);
      } else if (attr_name_str == "attack") {
        conf.attack_ = atoi(attr);
      } else if (attr_name_str == "armor") {
        conf.defence_ = atoi(attr);
      } else if (attr_name_str == "atk_speed") {
        conf.attack_speed_ = atoi(attr);
      } else if (attr_name_str == "atk_range") {
        conf.attack_range_ = atoi(attr);
      } else if (attr_name_str == "move_speed") {
        conf.move_speed_ = atoi(attr);
      } else if (attr_name_str == "sight") {
        conf.sight_ = atoi(attr);
      }

      field_node = field_node->NextSiblingElement("field");
    }

    this->warriors_.insert(std::make_pair(conf.id_, conf));

    row_node = row_node->NextSiblingElement("row");
  }

  return true;
}

const WarriorConf *WarriorConfig::GetWarriorConf(int id) const {
  const WarriorConf *conf = NULL;

  WarriorHashmap::const_iterator iter = this->warriors_.find(id);
  if (iter != this->warriors_.end()) {
    conf = &iter->second;
  }

  return conf;
}

const WarriorConfig::WarriorHashmap &WarriorConfig::GetWarriors() const {
  return this->warriors_;
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
