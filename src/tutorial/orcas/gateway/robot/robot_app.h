#ifndef TUTORIAL_ORCAS_GATEWAY_ROBOT_ROBOT_APP_H
#define TUTORIAL_ORCAS_GATEWAY_ROBOT_ROBOT_APP_H

#include <functional>
#include <unordered_map>

#include <mysya/ioevent/event_channel.h>
#include <mysya/ioevent/tcp_socket_app.h>
#include <mysya/util/class_util.h>

#include "tutorial/orcas/gateway/codec.h"
#include "tutorial/orcas/gateway/robot/message_handler.h"
#include "tutorial/orcas/gateway/robot/user_command_handler.h"

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace mysya {
namespace ioevent {

class EventLoop;
class SocketAddress;

}  // namespace ioevent
}  // namespace mysya

namespace tutorial {
namespace orcas {
namespace gateway {
namespace robot {

class Actor;

class RobotApp {
 public:
  typedef std::function<void (Actor *, const char *, int)> MessageCallback;
  typedef std::unordered_map<int, MessageCallback> MessageCallbackHashmap;

  typedef std::unordered_map<int, Actor *> ActorHashmap;

  void Start();

  bool Connect(const ::mysya::ioevent::SocketAddress &server_addr,
      int timeout_ms);
  void CloseSocket(int sockfd);

  void AddActor(Actor *actor);
  Actor *GetActor(int id);
  Actor *RemoveActor(int id);

  void SetMessageCallback(int type, const MessageCallback &cb);
  void ResetMessageCallback(int type);
  void Dispatch(Actor *actor, int type, const char *data, int size);

  void SendMessage(int sockfd, int type, ::google::protobuf::Message &data);

 private:
  void PrintPrompt();
  void OnReadCallback(::mysya::ioevent::EventChannel *channel);

  void OnConnected(::mysya::ioevent::TcpSocketApp *tcp_socket_app, int sockfd);
  void OnConnectError(::mysya::ioevent::TcpSocketApp *tcp_socket_app,
      int sockfd, int sys_errno);

  void OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd,
      ::mysya::ioevent::DynamicBuffer *buffer);
  void OnSendCompleted(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code);

  void OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *socket_app,
        int message_type, const char *data, int size);

  ::mysya::ioevent::EventLoop loop_;
  ::mysya::ioevent::EventChannel channel_;
  ::mysya::ioevent::TcpSocketApp socket_app_;

  ::tutorial::orcas::gateway::Codec codec_;

  MessageCallbackHashmap message_cbs_;

  ActorHashmap actors_;
  UserCommandHandler user_command_handler_;
  MessageHandler message_handler_;

  MYSYA_SINGLETON(RobotApp);
};

}  // namespace robot
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_ROBOT_ROBOT_APP_H
