#include "tutorial/orcas/gateway/robot/robot_app.h"

#include <unistd.h>

#include <google/protobuf/message.h>
#include <mysya/ioevent/event_channel.h>
#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/gateway/robot/actor.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace robot {

MYSYA_SINGLETON_IMPL(RobotApp);

RobotApp::RobotApp()
  : socket_app_(&loop_),
    codec_(&socket_app_) {
  this->channel_.SetFileDescriptor(0);
  this->channel_.SetReadCallback(
      std::bind(&RobotApp::OnReadCallback, this, std::placeholders::_1));
  this->channel_.AttachEventLoop(&this->loop_);

  this->socket_app_.SetConnectionCallback(
      std::bind(&RobotApp::OnConnected, this, std::placeholders::_1,
        std::placeholders::_2));
  this->socket_app_.SetReceiveCallback(
      std::bind(&RobotApp::OnReceive, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  this->socket_app_.SetSendCompleteCallback(
      std::bind(&RobotApp::OnSendCompleted, this, std::placeholders::_1,
        std::placeholders::_2));
  this->socket_app_.SetCloseCallback(
      std::bind(&RobotApp::OnClose, this, std::placeholders::_1,
        std::placeholders::_2));
  this->socket_app_.SetErrorCallback(
      std::bind(&RobotApp::OnError, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));

  this->codec_.SetMessageCallback(
      std::bind(&RobotApp::OnMessage, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
        std::placeholders::_5));
}

RobotApp::~RobotApp() {
  this->socket_app_.ResetConnectionCallback();
  this->socket_app_.ResetReceiveCallback();
  this->socket_app_.ResetSendCompleteCallback();
  this->socket_app_.ResetCloseCallback();
  this->socket_app_.ResetErrorCallback();

  this->codec_.ResetMessageCallback();

  this->channel_.DetachEventLoop();
}

void RobotApp::Start() {
  this->message_handler_.SetHandlers();
  this->PrintPrompt();
  this->loop_.Loop();
}

bool RobotApp::Connect(const ::mysya::ioevent::SocketAddress &server_addr,
    int timeout_ms) {
  return this->socket_app_.AsyncConnect(server_addr, timeout_ms) != -1;
}

void RobotApp::CloseSocket(int sockfd) {
  this->socket_app_.Close(sockfd);
}

void RobotApp::PrintPrompt() {
  ::printf("\033[;36m");
  ::printf("robot> ");
  ::printf("\033[0m");
  ::fflush(stdout);
}

void RobotApp::OnReadCallback(::mysya::ioevent::EventChannel *channel) {
  char buffer[2048];
  char *ret = fgets(buffer, sizeof(buffer), stdin);
  if (ret == NULL) {
    this->loop_.Quit();
    return;
  }

  char *new_line = ::strstr(buffer, "\n");
  if (new_line == NULL) {
    this->loop_.Quit();
    return;
  }

  std::string user_command(buffer, new_line - buffer);

  if (user_command == "q" ||
      user_command == "quit" ||
      user_command == "exit") {
    this->loop_.Quit();
    return;
  } else if (user_command == "daemon") {
    this->channel_.DetachEventLoop();
    ::daemon(1, 0);
  } else {
    if (user_command_handler_.Parse(user_command) == false) {
      ::printf("invalid command\n");
    }
  }

  PrintPrompt();
}

void RobotApp::AddActor(Actor *actor) {
  this->actors_.insert(std::make_pair(actor->GetId(), actor));
}

Actor *RobotApp::GetActor(int id) {
  Actor *actor = NULL;

  ActorHashmap::iterator iter = this->actors_.find(id);
  if (iter != this->actors_.end()) {
    actor = iter->second;
  }

  return actor;
}

Actor *RobotApp::RemoveActor(int id) {
  Actor *actor = NULL;

  ActorHashmap::iterator iter = this->actors_.find(id);
  if (iter != this->actors_.end()) {
    actor = iter->second;
    this->actors_.erase(iter);
  }

  return actor;
}

void RobotApp::SendMessage(int sockfd, int type, ::google::protobuf::Message &data) {
  std::string buffer;
  if (data.SerializeToString(&buffer) == false) {
    MYSYA_ERROR("Message::SerializeAsString() failed.");
    return;
  }
  this->codec_.SendMessage(sockfd, type, buffer.data(), buffer.size());
  MYSYA_DEBUG("SendMessage(%d, %d, %lu)", sockfd, type, buffer.size());
}

void RobotApp::SetMessageCalback(int type, const MessageCallback &cb) {
  this->message_cbs_[type] = cb;
}

void RobotApp::ResetMessageCallback(int type) {
  this->message_cbs_.erase(type);
}

void RobotApp::Dispatch(Actor *actor, int type, const char *data, int size) {
  MessageCallbackHashmap::iterator iter = this->message_cbs_.find(type);
  if (iter == this->message_cbs_.end()) {
    return;
  }

  iter->second(actor, data, size);
}

void RobotApp::OnConnected(::mysya::ioevent::TcpSocketApp *tcp_socket_app, int sockfd) {
  Actor *actor = new (std::nothrow) Actor(sockfd);
  if (actor == NULL) {
    MYSYA_ERROR("Allocate Actor(%d) failed.", sockfd);
    tcp_socket_app->Close(sockfd);
    return;
  }

  this->AddActor(actor);
}

void RobotApp::OnConnectError(::mysya::ioevent::TcpSocketApp *tcp_socket_app,
    int sockfd, int sys_errno) {
  MYSYA_ERROR("RobotApp::OnConnectError sockfd(%d) sys_errno(%d).",
      sockfd, sys_errno);
}

void RobotApp::OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd,
    ::mysya::ioevent::DynamicBuffer *buffer) {
  this->codec_.OnMessage(sockfd, buffer);
}

void RobotApp::OnSendCompleted(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
}

void RobotApp::OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
  Actor *actor = this->RemoveActor(sockfd);
  if (actor == NULL) {
    MYSYA_ERROR("RobotApp::RemoveActor(%d) failed.");
    return;
  }

  delete actor;
}

void RobotApp::OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code) {
  Actor *actor = this->RemoveActor(sockfd);
  if (actor == NULL) {
    MYSYA_ERROR("RobotApp::RemoveActor(%d) failed.");
    return;
  }

  delete actor;
}

void RobotApp::OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *socket_app,
    int message_type, const char *data, int size) {
  Actor *actor = this->GetActor(sockfd);
  if (actor == NULL) {
    MYSYA_ERROR("RobotApp::GetActor(%d) failed.", sockfd);
    return;
  }

  this->Dispatch(actor, message_type, data, size);
}

}  // namespace robot
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
