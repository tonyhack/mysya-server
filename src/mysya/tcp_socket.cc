#include <mysya/tcp_socket.h>

namespace mysya {

TcpSocket::TcpSocket() {}
TcpSocket::~TcpSocket() {}

bool TcpSocket::Open() {
  if (this->GetFileDescriptor() != -1) {
    this->Close();
  }

  int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    MYSYA_ERROR("::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) failed.")
    return false;
  }

  this->SetFileDescriptor(fd);

  if (this->event_channel_.SetCloseExec() == false) {
    this->Close();
  }

  return true;
}

void TcpSocket::Close() {
  int fd = this->GetFileDescriptor();
  if (fd == -1) {
    return;
  }

  this->DetachEventLoop();

  ::close(fd);
  this->SetFileDescriptor(-1);
}

bool TcpSocket::Connect(const SocketAddress &peer_addr) {
  int fd = this->GetFileDescriptor();

  if (::connect(fd, peer_addr.GetNativeHandle(),
        peer_addr.GetNativeHandleSize()) != 0 &&
      errno != EINPROGRESS && error != EISCONN) {
    this->Close();
    MYSYA_ERROR("::connect failed.");
    return false;
  }

  return true;
}

bool TcpSocket::AsyncConnect(const SocketAddress &peer_addr) {
  int fd = this->GetFileDescriptor();

  if (this->event_channel_->SetNonblock() == false ||
      this->SetReuseAddr() == false ||
      this->SetTcpNoDelay() == false) {
    MYSYA_ERROR("Nonblock|ReuseAddr|TcpNoDelay failed.");
    return false;
  }

  if (::connect(fd, peer_addr.GetNativeHandle(),
        peer_addr.GetNativeHandleSize()) != 0) {
    MYSYA_ERROR("::connect failed.");
    return false;
  }

  return true;
}

bool TcpSocket::Bind(const SocketAddress &addr) {
}

bool TcpSocket::Listen(int backlog) {
}

int TcpSocket::Accept(TcpSocket *peer_socket) {
}

int TcpSocket::ReadableSize() const {
}

int TcpSocket::Read(char *data, size_t size) {
}

int TcpSocket::Write(const char *data, size_t size) {
}

bool TcpSocket::SetReuseAddr() {
}

bool TcpSocket::SetTcpNoDelay() {
}

}  // namespace mysya
