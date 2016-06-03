#include "tutorial/orcas/combat/server/ai/event_observer.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/server/ai/auto.h"
#include "tutorial/orcas/combat/server/ai/auto_manager.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

MYSYA_SINGLETON_IMPL(EventObserver);

EventObserver::EventObserver() : dispatching_(false) {}
EventObserver::~EventObserver() {}

void EventObserver::Add(int32_t combat_id, int32_t entity_id,
    int32_t auto_id) {
  ObserverKey key;
  key.combat_id_ = combat_id;
  key.entity_id_ = entity_id;

  ObserverHashmap::iterator iter = this->observers_.find(key);
  if (iter == this->observers_.end()) {
    AutoSet autos;
    iter = this->observers_.insert(std::make_pair(key, autos)).first;
  }

  iter->second.insert(auto_id);
}

void EventObserver::Remove(int32_t combat_id, int32_t entity_id,
    int32_t auto_id) {
  if (this->dispatching_ == true) {
    this->PendingRemove(combat_id, entity_id, auto_id);
    return;
  }

  ObserverKey key;
  key.combat_id_ = combat_id;
  key.entity_id_ = entity_id;

  ObserverHashmap::iterator iter = this->observers_.find(key);
  if (iter == this->observers_.end()) {
    return;
  }

  iter->second.erase(auto_id);
  if (iter->second.empty() == true) {
    this->observers_.erase(key);
  }
}

void EventObserver::Dispatch(int32_t combat_id, int32_t entity_id,
    int type, const ProtoMessage *data) {
  ObserverKey key;
  key.combat_id_ = combat_id;
  key.entity_id_ = entity_id;

  ObserverHashmap::iterator iter = this->observers_.find(key);
  if (iter == this->observers_.end()) {
    return;
  }

  this->dispatching_ = true;

  for (AutoSet::iterator auto_iter = iter->second.begin();
      auto_iter != iter->second.end(); ++auto_iter) {
    Auto *autoz = AutoManager::GetInstance()->Get(combat_id, *auto_iter);
    if (autoz == NULL) {
      MYSYA_ERROR("[AI] AutoManager::Get(%d, %d) failed in dispatch event(%d).",
          combat_id, *auto_iter, type);
      continue;
    }

    autoz->GetPresentStatus()->DispatchEvent(type, data);
  }

  this->dispatching_ = false;

  for (PendingRemoveAutoVector::iterator iter = this->pending_removes_.begin();
      iter != this->pending_removes_.end(); ++iter) {
    this->Remove(iter->first.combat_id_, iter->first.entity_id_, iter->second);
  }
  this->pending_removes_.clear();
}

void EventObserver::PendingRemove(int32_t combat_id, int32_t entity_id,
    int32_t auto_id) {
  ObserverKey key;
  key.combat_id_ = combat_id;
  key.entity_id_ = entity_id;
  this->pending_removes_.push_back(std::make_pair(key, auto_id));
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
