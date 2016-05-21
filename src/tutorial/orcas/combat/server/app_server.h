#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_APP_SERVER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_APP_SERVER_H

#include <unordered_map>

#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/tcp_socket_app.h>
#include <mysya/util/class_util.h>

#include "tutorial/orcas/combat/message_dispatcher.h"
#include "tutorial/orcas/combat/server/apps.h"
#include "tutorial/orcas/combat/server/combat_message_handler.h"
#include "tutorial/orcas/combat/server/event_dispatcher.h"
#include "tutorial/orcas/combat/server/require_dispatcher.h"
#include "tutorial/orcas/combat/server/user_combat_message_handler.h"
#include "tutorial/orcas/combat/server/user_message_dispatcher.h"

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace mysya {
namespace codec {

class ProtobufCodec;

}  // namespace codec

namespace ioevent {

class DynamicBuffer;
class SocketAddress;

}  // namespace mysya
} // namespace ioevent

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppSession;

class AppServer {
 public:
  typedef std::unordered_map<const ::mysya::ioevent::TcpSocketApp *,
          ::mysya::codec::ProtobufCodec *> CodecHashmap;
  typedef std::unordered_map<int, AppSession *> AppSessionHashMap;
  typedef ::mysya::ioevent::EventLoop::ExpireCallback ExpireCallback;

  explicit AppServer(::mysya::ioevent::EventLoop *event_loop,
      int listen_backlog = 128);
  ~AppServer();

  bool Listen(const ::mysya::ioevent::SocketAddress &addr);

  int64_t StartTimer(int expire_ms, const ExpireCallback &cb,
      int call_times = -1);
  void StopTimer(int64_t timer_id);

  ::mysya::codec::ProtobufCodec *GetProtobufCodec(
      const ::mysya::ioevent::TcpSocketApp *app);
  AppSession *GetSession(int sockfd);

  EventDispatcher *GetEventDispatcher();
  RequireDispatcher *GetRequireDispatcher();
  MessageDispatcher *GetMessageDispatcher();
  UserMessageDispatcher *GetUserMessageDispatcher();

  const ::mysya::util::Timestamp &GetTimestamp() const;

 private:
  AppSession *RemoveSession(int sockfd);
  bool AddSession(AppSession *session);

  void OnConnected(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd,
      ::mysya::ioevent::DynamicBuffer *buffer);
  void OnSendCompleted(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code);

  void OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *app,
      const ::google::protobuf::Message *message);

  int listen_backlog_;

  ::mysya::ioevent::EventLoop *event_loop_;
  ::mysya::ioevent::TcpSocketApp tcp_socket_app_;

  CodecHashmap protobuf_codecs_;
  AppSessionHashMap app_sessions_;

  EventDispatcher event_dispatcher_;
  RequireDispatcher require_dispatcher_;
  MessageDispatcher message_dispatcher_;
  UserMessageDispatcher user_message_dispatcher_;
  CombatMessageHandler combat_message_handler_;
  UserCombatMessageHandler user_combat_message_handler_;

  Apps apps_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(AppServer);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_APP_SERVER_H
