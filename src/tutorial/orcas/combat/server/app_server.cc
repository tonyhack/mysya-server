#include "tutorial/orcas/combat/server/app_server.h"

#include <google/protobuf/message.h>
#include <mysya/codec/protobuf_codec.h>
#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/socket_address.h>

#include "tutorial/orcas/combat/message_dispatcher.h"
#include "tutorial/orcas/combat/server/combat_message_handler.h"
#include "tutorial/orcas/combat/server/app_session.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

AppServer::AppServer(::mysya::ioevent::EventLoop *event_loop,
      int listen_backlog)
  : listen_backlog_(listen_backlog),
    event_loop_(event_loop),
    tcp_socket_app_(event_loop_),
    combat_message_handler_(this),
    user_combat_message_handler_(this),
    combat_event_handler_(this),
    apps_(this) {
  this->combat_message_handler_.SetMessageHandlers();
  this->user_combat_message_handler_.SetMessageHandlers();
  this->combat_event_handler_.SetHandlers();

  this->tcp_socket_app_.SetConnectionCallback(
      std::bind(&AppServer::OnConnected, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_socket_app_.SetReceiveCallback(
      std::bind(&AppServer::OnReceive, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  this->tcp_socket_app_.SetSendCompleteCallback(
      std::bind(&AppServer::OnSendCompleted, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_socket_app_.SetCloseCallback(
      std::bind(&AppServer::OnClose, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_socket_app_.SetErrorCallback(
      std::bind(&AppServer::OnError, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));

  if (this->apps_.Initialize() == false) {
    ::mysya::util::ThrowSystemErrorException(
        "AppServer::Constructor(): Apps::Initialize failed.");
  }
}

AppServer::~AppServer() {
  this->apps_.Finalize();

  for (CodecHashmap::iterator iter = this->protobuf_codecs_.begin();
      iter != this->protobuf_codecs_.end(); ++iter) {
    iter->second->ResetMessageCallback();
    delete iter->second;
  }

  this->tcp_socket_app_.ResetConnectionCallback();
  this->tcp_socket_app_.ResetReceiveCallback();
  this->tcp_socket_app_.ResetSendCompleteCallback();
  this->tcp_socket_app_.ResetCloseCallback();
  this->tcp_socket_app_.ResetErrorCallback();

  this->combat_event_handler_.ResetHandlers();
  this->user_combat_message_handler_.ResetMessageHandlers();
  this->combat_message_handler_.ResetMessageHandlers();
}

bool AppServer::Listen(const ::mysya::ioevent::SocketAddress &addr) {
  return this->tcp_socket_app_.Listen(addr);
}

int64_t AppServer::StartTimer(int expire_ms, const ExpireCallback &cb,
    int call_times) {
  return this->event_loop_->StartTimer(expire_ms, cb, call_times);
}

void AppServer::StopTimer(int64_t timer_id) {
  this->event_loop_->StopTimer(timer_id);
}

::mysya::codec::ProtobufCodec *AppServer::GetProtobufCodec(
    const ::mysya::ioevent::TcpSocketApp *app) {
  ::mysya::codec::ProtobufCodec *codec = NULL;

  CodecHashmap::iterator iter = this->protobuf_codecs_.find(app);
  if (iter != this->protobuf_codecs_.end()) {
    codec = iter->second;
  }

  return codec;
}

AppSession *AppServer::GetSession(int sockfd) {
  AppSession *session = NULL;

  AppSessionHashMap::iterator iter = this->app_sessions_.find(sockfd);
  if (iter != this->app_sessions_.end()) {
    session = iter->second;
  }

  return session;
}

VoteDispatcher *AppServer::GetVoteDispatcher() {
  return &this->vote_dispatcher_;
}

EventDispatcher *AppServer::GetEventDispatcher() {
  return &this->event_dispatcher_;
}

RequireDispatcher *AppServer::GetRequireDispatcher() {
  return &this->require_dispatcher_;
}

MessageDispatcher *AppServer::GetMessageDispatcher() {
  return &this->message_dispatcher_;
}

UserMessageDispatcher *AppServer::GetUserMessageDispatcher() {
  return &this->user_message_dispatcher_;
}

const ::mysya::util::Timestamp &AppServer::GetTimestamp() const {
  return this->event_loop_->GetTimestamp();
}

AppSession *AppServer::RemoveSession(int sockfd) {
  AppSession *session = NULL;

  AppSessionHashMap::iterator iter = this->app_sessions_.find(sockfd);
  if (iter != this->app_sessions_.end()) {
    session = iter->second;
    this->app_sessions_.erase(iter);
  }

  return session;
}

bool AppServer::AddSession(AppSession *session) {
  AppSessionHashMap::iterator iter =
    this->app_sessions_.find(session->GetSockfd());
  if (iter != this->app_sessions_.end()) {
    return false;
  }

  this->app_sessions_.insert(std::make_pair(session->GetSockfd(), session));
  return true;
}

void AppServer::OnConnected(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
  ::mysya::ioevent::SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("[APP_SERVER] TcpSocketApp::GetPeerAddress() failed.");
    return;
  }

  do {
    if (this->GetProtobufCodec(app) != NULL) {
      break;
    }

    std::unique_ptr< ::mysya::codec::ProtobufCodec> codec(
        new (std::nothrow) ::mysya::codec::ProtobufCodec(app));
    if (codec.get() == NULL) {
      MYSYA_DEBUG("[APP_SERVER] Allocate ProtobufCodec failed.");
      return;
    }

    codec->SetMessageCallback(std::bind(&AppServer::OnMessage, this,
          std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    this->protobuf_codecs_.insert(std::make_pair(app, codec.get()));

    codec.release();
  } while (false);

  std::unique_ptr<AppSession> session(
      new (std::nothrow) AppSession(sockfd, app, this));
  if (session.get() == NULL) {
    MYSYA_DEBUG("[APP_SERVER] Allocate AppSession failed.");
    return;
  }

  if (this->AddSession(session.get()) == false) {
    MYSYA_DEBUG("[APP_SERVER] Duplicate AppSession(%p) failed.", session.get());
    return;
  }

  session.release();

  MYSYA_DEBUG("[APP_SERVER] New connection(%s:%d) sockfd(%d).",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd);
}

void AppServer::OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd,
    ::mysya::ioevent::DynamicBuffer *buffer) {
  ::mysya::codec::ProtobufCodec *codec = this->GetProtobufCodec(app);
  if (codec == NULL) {
    MYSYA_ERROR("[APP_SERVER] GetProtobufCodec(%p) failed.", app);
    return;
  }

  int read_bytes = codec->OnMessage(sockfd, buffer);
  if (read_bytes < 0) {
    MYSYA_ERROR("[APP_SERVER] ProtobufCodec::OnMessage failed.");
    return;
  }

  MYSYA_DEBUG("[APP_SERVER] decode message bytes(%d).", read_bytes);
}

void AppServer::OnSendCompleted(::mysya::ioevent::TcpSocketApp *app, int sockfd) {}

void AppServer::OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
  AppSession *session = this->RemoveSession(sockfd);
  if (session == NULL) {
    MYSYA_ERROR("[APP_SERVER] RemoveSession(%d) failed.", sockfd);
    return;
  }

  delete session;

  MYSYA_DEBUG("[APP_SERVER] sockfd(%d) closed.", sockfd);
}

void AppServer::OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code) {
  AppSession *session = this->RemoveSession(sockfd);
  if (session == NULL) {
    MYSYA_ERROR("[APP_SERVER] RemoveSession(%d) failed.", sockfd);
    return;
  }

  delete session;

  MYSYA_DEBUG("[APP_SERVER] sockfd(%d) error.", sockfd);
}

void AppServer::OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *app,
    const ::google::protobuf::Message *message) {
  AppSession *session = this->GetSession(sockfd);
  if (session == NULL) {
    MYSYA_ERROR("[APP_SERVER] GetSession(%d) failed.", sockfd);
    return;
  }

  this->message_dispatcher_.Dispatch(session, message);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
