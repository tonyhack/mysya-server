#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_MESSAGE_DISPATCHER_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_MESSAGE_DISPATCHER_H

#include <functional>
#include <unordered_map>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

class Actor;

class MessageDispatcher {
 public:
  typedef std::function<void (Actor *, const char *, int)> MessageCallback;
  typedef std::unordered_map<int, MessageCallback> MessageCallbackHashmap;

  MessageDispatcher();
  ~MessageDispatcher();

  void SetMessageCallback(int type, const MessageCallback &cb);
  void ResetMessageCallback(int type);

  void Dispatch(Actor *actor, int type, const char *data, int size);

 private:
  MessageCallbackHashmap message_cbs_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(MessageDispatcher);
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_MESSAGE_DISPATCHER_H
