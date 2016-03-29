#ifndef MYSYA_QSERVICE_TRANSPORT_AGENT_H
#define MYSYA_QSERVICE_TRANSPORT_AGENT_H

#include <atomic>
#include <unordered_map>

#include <mysya/qservice/message_queue.h>
#include <mysya/util/timestamp.h>

namespace mysya {
namespace ioevent {

class EventChannel;
class EventLoop;
class TcpSocket;

}  // namespace ioevent
}  // namespace mysya

namespace mysya {
namespace qservice {

class TransportAgent {
 public:
  typedef std::function<void (int, TransportAgent *)> ConnectCallback;
  typedef std::function<void (int, const char *, int)> ReceiveCallback;
  typedef std::function<void (int)> CloseCallback;
  typedef std::function<void (int, int)> ErrorCallback;
  typedef std::function<void (int, ::mysya::ioevent::DynamicBuffer *)> ReceiveDecodeCallback;

  class TransportChannel;
  typedef std::unordered_map<int, TransportChannel *> TransportChannelHashmap;

  explicit TransportAgent(::mysya::ioevent::EventLoop *network_event_loop,
      ::mysya::ioevent::EventLoop *app_event_loop);
  ~TransportAgent();

  ::mysya::ioevent::EventLoop *GetNetworkEventLoop() const;
  ::mysya::ioevent::EventLoop *GetAppEventLoop() const;

  // running in network event loop.
  bool AddTcpSocket(::mysya::ioevent::TcpSocket *tcp_socket);
  // running in app event loop.
  bool SendMessage(int sockfd, const char *data, size_t size);

  // running in app event loop.
  void FlushReceiveQueue();
  // running in network event loop.
  void FlushSendQueue();

  // running in app event loop.
  void SetConnectAppCallback(const ConnectCallback &cb);
  void ResetConnectAppCallback();
  void SetReceiveAppCallback(const ReceiveCallback &cb);
  void ResetReceiveAppCallback();
  void SetCloseAppCallback(const CloseCallback &cb);
  void ResetCloseAppCallback();
  void SetErrorAppCallback(const ErrorCallback &cb);
  void ResetErrorAppCallback();

  // running in network event loop.
  void SetReceiveDecodeCallback(const ReceiveDecodeCallback &cb);
  void ResetReceiveDecodeCallback();

  // running in ReceiveDecodeCallback.
  int DoReceive(int sockfd, const char *data, int size);

 private:
  void SetNextFlushReceiveQueueTimer();
  void SetNextFlushSendQueueTimer();

  ::mysya::ioevent::TcpSocket *RemoveTcpSocket(int sockfd);
  void CloseTcpSocket(int sockfd);

  void OnSocketRead(::mysya::ioevent::EventChannel *event_channel);
  void OnSocketWrite(::mysya::ioevent::EventChannel *event_channel);
  void OnSocketError(::mysya::ioevent::EventChannel *event_channel);

  void OnFlushReceiveQueue(int64_t timer_id);
  void OnFlushSendQueue(int64_t timer_id);

  void OnHandleConnected(int sockfd);
  void OnHandleClosed(int sockfd);
  void OnHandleError(int sockfd, int sys_errno);

  ::mysya::ioevent::EventLoop *network_event_loop_;
  ::mysya::ioevent::EventLoop *app_event_loop_;
  TransportChannelHashmap channels_;

  MessageQueue receive_queue_;
  MessageQueue send_queue_;

  // callbacks for app event loop.
  ConnectCallback connect_app_cb_;
  ReceiveCallback receive_app_cb_;
  CloseCallback close_app_cb_;
  ErrorCallback error_app_cb_;

  ReceiveDecodeCallback receive_decode_cb_;

  static const int kMinFlushExporedMsec_ = 1;
  static const int kMaxFlushExporedMsec_ = 1000;

  int next_flush_receive_expired_msec_;
  ::mysya::util::Timestamp last_flush_receive_timestamp_;
  std::atomic<int> pending_receive_num_;

  int next_flush_send_expired_msec_;
  ::mysya::util::Timestamp last_flush_send_timestamp_;
  std::atomic<int> pending_send_num_;

  int64_t flush_receive_queue_timer_id_;
  int64_t flush_send_queue_timer_id_;
};

}  // qservice
}  // namespace mysya 

#endif  // MYSYA_QSERVICE_TRANSPORT_AGENT_H
