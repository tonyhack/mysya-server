#include "tutorial/orcas/combat/server/scene/scene_manager.h"

#include <zlib.h>

#include <string>
#include <vector>

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/base64.h"
#include "tutorial/orcas/combat/server/configs.h"
#include "tutorial/orcas/combat/server/scene/scene.h"
#include "tutorial/orcas/combat/server/scene/scene.h"
#include "tutorial/orcas/deps/tinyxml/tinyxml.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

MYSYA_SINGLETON_IMPL(SceneManager);

SceneManager::SceneManager() {}
SceneManager::~SceneManager() {
  for (SceneHashmap::iterator iter = this->scenes_.begin();
      iter != this->scenes_.end(); ++iter) {
    delete iter->second;
  }
  this->scenes_.clear();

  for (SceneList::iterator iter = this->pool_.begin();
      iter != this->pool_.end(); ++iter) {
    delete *iter;
  }
  this->pool_.clear();

  this->scene_confs_.clear();
}

bool SceneManager::LoadConfig(const std::string &file) {
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
    int32_t map_id = 0;
    std::string block_file_name;

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
        MYSYA_ERROR("Config(%s) /return/row/field[text] null.",
            file.data());
        return false;
      }
      std::string attr_name_str = attr_name;

      if (attr_name_str == "id") {
        map_id = atoi(attr);
      } else if (attr_name_str == "map_build_id") {
      } else if (attr_name_str == "tilemap") {
        block_file_name = Configs::GetInstance()->conf_path_ + attr;
      } else {
      }

      field_node = field_node->NextSiblingElement("field");
    }

    if (this->LoadBlockFile(map_id, block_file_name) == false) {
      MYSYA_ERROR("LoadBlockFile(%d, %s) failed.",
          map_id, block_file_name.data());
      return false;
    }

    row_node = row_node->NextSiblingElement("row");
  }

  return true;
}

Scene *SceneManager::Allocate(int32_t map_id) {
  Scene *scene = NULL;

  if (this->pool_.empty() == true) {
    scene = new (std::nothrow) Scene();
  } else {
    SceneList::iterator iter = this->pool_.begin();
    scene = *iter;
    this->pool_.erase(iter);
  }

  return scene;
}

void SceneManager::Deallocate(Scene *scene) {
  this->pool_.push_back(scene);
}

bool SceneManager::Add(Scene *scene) {
  SceneHashmap::iterator iter = this->scenes_.find(scene->GetId());
  if (iter != this->scenes_.end()) {
    return false;
  }

  this->scenes_.insert(std::make_pair(scene->GetId(), scene));
  return true;
}

Scene *SceneManager::Get(int32_t scene_id) {
  Scene *scene = NULL;

  SceneHashmap::iterator iter = this->scenes_.find(scene_id);
  if (iter != this->scenes_.end()) {
    scene = iter->second;
  }

  return scene;
}

Scene *SceneManager::Remove(int32_t scene_id) {
  Scene *scene = NULL;

  SceneHashmap::iterator iter = this->scenes_.find(scene_id);
  if (iter != this->scenes_.end()) {
    scene = iter->second;
    this->scenes_.erase(iter);
  }

  return scene;
}

const SceneConf *SceneManager::GetSceneConf(int32_t map_id) {
  const SceneConf *conf = NULL;

  SceneConfHashmap::const_iterator iter = this->scene_confs_.find(map_id);
  if (iter != this->scene_confs_.end()) {
    conf = &iter->second;
  }

  return conf;
}

bool SceneManager::LoadBlockFile(int32_t map_id, const std::string &file) {
  TiXmlDocument doc;
  if (doc.LoadFile(file.data()) == false) {
    MYSYA_ERROR("Load config(%s) failed.", file.data());
    return false;
  }

  TiXmlElement *map_node = doc.FirstChildElement("map");
  if (map_node == NULL) {
    MYSYA_ERROR("Config(%s) /map not found.", file.data());
    return false;
  }

  SceneConf conf;
  const char *attr = NULL;
  TiXmlElement *path_layer_node = NULL;

  TiXmlElement *layer_node = map_node->FirstChildElement("layer");
  while (layer_node != NULL) {
    attr = layer_node->Attribute("name");
    if (attr == NULL) {
      MYSYA_ERROR("Config(%s) /map/layer[name] not found.",
          file.data());
      return false;
    }

    if (std::string(attr) == "path") {
      path_layer_node = layer_node;
      break;
    }

    layer_node = layer_node->NextSiblingElement("layer");
  }

  if (path_layer_node == NULL) {
    MYSYA_ERROR("Config(%s) not found path layer.",
        file.data());
    return false;
  }

  attr = path_layer_node->Attribute("width");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /map/layer[width] not found.",
        file.data());
    return false;
  }
  conf.width_ = atoi(attr);

  attr = path_layer_node->Attribute("height");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /map/layer[height] not found.",
        file.data());
    return false;
  }
  conf.height_ = atoi(attr);

  TiXmlElement *data_node = path_layer_node->FirstChildElement("data");
  if (data_node == NULL) {
    MYSYA_ERROR("Config(%s) /map/layer/data not found.",
        file.data());
    return false;
  }

  attr = data_node->Attribute("encoding");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /map/layer/data[encoding] not found.",
        file.data());
    return false;
  }
  if (std::string(attr) == "base64") {
    MYSYA_ERROR("Config(%s) /map/layer/data[encoding] is not base64.",
        file.data());
    return false;
  }

  attr = data_node->Attribute("compression");
  if (attr == NULL) {
    MYSYA_ERROR("Config(%s) /map/layer/data[compression] not found.",
        file.data());
    return false;
  }
  if (std::string(attr) == "zlib") {
    MYSYA_ERROR("Config(%s) /map/layer/data[compression] is not zlib.",
        file.data());
    return false;
  }

  attr = data_node->GetText();
  int attr_size = strlen(attr);

  std::string decode_str = base64_decode(attr, attr_size);
  uLongf decode_str_size = decode_str.size();

  uLongf buffer_size = conf.width_ * conf.height_ * 4;
  std::vector<char> buffer;
  buffer.resize(buffer_size);

  int ret = ::uncompress((Bytef *)&buffer[0], &buffer_size,
      (const Bytef *)decode_str.data(), decode_str_size);
  if (ret != Z_OK) {
    MYSYA_ERROR("::uncompress() failed(%d).", ret);
    return false;
  }

  for (size_t i = 0; i < buffer_size; ++i) {
    if (*(int *)(&buffer[i]) > 0) {
      conf.blocks_ += "0";
    } else {
      conf.blocks_ += "1";
    }
  }

  this->scene_confs_.insert(std::make_pair(map_id, conf));

  return true;
}

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
