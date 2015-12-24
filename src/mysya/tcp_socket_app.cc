#include "mysya/tcp_socket_app.h"

#include <string.h>

#include "mysya/dynamic_buffer.h"
#include "mysya/logger.h"
#include "mysya/socket_address.h"
#include "mysya/tcp_socket.h"

namespace mysya {

class TcpSocketApp::TcpConnection {
 public:
  TcpConnection(TcpSocketApp *host, TcpSocket *socket,
      size_t init_size, size_t extend_size);
  ~TcpConnection();

  TcpSocket *GetSocket() { return this->tcp_socket_; }
  DynamicBuffer *GetReceiveBuffer() { return &this->receive_buffer_; }
  DynamicBuffer *GetSendBuffer() { return &this->send_buffer_; }

  int GetErrno() const { return this->errno_; }
  void SetErrno(int value) { this->errno_ = value; }

  TcpSocketApp *GetHost() const { return this->host_; }

  bool SendMessage(const char *data, size_t size);

 private:
  TcpSocketApp *host_;
  TcpSocket *tcp_socket_;
  DynamicBuffer receive_buffer_;
  DynamicBuffer send_buffer_;

  int errno_;
};

TcpSocketApp::TcpConnection::TcpConnection(TcpSocketApp *host,
    TcpSocket *socket, size_t init_size, size_t extend_size)
  : host_(host), tcp_socket_(socket),
    receive_buffer_(init_size, extend_size),
    send_buffer_(init_size, extend_size),
    errno_(0) {}

TcpSocketApp::TcpConnection::~TcpConnection() {}

bool TcpSocketApp::TcpConnection::SendMessage(const char *data, size_t size) {
  int remain_size = size;
  int send_size = 0;

  if (this->send_buffer_.ReadableBytes() <= 0) {
    send_size = this->tcp_socket_->Write(data, size);
    if (send_size >= 0) {
      remain_size -= send_size;
    } else if (errno != EAGAIN) {
      this->SetErrno(errno);
      return false;
    }
  }

  if (remain_size > 0) {
    this->send_buffer_.Append(data + send_size, remain_size);
  } else {
    const TcpSocketApp::SendCompleteCallback &cb =
      this->host_->GetSendCompleteCallback();
    if (cb) {
      cb(this->host_, this->tcp_socket_->GetFileDescriptor());
    }
  }

  return true;
}


TcpSocketApp::TcpSocketApp(EventLoop *event_loop)
  : event_loop_(event_loop), listen_backlog_(64),
    socket_buffer_init_size_(256), socket_buffer_extend_size_(64) {}

TcpSocketApp::~TcpSocketApp() {}

bool TcpSocketApp::GetPeerAddress(int sockfd, SocketAddress *addr) {
  TcpSocketHashmap::iterator iter = this->sockets_.find(sockfd);
  if (iter == this->sockets_.end()) {
    return false;
  }

  TcpSocket *socket = iter->second;

  return socket->GetPeerAddress(addr);
}

bool TcpSocketApp::GetLocalAddress(int sockfd, SocketAddress *addr) {
  TcpSocketHashmap::iterator iter = this->sockets_.find(sockfd);
  if (iter == this->sockets_.end()) {
    return false;
  }

  TcpSocket *socket = iter->second;

  return socket->GetLocalAddress(addr);
}

void TcpSocketApp::SetConnectionCallback(const ConnectionCallback &cb) {
  this->connection_cb_ = cb;
}

void TcpSocketApp::ResetConnectionCallback() {
  ConnectionCallback cb;
  this->connection_cb_.swap(cb);
}

const TcpSocketApp::ConnectionCallback &TcpSocketApp::GetConnectionCallback() const {
  return this->connection_cb_;
}

void TcpSocketApp::SetReceiveCallback(const ReceiveCallback &cb) {
  this->receive_cb_ = cb;
}

void TcpSocketApp::ResetReceiveCallback() {
  ReceiveCallback cb;
  this->receive_cb_.swap(cb);
}

const TcpSocketApp::ReceiveCallback &TcpSocketApp::GetReceiveCallback() const {
  return this->receive_cb_;
}

void TcpSocketApp::SetSendCompleteCallback(const SendCompleteCallback &cb) {
  this->send_complete_cb_ = cb;
}

void TcpSocketApp::ResetetSendCompleteCallback() {
  SendCompleteCallback cb;
  this->send_complete_cb_.swap(cb);
}

const TcpSocketApp::SendCompleteCallback &TcpSocketApp::GetSendCompleteCallback() const {
  return this->send_complete_cb_;
}

void TcpSocketApp::SetCloseCallback(const CloseCallback &cb) {
  this->close_cb_ = cb;
}

void TcpSocketApp::ResetCloseCallback() {
  CloseCallback cb;
  this->close_cb_.swap(cb);
}

const TcpSocketApp::CloseCallback &TcpSocketApp::GetCloseCallback() const {
  return this->close_cb_;
}

void TcpSocketApp::SetErrorCallback(const ErrorCallback &cb) {
  this->error_cb_ = cb;
}

void TcpSocketApp::ResetErrorCallback() {
  ErrorCallback cb;
  this->error_cb_.swap(cb);
}

const TcpSocketApp::ErrorCallback &TcpSocketApp::GetErrorCallback() const {
  return error_cb_;
}

bool TcpSocketApp::Listen(const SocketAddress &addr) {
  std::unique_ptr<TcpSocket> socket(new (std::nothrow) TcpSocket());
  if (socket.get() == NULL) {
    MYSYA_ERROR("Allocate TcpSocket failed.");
    return false;
  }

  if (socket->Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return -1;
  }

  if (this->BuildListenSocket(socket, addr) == false) {
    MYSYA_ERROR("TcpSocketApp::BuildListenSocket() failed.");
    socket->Close();
    return false;
  }

  return true;
}

void TcpSocketApp::Close(int sockfd) {
  TcpConnectionHashmap::iterator connection_iter = this->connections_.find(sockfd);
  if (connection_iter != this->connections_.end()) {
    delete connection_iter->second;
    this->connections_.erase(connection_iter);
  }

  TcpSocketHashmap::iterator socket_iter = this->sockets_.find(sockfd);
  if (socket_iter != this->sockets_.end()) {
    delete socket_iter->second;
    this->sockets_.erase(socket_iter);
  }
}

int TcpSocketApp::Connect(const SocketAddress &addr) {
  std::unique_ptr<TcpSocket> socket(new (std::nothrow) TcpSocket());
  if (socket.get() == NULL) {
    MYSYA_ERROR("Allocate TcpSocket failed.");
    return -1;
  }

  if (socket->Open() == false) {
    MYSYA_ERROR("TcpSocket::Open() failed.");
    return -1;
  }

  int sockfd = socket->GetFileDescriptor();

  if (socket->Connect(addr) == false) {
    MYSYA_ERROR("TcpSocket::Connect(%s:%d) failed.",
        addr.GetHost().data(), addr.GetPort());
    socket->Close();
    return -1;
  }

  if (this->BuildConnectedSocket(socket) == false) {
    MYSYA_ERROR("TcpSocketApp::BuildConnectedSocket() failed.");
    socket->Close();
    return -1;
  }

  return sockfd;
}

bool TcpSocketApp::AsyncConnect(const SocketAddress &addr, int timeout_ms) {
  std::unique_ptr<TcpSocket> socket(new (std::nothrow) TcpSocket());
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
  TcpConnectionHashmap::iterator iter =
    this->connections_.find(sockfd);
  if (iter == this->connections_.end()) {
    MYSYA_ERROR("connection_socket(%d) not found in connections.", sockfd);
    return false;
  }

  TcpConnection *connection = iter->second;
  if (connection->SendMessage(data, size) == false) {
    if (this->error_cb_) {
      this->error_cb_(this, sockfd, connection->GetErrno());
    }

    this->Close(sockfd);

    return false;
  }

  return true;
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

  if (socket->Bind(addr) == false) {
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
      std::bind(&TcpSocketApp::OnListenError, this, std::placeholders::_1));

  if (socket->GetEventChannel()->AttachEventLoop(this->event_loop_) == false) {
    MYSYA_ERROR("EventChannel::AttachEventLoop() failed.");
    return false;
  }

  this->sockets_.insert(std::make_pair(sockfd, socket.get()));
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

  std::unique_ptr<TcpConnection> connection(
      new (std::nothrow) TcpConnection(this, socket.get(),
        this->GetSocketBufferInitSize(), this->GetSocketBufferExtendSize()));
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

bool TcpSocketApp::BuildAsyncConnectSocket(std::unique_ptr<TcpSocket> &socket,
    int timeout_ms) {
  int sockfd = socket->GetFileDescriptor();

  if (this->sockets_.find(sockfd) != this->sockets_.end()) {
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
      std::bind(&TcpSocketApp::OnConnectError, this, std::placeholders::_1));

  if (socket->GetEventChannel()->AttachEventLoop(this->event_loop_) == false) {
    MYSYA_ERROR("EventChannel::AttachEventLoop() failed.");
    return false;
  }

  std::unique_ptr<TcpConnection> connection(
      new (std::nothrow) TcpConnection(this, socket.get(),
        this->GetSocketBufferInitSize(), this->GetSocketBufferExtendSize()));
  if (connection.get() == NULL) {
    MYSYA_ERROR("Allocate TcpConnection failed.");
    return false;
  }

  this->sockets_.insert(std::make_pair(sockfd, socket.get()));
  this->connections_.insert(std::make_pair(sockfd, connection.get()));

  socket.release();
  connection.release();

  if (timeout_ms > 0) {
    this->AddSocketTimer(sockfd, timeout_ms,
        std::bind(&TcpSocketApp::OnConnectTimeout, this, std::placeholders::_1));
  }

  return true;
}

void TcpSocketApp::AddSocketTimer(int sockfd, int expire_ms, const ExpireCallback &cb) {
  int64_t timer_id = this->event_loop_->StartTimer(expire_ms, cb, 1);
  if (timer_id < 0) {
    MYSYA_ERROR("sockfd(%d) start timer(%d) failed.", sockfd, expire_ms);
    return;
  }

  this->timer_socket_ids_[timer_id] = sockfd;
  this->socket_timer_ids_[sockfd] = timer_id;
}

void TcpSocketApp::RemoveSocketTimer(int sockfd) {
  SocketTimerHashmap::iterator iter = this->socket_timer_ids_.find(sockfd);
  if (iter == this->socket_timer_ids_.end()) {
    MYSYA_ERROR("sockfd(%d) not found in socket_timer_ids_.");
    return;
  }

  int64_t timer_id = iter->second;
  this->socket_timer_ids_.erase(iter);
  this->timer_socket_ids_.erase(timer_id);

  this->event_loop_->StopTimer(timer_id);
}

void TcpSocketApp::OnListenRead(EventChannel *event_channel) {
  TcpSocket *listen_socket = (TcpSocket *)event_channel->GetAppHandle();
  int listen_sockfd = listen_socket->GetFileDescriptor();

  for (;;) {
    std::unique_ptr<TcpSocket> connection_socket(new (std::nothrow) TcpSocket());
    if (connection_socket.get() == NULL) {
      MYSYA_ERROR("Allocate TcpSocket failed.");
      return;
    }

    if (listen_socket->Accept(connection_socket.get()) == false) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      } else if (errno == ECONNABORTED || errno == EPROTO) {
        continue;
      } else {
        MYSYA_ERROR("listen_socket[%d]::Accept() failed, strerror(%s)",
            listen_sockfd, ::strerror(errno));
        return;
      }
    }

    int connection_sockfd = connection_socket->GetFileDescriptor();

    if (this->BuildConnectedSocket(connection_socket) == false) {
      MYSYA_WARNING("BuildConnectedSocket failed.");
      continue;
    }

    if (this->connection_cb_) {
      this->connection_cb_(this, connection_sockfd);
    }
  }
}

void TcpSocketApp::OnListenError(EventChannel *event_channel) {
  TcpSocket *listen_socket = (TcpSocket *)event_channel->GetAppHandle();
  int listen_sockfd = listen_socket->GetFileDescriptor();

  if (this->error_cb_) {
    this->error_cb_(this, listen_sockfd, errno);
  }

  MYSYA_ERROR("listen socket(%d) error.", listen_sockfd);

  this->Close(listen_sockfd);
}

void TcpSocketApp::OnConnectWrite(EventChannel *event_channel) {
  TcpSocket *connection_socket = (TcpSocket *)event_channel->GetAppHandle();
  int connection_sockfd = connection_socket->GetFileDescriptor();

  this->RemoveSocketTimer(connection_sockfd);

  TcpConnectionHashmap::iterator iter = this->connections_.find(connection_sockfd);
  if (iter == this->connections_.end()) {
    MYSYA_ERROR("connection_socket(%d) not found in connections.",
        connection_sockfd);
    return;
  }

  event_channel->SetReadCallback(
      std::bind(&TcpSocketApp::OnSocketRead, this, std::placeholders::_1));
  event_channel->SetErrorCallback(
      std::bind(&TcpSocketApp::OnSocketWrite, this, std::placeholders::_1));
  event_channel->SetErrorCallback(
      std::bind(&TcpSocketApp::OnSocketError, this, std::placeholders::_1));

  if (this->connection_cb_) {
    this->connection_cb_(this, connection_sockfd);
  }
}

void TcpSocketApp::OnConnectError(EventChannel *event_channel) {
  TcpSocket *connection_socket = (TcpSocket *)event_channel->GetAppHandle();
  int connection_sockfd = connection_socket->GetFileDescriptor();

  this->RemoveSocketTimer(connection_sockfd);

  TcpConnectionHashmap::iterator iter = this->connections_.find(connection_sockfd);
  if (iter == this->connections_.end()) {
    MYSYA_ERROR("connection_socket(%d) not found in connections.",
        connection_sockfd);
    return;
  }

  TcpConnection *connection = iter->second;
  connection->SetErrno(errno);

  if (this->error_cb_) {
    this->error_cb_(this, connection_sockfd, connection->GetErrno());
  }

  this->Close(connection_sockfd);
}

void TcpSocketApp::OnConnectTimeout(int64_t timer_id) {
  TimerSocketHashmap::iterator iter = this->timer_socket_ids_.find(timer_id);
  if (iter == this->timer_socket_ids_.end()) {
    MYSYA_ERROR("timer_id(%ld) not found in timer_socket_ids_.", timer_id);
    return;
  }

  int sockfd = iter->second;

  this->timer_socket_ids_.erase(iter);
  this->socket_timer_ids_.erase(sockfd);

  TcpConnectionHashmap::iterator connection_iter =
    this->connections_.find(sockfd);
  if (connection_iter == this->connections_.end()) {
    MYSYA_ERROR("connection_socket(%d) not found in connections.", sockfd);
    return;
  }

  TcpConnection *connection = connection_iter->second;
  connection->SetErrno(ETIMEDOUT);

  if (this->error_cb_) {
    this->error_cb_(this, sockfd, connection->GetErrno());
  }

  this->Close(sockfd);
}

void TcpSocketApp::OnSocketRead(EventChannel *event_channel) {
  TcpSocket *socket = (TcpSocket *)event_channel->GetAppHandle();
  int sockfd = socket->GetFileDescriptor();

  TcpConnectionHashmap::iterator connection_iter =
    this->connections_.find(sockfd);
  if (connection_iter == this->connections_.end()) {
    MYSYA_ERROR("connection_socket(%d) not found in connections.", sockfd);
    return;
  }

  TcpConnection *connection = connection_iter->second;
  DynamicBuffer *receive_buffer = connection->GetReceiveBuffer();

  bool received = false;
  bool peer_closed = false;

  for (;;) {
    int readable_size = socket->ReadableSize();
    if (readable_size <= 0) {
      peer_closed = true;
      break;
    }

    receive_buffer->ReserveWritableBytes(readable_size);

    int receive_bytes = socket->Read(receive_buffer->WriteBegin(),
        receive_buffer->WritableBytes());
    if (receive_bytes > 0) {
      receive_buffer->WrittenBytes(receive_bytes);
      received = true;
    } else if (receive_bytes == 0) {
      peer_closed = true;
    } else if (errno != EAGAIN) {
      connection->SetErrno(errno);
      if (this->error_cb_) {
        this->error_cb_(this, sockfd, connection->GetErrno());
      }

      this->Close(sockfd);
      return;
    }

    break;
  }

  if (received == true && this->receive_cb_) {
    this->receive_cb_(this, sockfd, receive_buffer);
  }
  if (peer_closed == true && this->close_cb_) {
    this->close_cb_(this, sockfd);
  }
}

void TcpSocketApp::OnSocketWrite(EventChannel *event_channel) {
  TcpSocket *socket = (TcpSocket *)event_channel->GetAppHandle();
  int sockfd = socket->GetFileDescriptor();

  TcpConnectionHashmap::iterator connection_iter =
    this->connections_.find(sockfd);
  if (connection_iter == this->connections_.end()) {
    MYSYA_ERROR("connection_socket(%d) not found in connections.", sockfd);
    return;
  }

  TcpConnection *connection = connection_iter->second;
  DynamicBuffer *send_buffer = connection->GetSendBuffer();

  int send_bytes = send_buffer->ReadableBytes();
  if (send_bytes < 0) {
    return;
  }

  int send_size = socket->Write(send_buffer->ReadBegin(), send_bytes);
  if (send_size < 0) {
    connection->SetErrno(errno);
    if (this->error_cb_) {
      this->error_cb_(this, sockfd, connection->GetErrno());
    }
    this->Close(sockfd);
  } else {
    send_buffer->ReadBytes(send_size);
    if (send_buffer->ReadableBytes() == 0 && this->send_complete_cb_) {
      this->send_complete_cb_(this, sockfd);
    }
  }
}

void TcpSocketApp::OnSocketError(EventChannel *event_channel) {
  TcpSocket *socket = (TcpSocket *)event_channel->GetAppHandle();
  int sockfd = socket->GetFileDescriptor();

  TcpConnectionHashmap::iterator connection_iter =
    this->connections_.find(sockfd);
  if (connection_iter == this->connections_.end()) {
    MYSYA_ERROR("connection_socket(%d) not found in connections.", sockfd);
    return;
  }

  TcpConnection *connection = connection_iter->second;
  connection->SetErrno(errno);

  if (this->error_cb_) {
    this->error_cb_(this, sockfd, connection->GetErrno());
  }

  this->Close(sockfd);
}

}  // namespace mysya
