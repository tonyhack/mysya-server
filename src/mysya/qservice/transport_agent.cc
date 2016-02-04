#include <mysya/qservice/transport_agent.h>

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/tcp_socket.h>
#include <mysya/ioevent/event_loop.h>

namespace mysya {
namespace qservice {

class TransportAgent::TransportChannel {
 public:
  TransportChannel(TransportAgent *agent, ::mysya::ioevent::TcpSocket *tcp_socket,
      int init_size, int ext_size);
  ~TransportChannel();

  TransportAgent *GetHost() const { return this->host_; }
  int GetFileDescriptor() const { return this->tcp_socket_->GetFileDescriptor(); }

  ::mysya::ioevent::TcpSocket *GetTcpSocket() const { return this->tcp_socket_; }
  ::mysya::ioevent::DynamicBuffer *GetReadBuffer() const { return this->read_buffer_; }
  ::mysya::ioevent::DynamicBuffer *GetWriteBuffer() const { return this->write_buffer_; }

  void SetErrno(int value) { this->errno_ = value; }
  int GetErrno() const { return this->errno_; }

  bool SendMessage(const char *data, size_t size);

 private:
  TransportAgent *host_;
  ::mysya::ioevent::TcpSocket *tcp_socket_;

  ::mysya::ioevent::DynamicBuffer read_buffer_;
  ::mysya::ioevent::DynamicBuffer write_buffer_;

  int errno_;
};

TransportChannel::TransportChannel(TransportAgent *agent,
    ::mysya::ioevent::TcpSocket *tcp_socket, int init_size, int ext_size);
  : host_(host), tcp_socket_(tcp_socket),
    read_buffer_(init_size, ext_size),
    write_buffer_(init_size, ext_size),
    errno_(0) {}

TransportChannel::~TransportChannel() {}

bool TransportChannel::SendMessage(const char *data, size_t size) {
  int remain_size = size;
  int send_size = 0;

  if (this->write_buffer_.ReadableBytes() <= 0) {
    send_size = this->tcp_socket_->Write(data, size);
    if (send_size >= 0) {
      remain_size -= send_size;
    } else if (errno != EAGAIN) {
      this->SetErrno(errno);
      return false;
    }
  }

  if (remain_size > 0) {
    this->write_buffer_.Append(data + send_size, remain_size);
  } else {
    // TODO: send complete callback.
  }

  return true;
}

TransportAgent::TransportAgent(::mysya::ioevent::EventLoop *network_event_loop,
    ::mysya::ioevent::EventLoop *app_event_loop)
  : network_event_loop_(network_event_loop),
    app_event_loop_(app_event_loop),
    flush_receive_queue_timer_id_(0),
    flush_send_queue_timer_id_(0) {}

TransportAgent::~TransportAgent() {}

void TransportAgent::PushPending(::mysya::ioevent::TcpSocket *tcp_socket) {
  this->event_loop_->WakeupCallback(
      std::bind(&TransportAgent::OnHandlePending, this, tcp_socket));
}

bool TransportAgent::SendMessage(int sockfd, const char *data, size_t size) {
  LockGuard lock(this->send_queue_.GetWriteMutex());
  return this->send_queue_.Write(sockfd, data, size) != size;
}

void TransportAgent::FlushReceiveQueue() {
  int sockfd = -1;
  ::mysya::ioevent::DynamicBuffer buffer;

  LockGuard lock_read(this->receive_queue_.GetReadMutex());
  for (;;) {
    int read_size = this->receive_queue_.Read(buffer, sockfd);
    if (read_size <= 0) {
      break;
    }

    if (this->receive_cb_) {
      this->receive_cb_(sockfd, buffer.ReadBegin(), (size_t)read_size);
    }
    buffer.ReadBytes(read_size);
  }

  LockGuard lock_write(this->receive_queue_.GetWriteBuffer());
  this->receive_queue_.Exchange();
}

void TransportAgent::FlushSendQueue() {
  int sockfd = -1;
  ::mysya::ioevent::DynamicBuffer buffer;

  LockGuard lock_read(this->send_queue_.GetReadMutex());
  for (;;) {
    int read_size = this->send_queue_.Read(buffer, sockfd);
    if (read_size <= 0) {
      break;
    }

    TransportChannelHashmap::iterator iter = this->channels_.find(sockfd);
    if (iter == this->channels_.end()) {
      continue;
    }

    TransportChannel *channel = iter->second;
    channel->SendMessage(buffer.ReadBegin(), read_size);
    buffer.ReadBytes(read_size);
  }

  LockGuard lock_write(this->send_queue_.GetWriteBuffer());
  this->send_queue_.Exchange();
}

void TransportAgent::SetConnectCallback(const ConnectCallback &cb) {
}

void TransportAgent::ResetConnectCallback() {}

void TransportAgent::SetReceiveCallback(const ReceiveCallback &cb) {}

void TransportAgent::ResetReceiveCallback() {}

void TransportAgent::SetCloseCallback(const CloseCallback &cb) {}

void TransportAgent::ResetCloseCallback() {}

void TransportAgent::SetNextFlushReceiveQueueTimer() {
  this->app_event_loop_->StartTimer(10,
      std::bind(&TransportAgent::OnFlushReceiveQueue, this, std::placeholders::_1), 1);
}

void TransportAgent::SetNextFlushSendQueueTimer() {
  this->network_event_loop_->StartTimer(10,
      std::bind(&TransportAgent::OnFlushSendQueue, this, std::placeholders::_1), 1);
}

::mysya::ioevent::TcpSocket *TransportAgent::RemoveTcpSocket(int sockfd) {
  ::mysya::ioevent::TcpSocket *tcp_socket = NULL;

  TransportChannelHashmap::iterator iter = this->channels_.find(sockfd);
  if (iter == this->channels_.end()) {
    TransportChannel *channel = iter->second;
    tcp_socket = iter->second->GetTcpSocket();
    this->channels_.erase(iter);
    delete channel;
  }

  return tcp_socket;
}

void TransportAgent::OnFlushReceiveQueue(int64_t timer_id) {
  this->FlushReceiveQueue();
  this->SetNextFlushReceiveQueueTimer();
}

void TransportAgent::OnFlushSendQueue(int64_t timer_id) {
  this->FlushSendQueue();
  this->SetNextFlushSendQueueTimer();
}

::mysya::ioevent::EventLoop *GetNetworkEventLoop() const {
  return this->network_event_loop_;
}

::mysya::ioevent::EventLoop *GetAppEventLoop() const {
  return this->app_event_loop_;
}

void TransportAgent::OnHandlePending(::mysya::ioevent::TcpSocket *tcp_socket) {
  TransportChannelHashmap::const_iterator iter =
    this->channels_.find(tcp_socket->GetFileDescriptor());
  if (iter != this->channels_.end()) {
    MYSYA_ERROR("Duplicate tcp_socket(%d).", tcp_socket->GetFileDescriptor());
    return;
  }

  std::unique_ptr<TransportChannel> channel(
      new (std::nothrow) TransportChannel(tcp_socket, this));
  if (channel.get() == NULL) {
    MYSYA_ERROR("Allocate TransportChannel(%d) failed.", tcp_socket->GetFileDescriptor());
    return;
  }

  this->channels_.insert(std::make_pair(tcp_socket->GetFileDescriptor(), channel.get());
  channel.release();

  return;
}

}  // namespace qservice
}  // namespace mysya
