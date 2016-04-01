#include <mysya/qservice/tcp_service.h>

#include <memory>

#include <mysya/ioevent/event_channel.h>
#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/logger.h>
#include <mysya/qservice/errno.h>
#include <mysya/qservice/event_loop_thread_pool.h>
#include <mysya/qservice/transport_agent.h>

namespace mysya {
namespace qservice {

TcpService::TcpService(::mysya::ioevent::EventLoop *app_event_loop,
    EventLoopThreadPool *thread_pool) : next_agent_(0) {
  this->app_event_loop_ = app_event_loop;
  this->thread_pool_ = thread_pool;

  typedef EventLoopThreadPool::EventLoopThreadVector EventLoopThreadVector;
  EventLoopThreadVector &threads = this->thread_pool_->GetThreads();
  for (EventLoopThreadVector::iterator iter = threads.begin();
      iter != threads.end(); ++iter) {
    TransportAgent *transport_agent = new (std::nothrow) TransportAgent(
        this, &(*iter)->event_loop_, this->app_event_loop_);
    if (transport_agent == NULL) {
      ::mysya::util::ThrowSystemErrorException(
          "TcpService::TcpService(): failed in allocate TransportAgent.");
    }
    this->transport_agents_.push_back(transport_agent);
  }
}

TcpService::~TcpService() {
  for (TcpSocketMap::iterator iter = this->listen_sockets_.begin();
      iter != this->listen_sockets_.end(); ++iter) {
    delete iter->first;
  }

  for (TransportAgentVector::iterator iter = this->transport_agents_.begin();
      iter != this->transport_agents_.end(); ++iter) {
    delete *iter;
  }

  this->transport_agents_.clear();
}

int TcpService::AsyncConnect(const ::mysya::ioevent::SocketAddress &addr, int timeout_ms) {
  return this->AllocateTransportAgent()->AsyncConnect(addr, timeout_ms);
}

int TcpService::Listen(const ::mysya::ioevent::SocketAddress &addr) {
  return this->AllocateTransportAgent()->Listen(addr);
}

TcpService::ListenedCallback TcpService::GetListenedCallback() {
  return this->listened_cb_;
}

void TcpService::SetListenedCallback(const ListenedCallback &cb) {
  this->listened_cb_ = cb;
}

void TcpService::ResetListenedCallback() {
  ListenedCallback cb;
  this->listened_cb_.swap(cb);
}

TcpService::ListenErrorCallback TcpService::GetListenErrorCallback() {
  return this->listen_error_cb_;
}

void TcpService::SetListenErrorCallback(const ListenErrorCallback &cb) {
  this->listen_error_cb_ = cb;
}

void TcpService::ResetListenErrorCallback() {
  ListenErrorCallback cb;
  this->listen_error_cb_.swap(cb);
}

TcpService::AsyncConnectedCallback TcpService::GetAsyncConnectedCallback() const {
  return this->async_connected_cb_;
}

void TcpService::SetAsyncConnectedCallback(const AsyncConnectedCallback &cb) {
  this->async_connected_cb_ = cb;
}

void TcpService::ResetAsyncConnectedCallback() {
  AsyncConnectedCallback cb;
  this->async_connected_cb_.swap(cb);
}

TcpService::AsyncConnectErroCallback TcpService::GetAsyncConnectErroCallback() const {
  return this->async_connect_error_cb_;
}

void TcpService::SetAsyncConnectErroCallback(const AsyncConnectErroCallback &cb) {
  this->async_connect_error_cb_ = cb;
}

void TcpService::ResetAsyncConnectErroCallback() {
  AsyncConnectErroCallback cb;
  this->async_connect_error_cb_.swap(cb);
}

TcpService::ConnectCallback TcpService::GetConnectCallback() const {
  return this->connect_cb_;
}

void TcpService::SetConnectCallback(const ConnectCallback &cb) {
  this->connect_cb_ = cb;
}

void TcpService::ResetConnectCallback() {
  ConnectCallback cb;
  this->connect_cb_.swap(cb);
}

TcpService::ReceiveCallback TcpService::GetReceiveCallback() const {
  return this->receive_cb_;
}

void TcpService::SetReceiveCallback(const ReceiveCallback &cb) {
  this->receive_cb_ = cb;
}

void TcpService::ResetReceiveCallback() {
  ReceiveCallback cb;
  this->receive_cb_.swap(cb);
}

TcpService::CloseCallback TcpService::GetCloseCallback() const {
  return this->close_cb_;
}

void TcpService::SetCloseCallback(const CloseCallback &cb) {
  this->close_cb_ = cb;
}

void TcpService::ResetCloseCallback() {
  CloseCallback cb;
  this->close_cb_.swap(cb);
}

TcpService::ErrorCallback TcpService::GetErrorCallback() const {
  return this->error_cb_;
}

void TcpService::SetErrorCallback(const ErrorCallback &cb) {
  this->error_cb_ = cb;
}

void TcpService::ResetErrorCallback() {
  ErrorCallback cb;
  this->error_cb_.swap(cb);
}

TcpService::ReceiveDecodeCallback TcpService::GetReceiveDecodeCallback() const {
  return this->receive_decode_cb_;
}

void TcpService::SetReceiveDecodeCallback(const ReceiveDecodeCallback &cb) {
  this->receive_decode_cb_ = cb;
}

void TcpService::ResetReceiveDecodeCallback() {
  ReceiveDecodeCallback cb;
  this->receive_decode_cb_.swap(cb);
}

bool TcpService::BuildListenSocket(TransportAgent *transport_agent,
    ::mysya::ioevent::TcpSocket *listen_socket, const ::mysya::ioevent::SocketAddress &listen_addr,
    int backlog) {
  int listen_sockfd = listen_socket->GetFileDescriptor();

  TcpSocketMap::iterator iter = this->listen_sockets_.find(listen_socket);
  if (iter != this->listen_sockets_.end()) {
    errno = SocketErrno::DUPLICATE_SOCKET;
    MYSYA_ERROR("Duplicate listen sockfd(%d).", listen_sockfd);
    return false;
  }

  if (listen_socket->SetReuseAddr() == false) {
    MYSYA_ERROR("TcpSocket::SetReuseAddr() failed.");
    return false;
  }

  if (listen_socket->SetTcpNoDelay() == false) {
    MYSYA_ERROR("TcpSocket::SetTcpNoDelay() failed.");
    return false;
  }

  if (listen_socket->Bind(listen_addr) == false) {
    MYSYA_ERROR("TcpSocket::Bind() failed.");
    return false;
  }

  if (listen_socket->Listen(backlog) == false) {
    MYSYA_ERROR("TcpSocket::Listen() failed.");
    return false;
  }

  if (listen_socket->SetNonblock() == false) {
    MYSYA_ERROR("TcpSocket::SetNonblock() failed.");
    return false;
  }

  ::mysya::ioevent::EventLoop *event_loop = transport_agent->GetNetworkEventLoop();

  listen_socket->GetEventChannel()->SetReadCallback(
      std::bind(&TcpService::OnListenRead, this, std::placeholders::_1));
  listen_socket->GetEventChannel()->SetErrorCallback(
      std::bind(&TcpService::OnListenError, this, std::placeholders::_1));

  if (listen_socket->GetEventChannel()->AttachEventLoop(event_loop) == false) {
    MYSYA_ERROR("EventChannel::AttachEventLoop() failed.");
    return false;
  }

  this->listen_sockets_.insert(std::make_pair(listen_socket, transport_agent));

  // app listened callbcak.
  event_loop->PushWakeupCallback(
      std::bind(&TransportAgent::OnHandleListened, transport_agent, listen_sockfd));

  return true;
}

bool TcpService::BuildConnectedSocket(::mysya::ioevent::TcpSocket *socket) {
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

  TransportAgent *transport_agent = this->AllocateTransportAgent();
  if (transport_agent == NULL) {
    MYSYA_ERROR("AllocateTransportAgent() failed.");
    return false;
  }

  if (transport_agent->AddTcpSocket(socket) == false) {
    MYSYA_ERROR("TransportAgent::AddTcpSocket() failed.");
    return false;
  }

  return true;
}

TransportAgent *TcpService::AllocateTransportAgent() {
  return this->transport_agents_[(this->next_agent_++ % this->transport_agents_.size())];
}

void TcpService::OnListenRead(::mysya::ioevent::EventChannel *event_channel) {
  ::mysya::ioevent::TcpSocket *listen_socket =
    (::mysya::ioevent::TcpSocket *)event_channel->GetAppHandle();
  int listen_sockfd = listen_socket->GetFileDescriptor();

  for (;;) {
    std::unique_ptr< ::mysya::ioevent::TcpSocket> tcp_socket(
        new (std::nothrow) ::mysya::ioevent::TcpSocket());
    if (tcp_socket.get() == NULL) {
      MYSYA_ERROR("Allocate TcpSocket failed.");
      return;
    }

    if (listen_socket->Accept(tcp_socket.get()) == false) {
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

    if (this->BuildConnectedSocket(tcp_socket.get()) == false) {
      MYSYA_WARNING("BuildConnectedSocket failed.");
      continue;
    }

    tcp_socket.release();
  }
}

void TcpService::OnListenError(::mysya::ioevent::EventChannel *event_channel) {
  ::mysya::ioevent::TcpSocket *listen_socket =
    (::mysya::ioevent::TcpSocket *)event_channel->GetAppHandle();
  int listen_sockfd = listen_socket->GetFileDescriptor();

  TcpSocketMap::iterator iter = this->listen_sockets_.find(listen_socket);
  if (iter == this->listen_sockets_.end()) {
    MYSYA_ERROR("listen_sockets_ find sockfd(%d) failed.", listen_sockfd);
    return;
  }

  TransportAgent *transport_agent = iter->second;

  this->listen_sockets_.erase(iter);
  delete listen_socket;

  // app listened callbcak.
  transport_agent->GetAppEventLoop()->PushWakeupCallback(
      std::bind(&TransportAgent::OnHandleListened, transport_agent, listen_sockfd));
}

}  // namespace qservice
}  // namespace mysya
