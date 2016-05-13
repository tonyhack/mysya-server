#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_EVENT_DISPATCHER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_EVENT_DISPATCHER_H

#include <stdint.h>

#include <functional>
#include <list>
#include <unordered_map>

#include <mysya/util/class_util.h>

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class EventDispatcher {
  typedef ::google::protobuf::Message Message;

  typedef std::function<void (const Message *)> EventCallback;

  typedef std::list<EventCallback> EventCallbackList;
  typedef EventCallbackList::iterator EventCallbackListIter;
  typedef std::unordered_map<int32_t, EventCallbackList>
    EventCallbackListHashmap;
  typedef std::unordered_map<uint64_t, EventCallbackListIter>
    EventCallbackListIterHashmap;

  class TokenAllocator {
   public:
    TokenAllocator() : value_(0) {}
    ~TokenAllocator() {}
  
    uint64_t Allocate(int32_t type) {
      return ((uint64_t)this->value_ << 32) + (uint64_t)type;
    }
    int32_t GetType(uint64_t token) {
      return token & 0xFFFFFFFF;
    }
  
   private:
    uint32_t value_;
  };

 public:
  EventDispatcher();
  ~EventDispatcher();

  uint64_t Attach(int32_t type, const EventCallback &cb);
  void Detach(uint64_t token);
  void Dispatch(int32_t type, const Message *event);

 private:
  TokenAllocator token_allocator_;

  EventCallbackListHashmap cbs_;
  EventCallbackListIterHashmap tokens_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(EventDispatcher);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_EVENT_DISPATCHER_H
