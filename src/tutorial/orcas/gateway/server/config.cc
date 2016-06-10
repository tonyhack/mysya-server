#include "tutorial/orcas/gateway/server/config.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/deps/tinyxml/tinyxml.h"
#include "tutorial/orcas/gateway/server/map_config.h"
#include "tutorial/orcas/gateway/server/warrior_config.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

MYSYA_SINGLETON_IMPL(Config);

Config::Config() {}
Config::~Config() {}

bool Config::Load(const std::string &file) {
  TiXmlDocument doc;

  if (doc.LoadFile(file.data()) == false) {
    MYSYA_ERROR("Load conf(%s) failed.", file.data());
    return false;
  }

  TiXmlElement *config_node = doc.FirstChildElement("config");
  if (config_node == NULL) {
    MYSYA_ERROR("Config file(%s) /config not found.", file.data());
    return false;
  }

  TiXmlElement *listen_node = config_node->FirstChildElement("listen");
  if (listen_node == NULL) {
    MYSYA_ERROR("Config file(%s) /config/listen not found.", file.data());
    return false;
  }

  const char *attr = NULL;

  attr = listen_node->Attribute("id");
  if (attr == NULL) {
    MYSYA_ERROR("Config file(%s) /config/listen[id] not found.", file.data());
    return false;
  }
  this->server_id_ = atoi(attr);

  attr = listen_node->Attribute("ip");
  if (attr == NULL) {
    MYSYA_ERROR("Config file(%s) /config/listen[ip] not found.", file.data());
    return false;
  }
  this->listen_host_ = attr;

  attr = listen_node->Attribute("port");
  if (attr == NULL) {
    MYSYA_ERROR("Config file(%s) /config/listen[port] not found.", file.data());
    return false;
  }
  this->listen_port_ = atoi(attr);

  TiXmlElement *combat_servers_node = config_node->FirstChildElement("combat_servers");
  if (combat_servers_node == NULL) {
    MYSYA_ERROR("Config file(%s) /config/combat_servers not found.", file.data());
    return false;
  }

  TiXmlElement *combat_server_node = combat_servers_node->FirstChildElement("value");
  while (combat_server_node != NULL) {
    CombatServerConf conf;

    attr = combat_server_node->Attribute("id");
    if (attr == NULL) {
      MYSYA_ERROR("Config file(%s) /config/combat_servers/value[id] not found.",
          file.data());
      return false;
    }
    conf.server_id_ = atoi(attr);

    attr = combat_server_node->Attribute("ip");
    if (attr == NULL) {
      MYSYA_ERROR("Config file(%s) /config/combat_servers/value[ip] not found.",
          file.data());
      return false;
    }
    conf.host_ = attr;

    attr = combat_server_node->Attribute("port");
    if (attr == NULL) {
      MYSYA_ERROR("Config file(%s) /config/combat_servers/value[port] not found.",
          file.data());
      return false;
    }
    conf.port_ = atoi(attr);

    this->combat_servers_.insert(std::make_pair(conf.server_id_, conf));

    combat_server_node = combat_server_node->NextSiblingElement("value");
  }

  TiXmlElement *configs_node = config_node->FirstChildElement("configs");
  if (configs_node == NULL) {
    MYSYA_ERROR("Config file(%s) /config/configs not found.", file.data());
    return false;
  }

  TiXmlElement *value_node = configs_node->FirstChildElement("value");
  while (value_node != NULL) {
    attr = value_node->Attribute("file");
    if (attr == NULL) {
      MYSYA_ERROR("Config file(%s) /config/configs[value] not found.",
          file.data());
      return false;
    }

    this->configs_.push_back(attr);

    value_node = value_node->NextSiblingElement("value");
  }

  for (ConfigVector::const_iterator iter = this->configs_.begin();
      iter != this->configs_.end(); ++iter) {
    if (this->OnLoadConfig(*iter) == false) {
      return false;
    }
  }

  return true;
}

bool Config::OnLoadConfig(const std::string &file) {
  std::string file_basename = ::basename(file.data());
  if (file_basename == "battle_build.xml") {
    return MapConfig::GetInstance()->LoadBattleBuilding(file);
  } else if (file_basename == "map_build.xml") {
    return MapConfig::GetInstance()->LoadBuilding(file);
  } else if (file_basename == "map.xml") {
    return MapConfig::GetInstance()->LoadMap(file);
  } else if (file_basename == "soldier.xml") {
    return WarriorConfig::GetInstance()->Load(file);
  } else {
    return false;
  }
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
