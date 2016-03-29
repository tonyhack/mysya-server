#include <mysya/qservice/transport_agent.h>

#include <memory>

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/mutex.h>
#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/tcp_socket.h>

#include <mysya/qservice/message_queue.h>

namespace mysya {
namespace qservice {

class TransportAgent::TransportChannel {
 public:
  TransportChannel(TransportAgent *agent, ::mysya::ioevent::TcpSocket *tcp_socket,
      int init_size = 1024, int ext_size = 256);
  ~TransportChannel();

  TransportAgent *GetHost() const { return this->host_; }
  int GetFileDescriptor() const { return this->tcp_socket_->GetFileDescriptor(); }

  ::mysya::ioevent::TcpSocket *GetTcpSocket() const { return this->tcp_socket_; }
  ::mysya::ioevent::DynamicBuffer *GetReadBuffer() { return &this->read_buffer_; }
  ::mysya::ioevent::DynamicBuffer *GetWriteBuffer() { return &this->write_buffer_; }

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

TransportAgent::TransportChannel::TransportChannel(TransportAgent *agent,
    ::mysya::ioevent::TcpSocket *tcp_socket, int init_size, int ext_size)
  : host_(agent), tcp_socket_(tcp_socket),
    read_buffer_(init_size, ext_size),
    write_buffer_(init_size, ext_size),
    errno_(0) {}

TransportAgent::TransportChannel::~TransportChannel() {}

bool TransportAgent::TransportChannel::SendMessage(const char *data, size_t size) {
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
    next_flush_receive_expired_msec_(kMaxFlushExporedMsec_),
    pending_receive_num_(0),
    next_flush_send_expired_msec_(kMaxFlushExporedMsec_),
    pending_send_num_(0),
    flush_receive_queue_timer_id_(0),
    flush_send_queue_timer_id_(0) {
  this->SetNextFlushSendQueueTimer();
  this->SetNextFlushReceiveQueueTimer();
}

TransportAgent::~TransportAgent() {}

::mysya::ioevent::EventLoop *TransportAgent::GetNetworkEventLoop() const {
  return this->network_event_loop_;
}

::mysya::ioevent::EventLoop *TransportAgent::GetAppEventLoop() const {
  return this->app_event_loop_;
}

bool TransportAgent::AddTcpSocket(::mysya::ioevent::TcpSocket *tcp_socket) {
  int sockfd = tcp_socket->GetFileDescriptor();
  TransportChannelHashmap::const_iterator iter = this->channels_.find(sockfd);
  if (iter != this->channels_.end()) {
    MYSYA_ERROR("Duplicate tcp_socket(%d).", sockfd);
    return false;
  }

  ::mysya::ioevent::EventChannel *event_channel = tcp_socket->GetEventChannel();

  std::unique_ptr<TransportChannel> transport_channel(
    new (std::nothrow) TransportChannel(this, tcp_socket));
  if (transport_channel.get() == NULL) {
    MYSYA_ERROR("Allocate TransportChannel(%d) failed.", sockfd);
    return false;
  }

  tcp_socket->SetAppHandle(transport_channel.get());
  this->channels_.insert(std::make_pair(sockfd, transport_channel.get()));

  event_channel->SetReadCallback(
      std::bind(&TransportAgent::OnSocketRead, this, std::placeholders::_1));
  event_channel->SetWriteCallback(
      std::bind(&TransportAgent::OnSocketWrite, this, std::placeholders::_1));
  event_channel->SetErrorCallback(
      std::bind(&TransportAgent::OnSocketError, this, std::placeholders::_1));

  if (event_channel->AttachEventLoop(this->network_event_loop_) == false) {
    MYSYA_ERROR("sockfd(%d) EventChannel::AttachEventLoop() failed.", sockfd);
    return false;
  }

  this->app_event_loop_->PushWakeupCallback(
      std::bind(&TransportAgent::OnHandleConnected, this, sockfd));

  transport_channel.release();

  return true;
}

bool TransportAgent::SendMessage(int sockfd, const char *data, size_t size) {
  this->pending_send_num_.fetch_add(1);
  ::mysya::ioevent::LockGuard lock(this->send_queue_.GetWriteMutex());
  return this->send_queue_.Write(sockfd, data, size) != (int)size;
}

void TransportAgent::FlushReceiveQueue() {
  this->pending_receive_num_.store(0);

  this->last_flush_receive_timestamp_ = this->app_event_loop_->GetTimestamp();

  int sockfd = -1;
  ::mysya::ioevent::DynamicBuffer buffer;

  ::mysya::ioevent::LockGuard lock_read(this->receive_queue_.GetReadMutex());
  for (;;) {
    int read_size = this->receive_queue_.Read(sockfd, buffer);
    if (read_size <= 0) {
      break;
    }

    if (this->receive_app_cb_) {
      this->receive_app_cb_(sockfd, buffer.ReadBegin(), (size_t)read_size);
    }
    buffer.ReadBytes(read_size);
  }

  ::mysya::ioevent::LockGuard lock_write(this->receive_queue_.GetWriteMutex());
  this->receive_queue_.Exchange();
}

void TransportAgent::FlushSendQueue() {
  this->pending_send_num_.store(0);

  this->last_flush_send_timestamp_ = this->network_event_loop_->GetTimestamp();

  int sockfd = -1;
  ::mysya::ioevent::DynamicBuffer buffer;

  ::mysya::ioevent::LockGuard lock_read(this->send_queue_.GetReadMutex());
  for (;;) {
    int read_size = this->send_queue_.Read(sockfd, buffer);
    if (read_size <= 0) {
      break;
    }

    TransportChannelHashmap::iterator iter = this->channels_.find(sockfd);
    if (iter == this->channels_.end()) {
      continue;
    }

    TransportChannel *transport_channel = iter->second;
    transport_channel->SendMessage(buffer.ReadBegin(), read_size);
    buffer.ReadBytes(read_size);
  }

  ::mysya::ioevent::LockGuard lock_write(this->send_queue_.GetWriteMutex());
  this->send_queue_.Exchange();
}

void TransportAgent::SetConnectAppCallback(const ConnectCallback &cb) {
  this->connect_app_cb_ = cb;
}

void TransportAgent::ResetConnectAppCallback() {
  ConnectCallback cb;
  this->connect_app_cb_.swap(cb);
}

void TransportAgent::SetReceiveAppCallback(const ReceiveCallback &cb) {
  this->receive_app_cb_ = cb;
}

void TransportAgent::ResetReceiveAppCallback() {
  ReceiveCallback cb;
  this->receive_app_cb_.swap(cb);
}

void TransportAgent::SetCloseAppCallback(const CloseCallback &cb) {
  this->close_app_cb_ = cb;
}

void TransportAgent::ResetCloseAppCallback() {
  CloseCallback cb;
  this->close_app_cb_.swap(cb);
}

void TransportAgent::SetErrorAppCallback(const ErrorCallback &cb) {
  this->error_app_cb_ = cb;
}

void TransportAgent::ResetErrorAppCallback() {
  ErrorCallback cb;
  this->error_app_cb_.swap(cb);
}

void TransportAgent::SetReceiveDecodeCallback(const ReceiveDecodeCallback &cb) {
  this->receive_decode_cb_ = cb;
}

void TransportAgent::ResetReceiveDecodeCallback() {
  ReceiveDecodeCallback cb;
  this->receive_decode_cb_.swap(cb);
}

int TransportAgent::DoReceive(int sockfd, const char *data, int size) {
  this->pending_receive_num_.fetch_add(1);
  ::mysya::ioevent::LockGuard lock_write(this->receive_queue_.GetWriteMutex());
  return this->receive_queue_.Write(sockfd, data, size);
}

void TransportAgent::SetNextFlushReceiveQueueTimer() {
  ::mysya::util::Timestamp now_timestamp = this->app_event_loop_->GetTimestamp();

  int pending_num = this->pending_receive_num_.load();
  int64_t distance_msec = now_timestamp.DistanceMillisecond(this->last_flush_receive_timestamp_);

  // estimate next one.
  if (pending_num <= 0) {
    this->next_flush_receive_expired_msec_ *= 2;
  } else {
    this->next_flush_receive_expired_msec_ = distance_msec / pending_num;
  }

  if (this->next_flush_receive_expired_msec_ > this->kMaxFlushExporedMsec_) {
    this->next_flush_receive_expired_msec_ = this->kMaxFlushExporedMsec_;
  } else if (this->next_flush_receive_expired_msec_ < this->kMinFlushExporedMsec_) {
    this->next_flush_receive_expired_msec_ = this->kMinFlushExporedMsec_;
  }

  this->app_event_loop_->StartTimer(this->next_flush_receive_expired_msec_,
      std::bind(&TransportAgent::OnFlushReceiveQueue, this, std::placeholders::_1), 1);

  MYSYA_DEBUG("[TransportAgent] StartTimer(%d) OnFlushReceiveQueue", this->next_flush_receive_expired_msec_);
}

void TransportAgent::SetNextFlushSendQueueTimer() {
  ::mysya::util::Timestamp now_timestamp = this->network_event_loop_->GetTimestamp();

  int pending_num = this->pending_send_num_.load();
  int64_t distance_msec = now_timestamp.DistanceMillisecond(this->last_flush_send_timestamp_);

  // estimate next one.
  if (pending_num <= 0) {
    this->next_flush_send_expired_msec_ *= 2;
  } else {
    this->next_flush_send_expired_msec_ = distance_msec / pending_num;
  }

  if (this->next_flush_send_expired_msec_ > this->kMaxFlushExporedMsec_) {
    this->next_flush_send_expired_msec_ = this->kMaxFlushExporedMsec_;
  } else if (this->next_flush_send_expired_msec_ < this->kMinFlushExporedMsec_) {
    this->next_flush_send_expired_msec_ = this->kMinFlushExporedMsec_;
  }

  this->network_event_loop_->StartTimer(this->next_flush_send_expired_msec_,
      std::bind(&TransportAgent::OnFlushSendQueue, this, std::placeholders::_1), 1);

  MYSYA_DEBUG("[TransportAgent] StartTimer(%d) OnFlushSendQueue", this->next_flush_send_expired_msec_);
}

::mysya::ioevent::TcpSocket *TransportAgent::RemoveTcpSocket(int sockfd) {
  ::mysya::ioevent::TcpSocket *tcp_socket = NULL;

  TransportChannelHashmap::iterator iter = this->channels_.find(sockfd);
  if (iter == this->channels_.end()) {
    TransportChannel *transport_channel = iter->second;
    tcp_socket = iter->second->GetTcpSocket();
    this->channels_.erase(iter);
    delete transport_channel;
  }

  return tcp_socket;
}

void TransportAgent::CloseTcpSocket(int sockfd) {
  delete this->RemoveTcpSocket(sockfd);
}

void TransportAgent::OnSocketRead(::mysya::ioevent::EventChannel *event_channel) {
  ::mysya::ioevent::TcpSocket *socket =
    (::mysya::ioevent::TcpSocket *)event_channel->GetAppHandle();

  int sockfd = socket->GetFileDescriptor();
  TransportChannel *transport_channel = (TransportChannel *)socket->GetAppHandle();
  ::mysya::ioevent::DynamicBuffer *receive_buffer = transport_channel->GetReadBuffer();

  bool received = false;
  bool peer_closed = false;

  for (;;) {
    int readable_bytes = socket->ReadableSize();
    if (readable_bytes <= 0) {
      peer_closed = true;
      break;
    }

    receive_buffer->ReserveWritableBytes(readable_bytes);

    int received_bytes = socket->Read(receive_buffer->WriteBegin(),
        receive_buffer->WritableBytes());
    if (received_bytes > 0) {
      receive_buffer->WrittenBytes(received_bytes);
      received = true;
    } else if (received_bytes == 0) {
      peer_closed = true;
    } else if (errno != EAGAIN) {
      transport_channel->SetErrno(errno);
      this->app_event_loop_->PushWakeupCallback(
          std::bind(&TransportAgent::OnHandleError, this, sockfd, errno));
      this->CloseTcpSocket(sockfd);
      return;
    } else {
      // do nothing.
    }

    break;
  }

  // received callback.
  if (received == true and this->receive_decode_cb_) {
    this->receive_decode_cb_(sockfd, receive_buffer);
  }

  if (peer_closed == true) {
    this->app_event_loop_->PushWakeupCallback(
        std::bind(&TransportAgent::OnHandleClosed, this, sockfd));
    this->CloseTcpSocket(sockfd);
  }
}

void TransportAgent::OnSocketWrite(::mysya::ioevent::EventChannel *event_channel) {
  ::mysya::ioevent::TcpSocket *socket =
    (::mysya::ioevent::TcpSocket *)event_channel->GetAppHandle();

  int sockfd = socket->GetFileDescriptor();
  TransportChannel *transport_channel = (TransportChannel *)socket->GetAppHandle();
  ::mysya::ioevent::DynamicBuffer *send_buffer = transport_channel->GetWriteBuffer();

  int sendable_bytes = send_buffer->ReadableBytes();
  int send_bytes = socket->Write(send_buffer->ReadBegin(), sendable_bytes);
  if (send_bytes < 0) {
    transport_channel->SetErrno(errno);

    this->app_event_loop_->PushWakeupCallback(
        std::bind(&TransportAgent::OnHandleError, this, sockfd, errno));
    this->CloseTcpSocket(sockfd);
    return;
  }

  send_buffer->ReadBytes(send_bytes);
}

void TransportAgent::OnSocketError(::mysya::ioevent::EventChannel *event_channel) {
  ::mysya::ioevent::TcpSocket *socket =
    (::mysya::ioevent::TcpSocket *)event_channel->GetAppHandle();

  int sockfd = socket->GetFileDescriptor();
  TransportChannel *transport_channel = (TransportChannel *)socket->GetAppHandle();
  transport_channel->SetErrno(errno);

  this->app_event_loop_->PushWakeupCallback(
      std::bind(&TransportAgent::OnHandleError, this, sockfd, errno));
  this->CloseTcpSocket(sockfd);
}

void TransportAgent::OnFlushReceiveQueue(int64_t timer_id) {
  this->SetNextFlushReceiveQueueTimer();
  this->FlushReceiveQueue();
}

void TransportAgent::OnFlushSendQueue(int64_t timer_id) {
  this->SetNextFlushSendQueueTimer();
  this->FlushSendQueue();
}

void TransportAgent::OnHandleConnected(int sockfd) {
  if (this->connect_app_cb_) {
    this->connect_app_cb_(sockfd, this);
  }
}

void TransportAgent::OnHandleClosed(int sockfd) {
  if (this->close_app_cb_) {
    this->close_app_cb_(sockfd);
  }
}

void TransportAgent::OnHandleError(int sockfd, int sys_errno) {
  if (this->error_app_cb_) {
    this->error_app_cb_(sockfd, sys_errno);
  }
}

}  // namespace qservice
}  // namespace mysya
