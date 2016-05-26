#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_VOTE_DISPATCHER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_VOTE_DISPATCHER_H

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

class VoteDispatcher {
  typedef ::google::protobuf::Message ProtoMessage;

  typedef std::function<int (const ProtoMessage *)> VoteCallback;
  typedef std::list<VoteCallback> VoteCallbackList;
  typedef VoteCallbackList::iterator VoteCallbackListIter;
  typedef std::unordered_map<int32_t, VoteCallbackList>
    VoteCallbackListHashmap;
  typedef std::unordered_map<int32_t, VoteCallbackListIter>
    VoteCallbackListIterHashmap;

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
  VoteDispatcher();
  ~VoteDispatcher();

  uint64_t Attach(int32_t type, const VoteCallback &cb);
  void Detach(uint64_t token);
  int Dispatch(int32_t type, const ProtoMessage *vote);

 private:
  TokenAllocator token_allocator_;

  VoteCallbackListHashmap cbs_;
  VoteCallbackListIterHashmap tokens_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(VoteDispatcher);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_VOTE_DISPATCHER_H
