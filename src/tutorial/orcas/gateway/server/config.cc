#include "tutorial/orcas/gateway/server/config.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/deps/tinyxml/tinyxml.h"

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

  return true;
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
