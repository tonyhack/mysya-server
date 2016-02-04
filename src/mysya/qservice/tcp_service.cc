#include <mysya/qservice/tcp_service.h>

namespace mysya {
namespace qservice {

TcpService::TcpService(const ::mysya::ioevent::SocketAddress &listen_addr,
    ::mysya::ioevent::EventLoop *app_event_loop, EventLoopThreadPool *thread_pool)
      : next_agent_(0) {
  this->listen_addr_ = listen_addr_;
  this->app_event_loop_ = app_event_loop;
  this->thread_pool_ = thread_pool;

  if (this->listen_socket_.Open() == false) {
    ::mysya::util::ThrowSystemErrorException(
        "TcpService::TcpService(): failed in TcpSocket::Open.");
  }

  if (this->BuildListenSocket(this->listen_socket_) == false) {
    this->listen_socket_.Close();
    ::mysya::util::ThrowSystemErrorException(
        "TcpService::TcpService(): failed in BuildListenSocket.");
  }

  typedef EventLoopThreadPool::EventLoopThreadVector EventLoopThreadVector;
  EventLoopThreadVector &threads = this->thread_pool_->GetThreads();
  for (EventLoopThreadVector::const_iterator iter = threads.begin();
      iter != threads.end(); ++iter) {
    TransportAgent *transport_agent =
      new (std::nothrow) TransportAgent(*iter, this->app_event_loop_);
    if (transport_agent == NULL) {
      ::mysya::util::ThrowSystemErrorException(
          "TcpService::TcpService(): failed in allocate TransportAgent.");
    }
    this->transport_agents_.push_back(transport_agent);
  }
}

TcpService::~TcpService() {
  for (TransportAgentVector::iterator iter = this->transport_agents_.begin();
      iter != this->transport_agents_.end(); ++iter) {
    delete *iter;
  }

  this->transport_agents_.clear();
}

bool TcpService::BuildListenSocket(const ::mysya::ioevent::SocketAddress &listen_addr) {
  if (this->listen_socket_->SetReuseAddr() == false) {
    MYSYA_ERROR("TcpSocket::SetReuseAddr() failed.");
    return false;
  }

  if (this->listen_socket_->SetTcpNoDelay() == false) {
    MYSYA_ERROR("TcpSocket::SetTcpNoDelay() failed.");
    return false;
  }

  if (this->listen_socket_->Bind(addr) == false) {
    MYSYA_ERROR("TcpSocket::Bind() failed.");
    return false;
  }

  if (this->listen_socket_->Listen(256) == false) {
    MYSYA_ERROR("TcpSocket::Listen() failed.");
    return false;
  }

  if (this->listen_socket_->SetNonblock() == false) {
    MYSYA_ERROR("TcpSocket::SetNonblock() failed.");
    return false;
  }

  ::mysya::ioevent::EventLoop *event_loop = thread_pool->Allocate();

  this->listen_socket_->GetEventChannel()->SetReadCallback(
      std::bind(&TcpService::OnListenRead, this, std::placeholders::_1));
  this->listen_socket_->GetEventChannel()->SetErrorCallback(
      std::bind(&TcpService::OnListenError, this, std::placeholders::_1));

  if (this->listen_socket_->GetEventChannel()->AttachEventLoop(event_loop) == false) {
    MYSYA_ERROR("EventChannel::AttachEventLoop() failed.");
    return false;
  }

  return true;
}

bool TcpService::BuildConnectedSocket(std::unique_ptr<::mysya::ioevent::TcpSocket> &socket) {
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

  transport_agent->PushPending(socket);

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
    std::unique_ptr<TcpSocket> tcp_socket(new (std::nothrow) TcpSocket());
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

    int sockfd = tcp_socket->GetFileDescriptor();

    if (this->BuildConnectedSocket(tcp_socket) == false) {
      MYSYA_WARNING("BuildConnectedSocket failed.");
      continue;
    }
  }
}

void TcpService::OnListenError(::mysya::ioevent::EventChannel *event_channel) {
}

}  // namespace qservice
}  // namespace mysya
