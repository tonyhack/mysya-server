#include <stdio.h>

#include <mysya/event_channel.h>
#include <mysya/event_loop.h>
#include <mysya/logger.h>
#include <mysya/tcp_socket.h>

namespace mysya {
namespace test {

void ConnectTestFunc() {
  SocketAddress peer_addr("127.0.0.1", 22);

  TcpSocket socket;
  if (socket.Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return;
  }

  if (socket.Connect(peer_addr) == 0) {
    MYSYA_ERROR("TcpSocket::Connect() failed.");
    return;
  }

  MYSYA_INFO("Connect host(%s:%d) success!",
      peer_addr.GetHost().data(), peer_addr.GetPort());
}

static EventLoop g_event_loop;

void OnAsyncConnectCallback(void *channel) {
  EventChannel *event_channel = (EventChannel *)channel;
  TcpSocket *tcp_socket = (TcpSocket *)event_channel->GetAppHandle();

  SocketAddress peer_addr;
  if (tcp_socket->GetPeerAddress(&peer_addr) == false) {
    MYSYA_ERROR("SocketAddress::GetPeerAddress() failed.)");
    return;
  }

  MYSYA_INFO("Async connec host(%s:%d) success!",
      peer_addr.GetHost().data(), peer_addr.GetPort());

  g_event_loop.Quit();
}

void AsyncConnectTestFunc() {
  SocketAddress peer_addr("127.0.0.1", 22);

  TcpSocket socket;
  if (socket.Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return;
  }

  if (socket.AsyncConnect(peer_addr) == 0) {
    MYSYA_ERROR("TcpSocket::Connect() failed.");
    return;
  }

  EventChannel *event_channel = socket.GetEventChannel();

  if (event_channel->AttachEventLoop(&g_event_loop) == false) {
    MYSYA_ERROR("EventChannel::AttachEventLoop() failed.");
    return;
  }

  event_channel->SetReadCallback(OnAsyncConnectCallback);

  g_event_loop.Loop();
}

}  // namespace test
}  // namespace mysya

int main(int argc, char *argv[]) {

  ::mysya::test::ConnectTestFunc();
  ::mysya::test::AsyncConnectTestFunc();

  return 0;
}
