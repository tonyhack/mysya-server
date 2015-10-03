#include "mysya/tcp_socket_app.h"

namespace mysya {

class TcpConnection {
 public:
  TcpConnection(TcpSocket *socket, size_t init_size, size_t extend_size);
  ~TcpConnection();

  TcpSocket *GetSocket() { return this->tcp_socket_; }
  DynamicBuffer *GetReceiveBuffer() { return &this->receive_buffer_; }
  DynamicBuffer *GetSendBuffer() { return &this->send_buffer_; }

 private:
  TcpSocket *tcp_socket_;
  DynamicBuffer receive_buffer_;
  DynamicBuffer send_buffer_;
}:

TcpConnection::TcpConnection(TcpSocket *socket, size_t init_size,
    size_t extend_size)
  : tcp_socket_(socket), receive_buffer_(init_size, extend_size),
    send_buffer_(init_size, extend_size) {}

TcpConnection::~TcpConnection() {}


TcpSocketApp::TcpSocketApp(EventLoop *event_loop)
  : event_loop_(NULL), listen_backlog_(64),
    socket_buffer_extend_size_(256), socket_buffer_extend_size_(64) {}

TcpSocketApp::~TcpSocketApp() {}

void TcpSocketApp::SetConnectionCallback(const ConnectionCallback &cb) {
  this->connection_cb_ = cb;
}

void TcpSocketApp::ResetConnectionCallback() {
  ConnectionCallback cb;
  this->connection_cb_.swap(cb;
}

void TcpSocketApp::SetReceiveCallback(const ReceiveCallback &cb) {
  this->receive_cb_ = cb;
}

void TcpSocketApp::ResetReceiveCallback() {
  ReceiveCallback cb;
  this->receive_cb_.swap(cb);
}

void TcpSocketApp::SetSendCompleteCallback(const SendCompleteCallback &cb) {
  this->send_complete_cb_ = cb;
}

void TcpSocketApp::ResetetSendCompleteCallback() {
  SendCompleteCallback cb;
  this->send_complete_cb_.swap(cb);
}

void TcpSocketApp::SetCloseCallback(const CloseCallback &cb) {
  this->close_cb_ = cb;
}

void TcpSocketApp::ResetCloseCallback() {
  CloseCallback cb;
  this->close_cb_.swap(cb);
}

void TcpSocketApp::SetErrorCallback(const ErrorCallback &cb) {
  this->error_cb_ = cb;
}

void TcpSocketApp::ResetErrorCallback() {
  ErrorCallback cb;
  this->error_cb_.swap(cb);
}

bool TcpSocketApp::Listen(const SocketAddress &addr) {
  std::unique_ptr<TcpSocket> socket = new (std::nothrow) TcpSocket();
  if (socket.get() == NULL) {
    MYSYA_ERROR("Allocate TcpSocket failed.");
    return false;
  }

  if (socket->Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return -1;
  }

  if (this->BuildListenSocket(socket, attr) == false) {
    MYSYA_ERROR("TcpSocketApp::BuildListenSocket() failed.");
    socket->Close();
    return false;
  }

  return true;
}

void TcpSocketApp::Close(int sockfd) {
  TcpConnectionHashmap::iterater connection_iter = this->connections_.find(sockfd);
  if (connection_iter != this->connections_.end()) {
    delete iter->second;
    this->connections_.erase(connection_iter);
  }

  TcpSocketHashmap::iterater socket_iter = this->sockets_.find(sockfd);
  if (socket_iter != this->sockets_.end()) {
    delete iter->second;
    this->sockets_.erase(socket_iter);
  }
}

bool TcpSocketApp::Connect(const SocketAddress &addr) {
  std::unique_ptr<TcpSocket> socket = new (std::nothrow) TcpSocket();
  if (socket.get() == NULL) {
    MYSYA_ERROR("Allocate TcpSocket failed.");
    return false;
  }

  if (socket->Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return false;
  }

  if (socket->Connect(addr) == false) {
    MYSYA_ERROR("TcpSocket::Connect(%s:%d) failed.",
        addr.GetHost().data(), addr.GetPort());
    socket->Close();
    return false;
  }

  if (this->BuildConnectedSocket(socket) == false) {
    MYSYA_ERROR("TcpSocketApp::BuildConnectedSocket() failed.");
    socket->Close();
    return false;
  }

  return true;
}

bool TcpSocketApp::AsyncConnect(const SocketAddress &addr, int timeout_ms) {
  std::unique_ptr<TcpSocket> socket = new (std::nothrow) TcpSocket();
  if (socket.get() == NULL) {
    MYSYA_ERROR("Allocate TcpSocket failed.");
    return false;
  }

  if (socket->Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return false;
  }

  if (socket->AsyncConnect(addr) == false) {
    MYSYA_ERROR("TcpSocket::AsyncConnect(%s:%d) failed.",
        addr.GetHost().data(), addr.GetPort());
    socket->Close();
    return false;
  }

  if (this->BuildAsyncConnectSocket(socket, timeout_ms) == false) {
    MYSYA_ERROR("TcpSocketApp::BuildAsyncConnectSocket() failed.");
    socket->Close();
    return false;
  }

  return true;
}

bool TcpSocketApp::SendMessage(int sockfd, const char *data, size_t size) {
}

bool TcpSocketApp::BuildListenSocket(std::unique_ptr<TcpSocket> &socket, const SocketAddress &addr) {
  int sockfd = socket->GetFileDescriptor();

  if (this->sockets_.find(sockfd) != this->sockets_.end()) {
    MYSYA_ERROR("Duplicate sockfd(%d).", sockfd);
    // DetachEventLoop() will be invoked by TcpSocketApp::Close().
    // socket->GetEventChannel()->DetachEventLoop();
    return false;
  }

  if (socket->SetReuseAddr() == false) {
    MYSYA_ERROR("TcpSocket::SetReuseAddr() failed.");
    return false;
  }

  if (socket->SetTcpNoDelay() == false) {
    MYSYA_ERROR("TcpSocket::SetTcpNoDelay() failed.");
    return false;
  }

  if (socket->Bind(attr) == false) {
    MYSYA_ERROR("TcpSocket::Bind() failed.");
    return false;
  }

  if (socket->Listen(this->GetListenBacklog()) == false) {
    MYSYA_ERROR("TcpSocket::Listen() failed.");
    return false;
  }

  if (socket->SetNonblock() == false) {
    MYSYA_ERROR("TcpSocket::SetNonblock() failed.");
    return false;
  }

  socket->GetEventChannel()->SetReadCallback(
      std::bind(&TcpSocketApp::OnListenRead, this, std::placeholders::_1));
  socket->GetEventChannel()->SetErrorCallback(
      std::bind(&TcpSocketApp::OnSocketError, this, std::placeholders::_1));

  if (socket->GetEventChannel()->AttachEventLoop(this->event_loop_) == false) {
    MYSYA_ERROR("EventChannel::AttachEventLoop() failed.");
    return false;
  }

  this->sockets_.insert(std::make_pair(sockfd, socket.get());
  socket.release();

  return true;
}

bool TcpSocketApp::BuildConnectedSocket(std::unique_ptr<TcpSocket> &socket) {
  int sockfd = socket->GetFileDescriptor();

  if (this->sockets_.find(sockfd) != this->sockets_.end()) {
    MYSYA_ERROR("Duplicate sockfd(%d).", sockfd);
    return false;
  }

  if (this->connections_.find(sockfd) != this->connections_.end()) {
    MYSYA_ERROR("Duplicate connection socket(%d).", sockfd);
    return false;
  }

  if (socket->SetReuseAddr() == false) {
    MYSYA_ERROR("TcpSocket::SetReuseAddr() failed.");
    return false;
  }

  if (socket->SetTcpNoDelay() == false) {
    MYSYA_ERROR("TcpSocket::SetTcpNoDelay() failed.");
    return false;
  }

  if (socket->SetNonblock() == false) {
    MYSYA_ERROR("TcpSocket::SetNonblock() failed.");
    return false;
  }

  socket->GetEventChannel()->SetReadCallback(
      std::bind(&TcpSocketApp::OnSocketRead, this, std::placeholders::_1));
  socket->GetEventChannel()->SetWriteCallback(
      std::bind(&TcpSocketApp::OnSocketWrite, this, std::placeholders::_1));
  socket->GetEventChannel()->SetErrorCallback(
      std::bind(&TcpSocketApp::OnSocketError, this, std::placeholders::_1));

  if (socket->GetEventChannel()->AttachEventLoop(this->event_loop_) == false) {
    MYSYA_ERROR("EventChannel::AttachEventLoop() failed.");
    return false;
  }

  std::unique_ptr<TcpConnection> connection = new (std::nothrow) TcpConnection(
      socket.get(), this->GetSocketBufferInitSize(), this->GetSocketBufferExtendSize());
  if (connection.get() == NULL) {
    MYSYA_ERROR("Allocate TcpConnection failed.");
    return false;
  }

  this->sockets_.insert(std::make_pair(sockfd, socket.get()));
  this->connections_.insert(std::make_pair(sockfd, connection.get()));

  socket.release();
  connection.release();

  return true;
}

bool TcpSocketApp::BuildAsyncConnectSocket(std::unique_ptr<TcpSocket> &socket, int timeout_ms) {
  int sockfd = socket->GetFileDescriptor();

  if (this->sockets_.find(socket) != this->sockets_.end()) {
    MYSYA_ERROR("Duplicate sockfd(%d).", sockfd);
    return false;
  }

  if (this->connections_.find(sockfd) != this->connections_.end()) {
    MYSYA_ERROR("Duplicate connection socket(%d).", sockfd);
    return false;
  }

  socket->GetEventChannel()->SetWriteCallback(
      std::bind(&TcpSocketApp::OnConnectWrite, this, std::placeholders::_1));
  socket->GetEventChannel()->SetErrorCallback(
      std::bind(&TcpSocketApp::OnSocketError, this, std::placeholders::_1));

  if (socket->GetEventChannel()->AttachEventLoop(this->event_loop_) == false) {
    MYSYA_ERROR("EventChannel::AttachEventLoop() failed.");
    return false;
  }

  std::unique_ptr<TcpConnection> connection = new (std::nothrow) TcpConnection(
      socket.get(), this->GetSocketBufferInitSize(), this->GetSocketBufferExtendSize());
  if (connection.get() == NULL) {
    MYSYA_ERROR("Allocate TcpConnection failed.");
    return false;
  }

  this->sockets_.insert(std::make_pair(sockfd, socket.get()));
  this->connections_.insert(std::make_pair(sockfd, connection.get()));

  socket.release();
  connection.release();

  if (timeout_ms > 0) {
    // TODO: add timeout connecting timer.
  }

  return true;
}

void TcpSocketApp::OnListenRead(EventChannel *event_channel) {
  TcpSocket *socket = (TcpSocket *)event_channel->GetAppHandle();
}

void TcpSocketApp::OnConnectWrite(EventChannel *event_channel) {
}

void TcpSocketApp::OnSocketRead(EventChannel *event_channel) {
}

void TcpSocketApp::OnSocketWrite(EventChannel *event_channel) {
}

void TcpSocketApp::OnSocketError(EventChannel *event_channel) {
}

}  // namespace mysya
