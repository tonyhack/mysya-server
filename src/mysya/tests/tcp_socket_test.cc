#include <stdio.h>
#include <string.h>

#include <mysya/dynamic_buffer.h>
#include <mysya/event_channel.h>
#include <mysya/event_loop.h>
#include <mysya/logger.h>
#include <mysya/tcp_socket.h>

namespace mysya {
namespace test {

static EventLoop g_event_loop;

namespace socket_connect {

void TestFunc() {
  SocketAddress peer_addr("127.0.0.1", 22);

  TcpSocket socket;
  if (socket.Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return;
  }

  if (socket.Connect(peer_addr) == 0) {
    MYSYA_ERROR("TcpSocket::Connect() failed.");
    socket.Close();
    return;
  }

  MYSYA_INFO("Connect host(%s:%d) success!",
      peer_addr.GetHost().data(), peer_addr.GetPort());
}

}  // namespace socket_connect

namespace socket_async_connect {

void OnConnection(EventChannel *channel) {
  TcpSocket *tcp_socket = (TcpSocket *)channel->GetAppHandle();

  SocketAddress peer_addr;
  if (tcp_socket->GetPeerAddress(&peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.)");
    return;
  }

  MYSYA_INFO("Async connec host(%s:%d) success!",
      peer_addr.GetHost().data(), peer_addr.GetPort());

  g_event_loop.Quit();
}

void TestFunc() {
  SocketAddress peer_addr("127.0.0.1", 22);

  TcpSocket socket;
  if (socket.Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return;
  }

  if (socket.AsyncConnect(peer_addr) == 0) {
    MYSYA_ERROR("TcpSocket::Connect() failed.");
    socket.Close();
    return;
  }

  EventChannel *event_channel = socket.GetEventChannel();

  if (event_channel->AttachEventLoop(&g_event_loop) == false) {
    MYSYA_ERROR("EventChannel::AttachEventLoop() failed.");
    return;
  }

  event_channel->SetReadCallback(OnConnection);

  g_event_loop.Loop();
}

}  // namespace socket_async_connect

namespace socket_server {

void OnRead(EventChannel *channel) {
  MYSYA_INFO("OnRead.");

  TcpSocket *tcp_socket = (TcpSocket *)channel->GetAppHandle();

  bool quit = false;

  do {
    int readable_size = tcp_socket->ReadableSize();
    if (readable_size <= 0) {
      quit = true;
      break;
    }

    static DynamicBuffer buffer;
    buffer.ReserveWritableBytes(readable_size);

    do {
      int read_size = tcp_socket->Read(buffer.WriteBegin(), buffer.WritableBytes());
      if (read_size <= 0) {
        quit = true;
        break;
      }

      buffer.WrittenBytes(read_size);

      for (size_t i = 0; i < (size_t)read_size; ++i) {
        if (buffer.ReadBegin()[i] == '\n' || buffer.ReadBegin()[i] == '\r') {
          buffer.ReadBegin()[i] = '\0';
        }
      }

      if (strcmp(buffer.ReadBegin(), "quit") == 0) {
        quit = true;
      }

      MYSYA_INFO("Receive from client: %s", buffer.ReadBegin());

      buffer.ReadBytes(read_size);
    } while (false);

  } while (false);


  if (quit == true) {
    channel->DetachEventLoop();
    delete tcp_socket;
    g_event_loop.Quit();
  }
}

void OnWrite(EventChannel *channel) {
  MYSYA_INFO("OnWrite.");
}

void OnError(EventChannel *channel) {
  MYSYA_INFO("OnError.");
  channel->DetachEventLoop();

  TcpSocket *tcp_socket = (TcpSocket *)channel->GetAppHandle();
  delete tcp_socket;
}

void OnConnection(EventChannel *channel) {
  TcpSocket *tcp_socket = (TcpSocket *)channel->GetAppHandle();

  TcpSocket *connection_socket = new TcpSocket();
  if (tcp_socket->Accept(connection_socket) == false) {
    MYSYA_ERROR("TcpSocket::Accept() failed.");
    return;
  }

  EventChannel *connection_event_channel = connection_socket->GetEventChannel();
  connection_event_channel->SetNonblock();

  connection_event_channel->SetReadCallback(OnRead);
  connection_event_channel->SetErrorCallback(OnError);

  if (connection_event_channel->AttachEventLoop(&g_event_loop) == false) {
    MYSYA_ERROR("EventLoop::AttachEventLoop() failed.");
    return;
  }

  static const char g_default_send_str[] = "This is a message from server!\n";
  connection_socket->Write(g_default_send_str, strlen(g_default_send_str));
}

void OnServerWrite(EventChannel *channel) {
  MYSYA_INFO("OnServerWrite");
}

void TestFunc() {
  SocketAddress sock_addr("0.0.0.0", 9999);

  TcpSocket socket;
  if (socket.Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return;
  }

  if (socket.SetReuseAddr() == false) {
    MYSYA_ERROR("TcpSocket::SetReuseAddr() failed.");
    socket.Close();
    return;
  }

  if (socket.Bind(sock_addr) == false) {
    MYSYA_ERROR("TcpSocket::Bind addr(%s:%d) failed.",
        sock_addr.GetHost().data(), sock_addr.GetPort());
    socket.Close();
    return;
  }

  if (socket.Listen(1024) == false) {
    MYSYA_ERROR("TcpSocket::Listen addr(%s:%d) failed.",
        sock_addr.GetHost().data(), sock_addr.GetPort());
    socket.Close();
    return;
  }

  EventChannel *server_event_channel = socket.GetEventChannel();
  server_event_channel->SetNonblock();

  server_event_channel->SetReadCallback(OnConnection);

  if (server_event_channel->AttachEventLoop(&g_event_loop) == false) {
    MYSYA_ERROR("EventLoop::AttachEventLoop() failed.");
    socket.Close();
    return;
  }

  g_event_loop.Loop();
}

}  // namespace socket_server

}  // namespace test
}  // namespace mysya

int main(int argc, char *argv[]) {

  ::mysya::test::socket_connect::TestFunc();
  ::mysya::test::socket_async_connect::TestFunc();
  ::mysya::test::socket_server::TestFunc();

  return 0;
}
