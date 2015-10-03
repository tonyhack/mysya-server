#include <mysya/tcp_socket.h>

#include <errno.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>

#include <mysya/logger.h>

namespace mysya {

TcpSocket::TcpSocket() : event_channel_(this) {}
TcpSocket::~TcpSocket() {}

bool TcpSocket::Open() {
  if (this->GetFileDescriptor() != -1) {
    this->Close();
  }

  int fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1) {
    MYSYA_ERROR("::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) errno(%d).",
        errno);
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

  this->event_channel_.DetachEventLoop();

  ::close(fd);
  this->SetFileDescriptor(-1);
}

bool TcpSocket::Connect(const SocketAddress &peer_addr) {
  int fd = this->GetFileDescriptor();

  if (::connect(fd, (const struct sockaddr *)peer_addr.GetNativeHandle(),
        peer_addr.GetNativeHandleSize()) != 0) {
    MYSYA_ERROR("::connect errno(%d).", errno);
    return false;
  }

  return true;
}

bool TcpSocket::AsyncConnect(const SocketAddress &peer_addr) {
  int fd = this->GetFileDescriptor();

  if (this->event_channel_.SetNonblock() == false ||
      this->SetReuseAddr() == false ||
      this->SetTcpNoDelay() == false) {
    MYSYA_ERROR("Nonblock|ReuseAddr|TcpNoDelay failed.");
    return false;
  }

  if (::connect(fd, (const struct sockaddr *)peer_addr.GetNativeHandle(),
        peer_addr.GetNativeHandleSize()) != 0 &&
      errno != EINPROGRESS && errno != EISCONN) {
    MYSYA_ERROR("::connect errno(%d).", errno);
    return false;
  }

  return true;
}

bool TcpSocket::Bind(const SocketAddress &addr) {
  int fd = this->GetFileDescriptor();

  if (::bind(fd, (const struct sockaddr *)addr.GetNativeHandle(),
        addr.GetNativeHandleSize()) != 0) {
    MYSYA_ERROR("::bind addr(%s:%d) errno(%d).",
        addr.GetHost().data(), addr.GetPort(), errno);
    return false;
  }

  return true;
}

bool TcpSocket::Listen(int backlog) {
  int fd = this->GetFileDescriptor();

  if (::listen(fd, backlog) != 0) {
    MYSYA_ERROR("::listen errno(%d).", errno);
    return false;
  }

  return true;
}

bool TcpSocket::Accept(TcpSocket *peer_socket) {
  int fd = this->GetFileDescriptor();

  int sockfd = ::accept(fd, NULL, NULL);
  if (sockfd == -1) {
    MYSYA_ERROR("::accept errno(%d).", errno);
    return false;
  }

  peer_socket->SetFileDescriptor(sockfd);
  if (peer_socket->GetEventChannel()->SetCloseExec() == false) {
    peer_socket->Close();
    MYSYA_ERROR("TcpSocket::SetCloseExec() errno(%d).", errno);
    return false;
  }

  return true;
}

int TcpSocket::ReadableSize() const {
  int fd = this->GetFileDescriptor();
  int readable_size = 0;

  if (::ioctl(fd, FIONREAD, &readable_size) == -1) {
    MYSYA_ERROR("::ioctl errno(%d)", errno);
    return -1;
  }

  return readable_size;
}

int TcpSocket::Read(char *data, size_t size) {
  int fd = this->GetFileDescriptor();

  return ::recv(fd, data, size, MSG_NOSIGNAL);
}

int TcpSocket::Write(const char *data, size_t size) {
  int fd = this->GetFileDescriptor();

  return ::send(fd, data, size, MSG_NOSIGNAL);
}

bool TcpSocket::SetReuseAddr() {
  int opt = 1;
  int fd = this->GetFileDescriptor();

  if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
    MYSYA_ERROR("::setsockopt SOL_SOCKET SO_REUSEADDR errno(%d)", errno);
    return false;
  }

  return true;
}

bool TcpSocket::SetTcpNoDelay() {
  int opt = 1;
  int fd = this->GetFileDescriptor();

  if (::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) != 0) {
    MYSYA_ERROR("::setsockopt IPPROTO_TCP TCP_NODELAY errno(%d)", errno);
    return false;
  }

  return true;
}

bool TcpSocket::SetNonblock() {
  return this->event_channel_.SetNonblock();
}

bool TcpSocket::SetCloseExec() {
  return this->event_channel_.SetCloseExec();
}

bool TcpSocket::GetLocalAddress(SocketAddress *addr) const {
  int fd = this->GetFileDescriptor();

  struct sockaddr_in sock_addr;
  socklen_t addr_len = sizeof(sock_addr);
  if (::getsockname(fd, (struct sockaddr *)&sock_addr, &addr_len) != 0) {
    MYSYA_ERROR("::getsockname errno(%d)", errno);
    return false;
  }

  addr->SetNativeHandle(sock_addr);

  return true;
}

bool TcpSocket::GetPeerAddress(SocketAddress *addr) const {
  int fd = this->GetFileDescriptor();

  struct sockaddr_in sock_addr;
  socklen_t addr_len = sizeof(sock_addr);
  if (::getpeername(fd, (struct sockaddr *)&sock_addr, &addr_len) != 0) {
    MYSYA_ERROR("::getpeername errno(%d)", errno);
    return false;
  }

  addr->SetNativeHandle(sock_addr);

  return true;
}

}  // namespace mysya
