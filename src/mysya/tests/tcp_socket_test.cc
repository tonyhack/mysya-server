#include <stdio.h>

#include <mysya/event_channel.h>
#include <mysya/tcp_socket.h>

namespace mysya {
namespace test {

void ConnectTestFunc() {
  SocketAddress peer_addr("127.0.0.1", 22);

  TcpSocket socket;
  if (socket.Open() == false) {
    ::printf("TcpSocket::Open() failed.\n");
    return;
  }

  if (socket.Connect(peer_addr) == 0) {
    ::printf("TcpSocket::Connect() failed.\n");
    return;
  }

  ::printf("Connect host(%s:%d) success!\n",
      peer_addr.GetHost().data(), peer_addr.GetPort());
}

}  // namespace test
}  // namespace mysya

int main(int argc, char *argv[]) {

  ::mysya::test::ConnectTestFunc();

  return 0;
}
