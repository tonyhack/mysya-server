#include "tutorial/orcas/combat/server/vote_dispatcher.h"

#include <google/protobuf/message.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

VoteDispatcher::VoteDispatcher() {}
VoteDispatcher::~VoteDispatcher() {}

uint64_t VoteDispatcher::Attach(int32_t type, const VoteCallback &cb) {
  VoteCallbackListHashmap::iterator iter = this->cbs_.find(type);
  if (iter == this->cbs_.end()) {
    VoteCallbackList cbs;
    iter = this->cbs_.insert(std::make_pair(type, cbs)).first;
  }

  uint64_t token = this->token_allocator_.Allocate(type);
  VoteCallbackListIter iter2 = iter->second.insert(iter->second.end(), cb);
  this->tokens_[token] = iter2;

  return token;
}

void VoteDispatcher::Detach(uint64_t token) {
  VoteCallbackListIterHashmap::iterator iter = this->tokens_.find(token);
  if (iter == this->tokens_.end()) {
    return;
  }

  int32_t type = this->token_allocator_.GetType(token);
  VoteCallbackListHashmap::iterator iter2 = this->cbs_.find(type);
  if (iter2 != this->cbs_.end()) {
    iter2->second.erase(iter->second);
  }

  this->tokens_.erase(iter);
}

int VoteDispatcher::Dispatch(int32_t type, const ProtoMessage *vote) {
  VoteCallbackListHashmap::iterator iter = this->cbs_.find(type);
  if (iter == this->cbs_.end()) {
    return -1;
  }

  VoteCallbackList &cbs = iter->second;

  int ret_code = 0;

  for (VoteCallbackListIter iter2 = cbs.begin(); iter2 != cbs.end();) {
    VoteCallbackListIter iter3 = iter2;
    ++iter3;

    if (*iter2) {
      ret_code = (*iter2)(vote);
    }

    if (ret_code < 0) {
      return ret_code;
    }

    iter2 = iter3;
  }

  return ret_code;
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
