#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_MANAGER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_MANAGER_H

#include <list>
#include <string>
#include <unordered_map>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace scene {

class Scene;

struct SceneConf {
  SceneConf()
    : map_id_(0), height_(0), width_(0) {}

  int32_t map_id_;
  int32_t height_;
  int32_t width_;
  std::string blocks_;
};

class SceneManager {
 public:
  typedef std::unordered_map<int32_t, Scene *> SceneHashmap;
  typedef std::unordered_map<int32_t, std::string> BlockHashmap;
  typedef std::unordered_map<int32_t, SceneConf> SceneConfHashmap;
  typedef std::list<Scene *> SceneList;

  bool LoadConfig(const std::string &file);

  Scene *Allocate(int32_t map_id);
  void Deallocate(Scene *scene);

  bool Add(Scene *scene);
  Scene *Get(int32_t scene_id);
  Scene *Remove(int32_t scene_id);

  const SceneConf *GetSceneConf(int32_t map_id);

 private:
  bool LoadBlockFile(int32_t map_id, const std::string &file);

  SceneHashmap scenes_;
  SceneList pool_;
  SceneConfHashmap scene_confs_;

  MYSYA_SINGLETON(SceneManager);
};

}  // namespace scene
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_SCENE_SCENE_MANAGER_H
