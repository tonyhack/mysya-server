#include <stdio.h>
#include <string.h>

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/socket_address.h>
#include <mysya/ioevent/tcp_socket_app.h>

namespace mysya {
namespace ioevent {
namespace test {

static EventLoop g_event_loop;

namespace socket_connect {

void TestFunc() {
  SocketAddress peer_addr("127.0.0.1", 22);
  TcpSocketApp app(&g_event_loop);

  int sockfd = app.Connect(peer_addr);
  if (sockfd < 0) {
    MYSYA_ERROR("TcpSocketApp::Connect() failed.");
    return;
  }

  MYSYA_INFO("Connect host(%s:%d) success!",
      peer_addr.GetHost().data(), peer_addr.GetPort());

  app.Close(sockfd);
}

}  // namespace socket_connect

namespace socket_async_connect {

void OnConnection(TcpSocketApp *app, int sockfd) {
  SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("Async connect host(%s:d) success, sockfd(%d)!",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd);

  app->Close(sockfd);

  g_event_loop.Quit();
}

void OnClose(TcpSocketApp *app, int sockfd) {
  SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("Async connect host(%s:d) closed, sockfd(%d)!",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd);

  g_event_loop.Quit();
}

void OnError(TcpSocketApp *app, int sockfd, int error_code) {
  SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("Async connect host(%s:%d) strerror(%s), sockfd(%d)!",
      peer_addr.GetHost().data(), peer_addr.GetPort(),
      ::strerror(error_code), sockfd);

  g_event_loop.Quit();
}

void TestFunc() {
  SocketAddress peer_addr("127.0.0.1", 22);

  TcpSocketApp app(&g_event_loop);
  app.SetConnectionCallback(OnConnection);
  app.SetCloseCallback(OnClose);
  app.SetErrorCallback(OnError);

  if (app.AsyncConnect(peer_addr, 4000) < 0) {
    MYSYA_ERROR("TcpSocketApp::AsyncConnect() failed.");
    return;
  }

  g_event_loop.Loop();
}

}  // namespace socket_async_connect

namespace socket_server {

void OnConnection(TcpSocketApp *app, int sockfd) {
  SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("New connection(%s:%d) sockfd(%d)!",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd);

  const char *kNodifyConnectedStr = "This is a socket server test from mysya-server: connection has been established.\n";
  app->SendMessage(sockfd, kNodifyConnectedStr, strlen(kNodifyConnectedStr));
}

void OnReceive(TcpSocketApp *app, int sockfd, DynamicBuffer *buffer) {
  SocketAddress peer_addr;
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

    MYSYA_INFO("Receive from client(%s:%d) socket(%d) content_size[%s].",
        peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd, data);

    buffer->ReadBytes(readable_bytes);
  }

  if (shutdown_server == true) {
    app->Close(sockfd);
    g_event_loop.Quit();
  } else if (quit == true) {
    app->Close(sockfd);
  }
}

void OnSendComplete(TcpSocketApp *app, int sockfd) {
  SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("Connection(%s:d) sockfd(%d) send complete.",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd);
}

void OnClose(TcpSocketApp *app, int sockfd) {
  SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("Connection(%s:d) sockfd(%d) peer closed.",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd);
}

void OnError(TcpSocketApp *app, int sockfd, int error_code) {
  SocketAddress peer_addr;
  if (app->GetPeerAddress(sockfd, &peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.");
    return;
  }

  MYSYA_INFO("Connection(%s:d) sockfd(%d) error(%s).",
      peer_addr.GetHost().data(), peer_addr.GetPort(), sockfd,
      ::strerror(error_code));
}

void TestFunc() {
  SocketAddress sock_addr("0.0.0.0", 9999);

  TcpSocketApp app(&g_event_loop);
  app.SetConnectionCallback(OnConnection);
  app.SetReceiveCallback(OnReceive);
  app.SetSendCompleteCallback(OnSendComplete);
  app.SetCloseCallback(OnClose);
  app.SetErrorCallback(OnError);

  if (app.Listen(sock_addr) == false) {
    MYSYA_ERROR("TcpSocketApp::Listen() failed.");
    return;
  }

  g_event_loop.Loop();
}

}  // namespace socket_server

}  // namespace test
}  // namespace ioevent
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::ioevent::test::socket_connect::TestFunc();
  ::mysya::ioevent::test::socket_async_connect::TestFunc();
  ::mysya::ioevent::test::socket_server::TestFunc();

  return 0;
}
