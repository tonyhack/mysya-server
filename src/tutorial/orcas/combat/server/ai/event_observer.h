#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_EVENT_OBSERVER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_EVENT_OBSERVER_H

#include <map>
#include <set>
#include <unordered_map>

#include <mysya/util/class_util.h>

#include "tutorial/orcas/protocol/cc/combat.pb.h"

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

class EventObserver {
 public:
  typedef ::google::protobuf::Message ProtoMessage;

  struct ObserverKey {
    int32_t combat_id_;
    int32_t entity_id_;

    bool operator==(const ObserverKey &k) const {
      return this->combat_id_ == k.combat_id_ &&
        this->entity_id_ == k.entity_id_;
    }
  };

  struct ObserverKeyHash {
    std::size_t operator()(const ObserverKey &k) const {
      return ((size_t)k.combat_id_<< 32) + (size_t)k.entity_id_;
    }
  };

  typedef std::set<int32_t> AutoSet;
  typedef std::unordered_map<ObserverKey, AutoSet, ObserverKeyHash>
          ObserverHashmap;

  void Add(int32_t combat_id, int32_t entity_id, int32_t auto_id);
  void Remove(int32_t combat_id, int32_t entity_id, int32_t auto_id);
  void Dispatch(int32_t combat_id, int32_t entity_id, int type,
      const ProtoMessage *data);

 private:
  ObserverHashmap observers_;

  MYSYA_SINGLETON(EventObserver);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_EVENT_OBSERVER_H
