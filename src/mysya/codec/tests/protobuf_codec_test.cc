#include <stdio.h>
#include <string.h>

#include <google/protobuf/message.h>

#include <mysya/codec/protobuf_codec.h>
#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/socket_address.h>
#include <mysya/ioevent/tcp_socket_app.h>

namespace mysya {
namespace codec {
namespace tests {

class TcpServer {
 public:
  TcpServer() : socket_app_(&event_loop_), codec_(&socket_app_) {
    ::mysya::ioevent::SocketAddress sock_addr("0.0.0.0", 9999);

    this->socket_app_.SetConnectionCallback(
        std::bind(&TcpServer::OnConnection, this, std::placeholders::_1,
          std::placeholders::_2));
    this->socket_app_.SetReceiveCallback(
        std::bind(&TcpServer::OnReceive, this, std::placeholders::_1,
          std::placeholders::_2, std::placeholders::_3));
    this->socket_app_.SetSendCompleteCallback(
        std::bind(&TcpServer::OnSendComplete, this, std::placeholders::_1,
          std::placeholders::_2));
    this->socket_app_.SetCloseCallback(
        std::bind(&TcpServer::OnClose, this, std::placeholders::_1,
          std::placeholders::_2));
    this->socket_app_.SetErrorCallback(
        std::bind(&TcpServer::OnError, this, std::placeholders::_1,
          std::placeholders::_2, std::placeholders::_3));
    this->codec_.SetMessageCallback(
        std::bind(&TcpServer::OnMessage, this, std::placeholders::_1,
          std::placeholders::_2, std::placeholders::_3));

    if (this->socket_app_.Listen(sock_addr) == false) {
      MYSYA_ERROR("TcpSocketApp::Listen() failed.");
      exit(0);
    }

    this->event_loop_.Loop();
  }

  ~TcpServer() {}

 private:
  void OnConnection(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd, ::mysya::ioevent::DynamicBuffer *buffer);
  void OnSendComplete(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd);
  void OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code);
  void OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *app, const ::google::protobuf::Message *message);

  ::mysya::ioevent::EventLoop event_loop_;
  ::mysya::ioevent::TcpSocketApp socket_app_;
  ProtobufCodec codec_;
};

void TcpServer::OnConnection(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
  ::mysya::ioevent::SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("New connection(%s:%d) sockfd(%d)!",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd);
}

void TcpServer::OnReceive(::mysya::ioevent::TcpSocketApp *app, int sockfd, ::mysya::ioevent::DynamicBuffer *buffer) {
  ::mysya::ioevent::SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  bool shutdown_server = false;
  bool quit = false;

  for (;;) {
    int readable_bytes = buffer->ReadableBytes();
    if (readable_bytes <= 0) {
      break;
    }

    char *data = buffer->ReadBegin();

    for (size_t i = 0; i < (size_t)readable_bytes; ++i) {
      if (data[i] == '\n' || data[i] == '\r') {
        data[i] = '\0';
        break;
      }
    }

    if (strcmp(data, "shutdown") == 0) {
      shutdown_server = true;
    } else if (strcmp(data, "quit") == 0) {
      quit = true;
    }

    MYSYA_INFO("Receive from client(%s:%d) socket(%d) content[%s].",
        peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd, data);

    buffer->ReadBytes(readable_bytes);
  }

  if (shutdown_server == true) {
    app->Close(sockfd);
    this->event_loop_.Quit();
  } else if (quit == true) {
    app->Close(sockfd);
  }
}

void TcpServer::OnSendComplete(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
  ::mysya::ioevent::SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("Connection(%s:d) sockfd(%d) send complete.",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd);
}

void TcpServer::OnClose(::mysya::ioevent::TcpSocketApp *app, int sockfd) {
  ::mysya::ioevent::SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("Connection(%s:d) sockfd(%d) peer closed.",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd);
}

void TcpServer::OnError(::mysya::ioevent::TcpSocketApp *app, int sockfd, int error_code) {
  ::mysya::ioevent::SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("Connection(%s:d) sockfd(%d) error(%s).",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd,
      ::strerror(error_code));
}

void TcpServer::OnMessage(int sockfd, ::mysya::ioevent::TcpSocketApp *app,
    const ::google::protobuf::Message *message) {
  MYSYA_INFO("Message(%s) DebugString(%s)",
      message->GetTypeName().data(), message->DebugString().data());
}

void TestFunc() {
  TcpServer server;
}

}  // namespace tests
}  // namespace codec
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::codec::tests::TestFunc();

  return 0;
}
