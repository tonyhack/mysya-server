#include "tutorial/orcas/gateway/server/app_server.h"

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/socket_address.h>

#include "tutorial/orcas/gateway/server/actor.h"
#include "tutorial/orcas/gateway/server/config.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

AppServer::AppServer(::mysya::ioevent::EventLoop *event_loop,
    int listen_backlog)
  : listen_backlog_(listen_backlog),
    event_loop_(event_loop),
    tcp_socket_app_(event_loop_),
    combat_clients_(&tcp_socket_app_),
    codec_(&tcp_socket_app_) {
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

  this->codec_.SetMessageCallback(
      std::bind(&AppServer::OnMessage, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
        std::placeholders::_5));

  for (Config::CombatServerMap::iterator iter =
      Config::GetInstance()->combat_servers_.begin();
      iter != Config::GetInstance()->combat_servers_.end(); ++iter) {
    ::mysya::ioevent::SocketAddress server_addr(
        iter->second.host_, iter->second.port_);

    if (this->combat_clients_.AsyncConnect(iter->second.server_id_,
          server_addr, 10) == false) {
      MYSYA_ERROR("[APP_SERVER] AsyncConnect(%d, %s:%d) failed.",
          iter->second.server_id_, iter->second.host_.data(),
          iter->second.port_);
    }
  }
}

AppServer::~AppServer() {
  this->codec_.ResetMessageCallback();

  this->tcp_socket_app_.ResetConnectionCallback();
  this->tcp_socket_app_.ResetReceiveCallback();
  this->tcp_socket_app_.ResetSendCompleteCallback();
  this->tcp_socket_app_.ResetCloseCallback();
  this->tcp_socket_app_.ResetErrorCallback();

  for (ActorHashmap::iterator iter = this->actors_.begin();
      iter != this->actors_.end(); ++iter) {
    delete iter->second;
  }

  this->actors_.clear();
}

bool AppServer::Listen(const ::mysya::ioevent::SocketAddress &addr) {
  return this->tcp_socket_app_.Listen(addr);
}

int AppServer::SendMessage(int sockfd, int message_type,
    ::google::protobuf::Message *message) {
  std::string buffer;
  if (message->SerializeToString(&buffer) == false) {
    MYSYA_ERROR("[APP_SERVER] Message::SerializeAsString() failed.");
    return -1;
  }

  return this->codec_.SendMessage(sockfd, message_type, buffer.data(), buffer.size());
}

MessageDispatcher *AppServer::GetMessageDispatcher() {
  return &this->message_dispatcher_;
}

void AppServer::AddActor(Actor *actor) {
  this->actors_.insert(std::make_pair(actor->GetSockfd(), actor));
}

Actor *AppServer::RemoveActor(int sockfd) {
  Actor *actor = NULL;

  ActorHashmap::iterator iter = this->actors_.find(sockfd);
  if (iter != this->actors_.end()) {
    actor = iter->second;
    this->actors_.erase(iter);
  }

  return actor;
}

Actor *AppServer::GetActor(int sockfd) {
  Actor *actor = NULL;

  ActorHashmap::iterator iter = this->actors_.find(sockfd);
  if (iter != this->actors_.end()) {
    actor = iter->second;
  }

  return actor;
}

void AppServer::OnConnected(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
  std::unique_ptr<Actor> actor(new (std::nothrow) Actor(sockfd, app, this));
  if (actor.get() == NULL) {
    MYSYA_ERROR("[APP_SERVER] Allocate Actor(%d) failed.", sockfd);
    return;
  }

  this->AddActor(actor.get());

  actor.release();
}

void AppServer::OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd,
    ::mysya::ioevent::DynamicBuffer *buffer) {
}

void AppServer::OnSendCompleted(::mysya::ioevent::TcpSocketApp *app, int sockfd) {}

void AppServer::OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
  Actor *actor = this->RemoveActor(sockfd);
  if (actor == NULL) {
    MYSYA_ERROR("[APP_SERVER] RemoveActor(%d) failed.", sockfd);
    return;
  }

  delete actor;
}

void AppServer::OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code) {
  Actor *actor = this->RemoveActor(sockfd);
  if (actor == NULL) {
    MYSYA_ERROR("[APP_SERVER] RemoveActor(%d) failed.", sockfd);
    return;
  }

  delete actor;
}

void AppServer::OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *socket_app,
    int message_type, const char *data, int size) {
  Actor *actor = this->GetActor(sockfd);
  if (actor == NULL) {
    MYSYA_ERROR("[APP_SERVER] GetActor(%d) failed.", sockfd);
    return;
  }

  this->message_dispatcher_.Dispatch(actor, message_type, data, size);
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
