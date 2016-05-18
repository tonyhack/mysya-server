#include "tutorial/orcas/combat/client/combat_sessions.h"

#include <memory>

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/socket_address.h>
#include <mysya/ioevent/tcp_socket_app.h>

#include "tutorial/orcas/combat/client/combat_session.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace client {

CombatSessions::CombatSessions(::mysya::ioevent::TcpSocketApp *tcp_socket_app)
  : tcp_socket_app_(tcp_socket_app), codec_(tcp_socket_app_) {
  this->tcp_socket_app_->SetConnectionCallback(
      std::bind(&CombatSessions::OnConnected, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_socket_app_->SetReceiveCallback(
      std::bind(&CombatSessions::OnReceive, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  this->tcp_socket_app_->SetSendCompleteCallback(
      std::bind(&CombatSessions::OnSendCompleted, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_socket_app_->SetCloseCallback(
      std::bind(&CombatSessions::OnClose, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_socket_app_->SetErrorCallback(
      std::bind(&CombatSessions::OnError, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));

  this->codec_.SetMessageCallback(std::bind(&CombatSessions::OnMessage, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

CombatSessions::~CombatSessions() {
  this->codec_.ResetMessageCallback();

  this->tcp_socket_app_->ResetReceiveCallback();
  this->tcp_socket_app_->ResetSendCompleteCallback();
  this->tcp_socket_app_->ResetCloseCallback();
  this->tcp_socket_app_->ResetErrorCallback();

  for (SessionMap::iterator iter = this->sessions_.begin();
      iter != this->sessions_.end(); ++iter) {
    CombatSession *session = iter->second;
    delete session;
  }

  this->sessions_.clear();
  this->connections_.clear();
}

bool CombatSessions::AsyncConnect(int32_t server_id,
    const ::mysya::ioevent::SocketAddress &addr, int timeout_ms) {
  if (this->GetConnection(server_id) != -1) {
    MYSYA_ERROR("[COMBAT_SESSION] connect duplicate server_id(%d).", server_id);
    return false;
  }

  int sockfd = this->tcp_socket_app_->AsyncConnect(addr, timeout_ms);
  if (sockfd == -1) {
    MYSYA_ERROR("[COMBAT_SESSION] AsyncConnect failed.");
    return false;
  }

  this->connections_.insert(std::make_pair(server_id, sockfd));

  return true;
}

CombatSession *CombatSessions::GetSessionByServerId(int32_t server_id) {
  int sockfd = this->GetConnection(server_id);
  if (sockfd == -1) {
    return NULL;
  }

  return this->GetSession(sockfd);
}

::mysya::codec::ProtobufCodec *CombatSessions::GetCodec() {
  return &this->codec_;
}

MessageDispatcher *CombatSessions::GetMessageDispatcher() {
  return &this->message_dispatcher_;
}

void CombatSessions::AddConnection(int32_t server_id, int sockfd) {
  this->connections_.insert(std::make_pair(server_id, sockfd));
}

int CombatSessions::GetConnection(int32_t server_id) {
  int sockfd = -1;

  ConnectionMap::const_iterator iter = this->connections_.find(server_id);
  if (iter != this->connections_.end()) {
    sockfd = iter->second;
  }

  return sockfd;
}

void CombatSessions::RemoveConnection(int32_t server_id) {
  this->connections_.erase(server_id);
}

void CombatSessions::AddSession(int sockfd, CombatSession *session) {
  this->sessions_.insert(std::make_pair(sockfd, session));
}

CombatSession *CombatSessions::GetSession(int sockfd) {
  CombatSession *session = NULL;

  SessionMap::const_iterator iter = this->sessions_.find(sockfd);
  if (iter != this->sessions_.end()) {
    session = iter->second;
  }

  return session;
}

void CombatSessions::RemoveSession(int sockfd) {
  this->sessions_.erase(sockfd);
}

void CombatSessions::OnConnected(::mysya::ioevent::TcpSocketApp *tcp_socket_app, int sockfd) {
  int32_t server_id = -1;

  for (ConnectionMap::const_iterator iter = this->connections_.begin();
      iter != this->connections_.end(); ++iter) {
    if (iter->second == sockfd) {
      server_id = iter->first;
      break;
    }
  }

  if (server_id == -1) {
    MYSYA_ERROR("[COMBAT_SESSION] sockfd(%d) missed serverid.", sockfd);
    this->tcp_socket_app_->Close(sockfd);
    return;
  }

  std::unique_ptr<CombatSession> session(
      new (std::nothrow) CombatSession(sockfd, server_id, tcp_socket_app, this));
  if (session.get() == NULL) {
    MYSYA_ERROR("[COMBAT_SESSION] Allocate CombatSession(%d) failed.", sockfd);
    this->RemoveConnection(server_id);
    this->tcp_socket_app_->Close(sockfd);
    return;
  }

  this->AddSession(sockfd, session.get());

  session.release();

  MYSYA_DEBUG("[COMBAT_SESSION] sockfd(%d) connected.", sockfd);
}

void CombatSessions::OnConnectError(::mysya::ioevent::TcpSocketApp *tcp_socket_app,
    int sockfd, int sys_errno) {
  CombatSession *session = this->GetSession(sockfd);
  if (session != NULL) {
    this->RemoveConnection(session->GetServerId());
    this->RemoveSession(sockfd);
    delete session;
  } else {
    for (ConnectionMap::iterator iter = this->connections_.begin();
        iter != this->connections_.end(); ++iter) {
      if (iter->second == sockfd) {
        this->connections_.erase(iter);
        break;
      }
    }
  }

  MYSYA_DEBUG("[COMBAT_SESSION] sockfd(%d) connect error(%d)", sockfd, sys_errno);
}

void CombatSessions::OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd,
    ::mysya::ioevent::DynamicBuffer *buffer) {
  int read_bytes = this->codec_.OnMessage(sockfd, buffer);
  if (read_bytes < 0) {
    MYSYA_ERROR("[COMBAT_SESSION] ProtobufCodec::OnMessage failed.");
    return;
  }

  MYSYA_DEBUG("[COMBAT_SESSION] decode message bytes(%d).", read_bytes);
}

void CombatSessions::OnSendCompleted(::mysya::ioevent::TcpSocketApp *app, int sockfd) {}

void CombatSessions::OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
  CombatSession *session = this->GetSession(sockfd);
  if (session == NULL) {
    return;
  }

  this->RemoveConnection(session->GetServerId());
  this->RemoveSession(sockfd);

  delete session;

  MYSYA_DEBUG("[COMBAT_SESSION] sockfd(%d) close.", sockfd);
}

void CombatSessions::OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code) {
  CombatSession *session = this->GetSession(sockfd);
  if (session == NULL) {
    return;
  }

  this->RemoveConnection(session->GetServerId());
  this->RemoveSession(sockfd);

  delete session;

  MYSYA_DEBUG("[COMBAT_SESSION] sockfd(%d) error(%d).", sockfd, error_code);
}

void CombatSessions::OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *app,
    const ::google::protobuf::Message *message) {
  CombatSession *session = this->GetSession(sockfd);
  if (session == NULL) {
    MYSYA_ERROR("[COMBAT_SESSION] GetSession(%d) failed.", sockfd); 
    return;
  }

  this->message_dispatcher_.Dispatch(session, message);
}

}  // namespace client
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
