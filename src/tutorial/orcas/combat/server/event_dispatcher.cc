#include "tutorial/orcas/combat/server/event_dispatcher.h"

#include <google/protobuf/message.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

EventDispatcher::EventDispatcher() {}
EventDispatcher::~EventDispatcher() {}

uint64_t EventDispatcher::Attach(int32_t type, const EventCallback &cb) {
  EventCallbackListHashmap::iterator iter = this->cbs_.find(type);
  if (iter == this->cbs_.end()) {
    EventCallbackList cbs;
    iter = this->cbs_.insert(std::make_pair(type, cbs)).first;
  }

  uint64_t token = this->token_allocator_.Allocate(type);
  EventCallbackListIter iter2 = iter->second.insert(iter->second.end(), cb);
  this->tokens_[token] = iter2;

  return token;
}

void EventDispatcher::Detach(uint64_t token) {
  EventCallbackListIterHashmap::iterator iter = this->tokens_.find(token);
  if (iter == this->tokens_.end()) {
    return;
  }

  int32_t type = this->token_allocator_.GetType(token);
  EventCallbackListHashmap::iterator iter2 = this->cbs_.find(type);
  if (iter2 != this->cbs_.end()) {
    iter2->second.erase(iter->second);
  }

  this->tokens_.erase(iter);
}

void EventDispatcher::Dispatch(int32_t type, const Message *event) {
  EventCallbackListHashmap::iterator iter = this->cbs_.find(type);
  if (iter == this->cbs_.end()) {
    return;
  }

  EventCallbackList &cbs = iter->second;

  for (EventCallbackListIter iter2 = cbs.begin(); iter2 != cbs.end();) {
    EventCallbackListIter iter3 = iter2;
    ++iter3;

    if (*iter2) {
      (*iter2)(event);
    }

    iter2 = iter3;
  }
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
