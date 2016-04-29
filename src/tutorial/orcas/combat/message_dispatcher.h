#ifndef TUTORIAL_ORCAS_COMBAT_MESSAGE_DISPATCHER_H
#define TUTORIAL_ORCAS_COMBAT_MESSAGE_DISPATCHER_H

#include <functional>
#include <string>
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

class TransportChannel;

class MessageDispatcher {
 public:
  typedef std::function<void (TransportChannel *,
      const ::google::protobuf::Message *)> MessageCallback;
  typedef std::unordered_map<std::string, MessageCallback> MessageCallbackHashmap;

  MessageDispatcher();
  ~MessageDispatcher();

  void SetMessageCalback(const std::string &type_name, const MessageCallback &cb);
  void ResetMessageCallback(const std::string &type_name);

  void Dispatch(TransportChannel *transport_channel,
      const ::google::protobuf::Message *message);

 private:
  MessageCallbackHashmap message_cbs_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(MessageDispatcher);
};

}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_MESSAGE_DISPATCHER_H
