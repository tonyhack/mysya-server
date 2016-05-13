#include "tutorial/orcas/combat/server/configs.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/deps/tinyxml/tinyxml.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

MYSYA_SINGLETON_IMPL(Configs);

Configs::Configs() {}
Configs::~Configs() {}

bool Configs::Load(const std::string &file) {
  TiXmlDocument doc;

  if (doc.LoadFile(file.data()) == false) {
    MYSYA_ERROR("Load conf(%s) failed.", file.data());
    return false;
  }

  TiXmlElement *config_node = doc.FirstChildElement("config");
  if (config_node == NULL) {
    MYSYA_ERROR("Config(%s) /config not found.", file.data());
    return false;
  }

  TiXmlElement *listen_node = config_node->FirstChildElement("listen");
  if (listen_node == NULL) {
    MYSYA_ERROR("Config(%s) /config/listen not found.", file.data());
    return false;
  }

  const char *attr = NULL;

  attr = listen_node->Attribute("id");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/listen[id] not found.", file.data());
    return false;
  }
  this->server_id_ = atoi(attr);

  attr = listen_node->Attribute("ip");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/listen[ip] not found.", file.data());
    return false;
  }
  this->listen_host_ = attr;

  attr = listen_node->Attribute("port");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/listen[port] not found.", file.data());
    return false;
  }
  this->listen_port_ = atoi(attr);

  TiXmlElement *pool_node = config_node->FirstChildElement("pool");
  if (pool_node == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool not found.", file.data());
    return false;
  }

  TiXmlElement *combat_node = pool_node->FirstChildElement("combat");
  if (combat_node == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat not found.", file.data());
    return false;
  }

  attr = combat_node->Attribute("initial_size");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat[initial_size] not found.",
        file.data());
    return false;
  }
  this->combat_initial_size_ = (size_t)atoi(attr);

  attr = combat_node->Attribute("extend_size");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat[extend_size] not found.",
        file.data());
    return false;
  }
  this->combat_extend_size_ = (size_t)atoi(attr);

  TiXmlElement *combat_building_node = pool_node->FirstChildElement("combat_building");
  if (combat_building_node == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat_building not found.",
        file.data());
    return false;
  }

  attr = combat_building_node->Attribute("initial_size");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat_building[initial_size] not found.",
        file.data());
    return false;
  }
  this->combat_building_initial_size_ = (size_t)atoi(attr);

  attr = combat_building_node->Attribute("extend_size");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat_building[extend_size] not found.",
        file.data());
    return false;
  }
  this->combat_building_extend_size_ = (size_t)atoi(attr);

  TiXmlElement *combat_role_node = pool_node->FirstChildElement("combat_role");
  if (combat_role_node == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat_role not found.",
        file.data());
    return false;
  }

  attr = combat_role_node->Attribute("initial_size");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat_role[initial_size] not found.",
        file.data());
    return false;
  }
  this->combat_role_initial_size_ = (size_t)atoi(attr);

  attr = combat_role_node->Attribute("extend_size");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat_role[extend_size] not found.",
        file.data());
    return false;
  }
  this->combat_role_extend_size_ = (size_t)atoi(attr);

  TiXmlElement *combat_warrior_node = pool_node->FirstChildElement("combat_warrior");
  if (combat_warrior_node == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat_warrior not found.",
        file.data());
    return false;
  }

  attr = combat_warrior_node->Attribute("initial_size");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat_warrior[initial_size] not found.",
        file.data());
    return false;
  }
  this->combat_warrior_initial_size_ = (size_t)atoi(attr);

  attr = combat_warrior_node->Attribute("extend_size");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/pool/combat_warrior[extend_size] not found.",
        file.data());
    return false;
  }
  this->combat_warrior_extend_size_ = (size_t)atoi(attr);

  TiXmlElement *conf_path_node = config_node->FirstChildElement("conf_path");
  if (conf_path_node == NULL) {
    MYSYA_ERROR("Config(%s) /config/confs not found.", file.data());
    return false;
  }

  attr = conf_path_node->Attribute("value");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /config/conf_path[value] not found.",
        file.data());
    return false;
  }
  this->conf_path_ = attr;

  return true;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
