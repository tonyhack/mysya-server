#ifndef MYSYA_QSERVICE_TRANSPORT_AGENT_H
#define MYSYA_QSERVICE_TRANSPORT_AGENT_H

#include <unordered_map>

#include <mysya/qservice/message_queue.h>

namespace mysya {
namespace ioevent {

class EventLoop;
class TcpSocket;

}  // namespace ioevent
}  // namespace mysya

namespace mysya {
namespace qservice {

class TransportAgent {
 public:
  typedef std::function<void (int, TransportAgent *)> ConnectCallback;
  typedef std::function<void (int, const char *, size_t)> ReceiveCallback;
  typedef std::function<void (int)> CloseCallback;

  class TransportChannel;
  typedef std::unordered_map<int, TransportChannel *> TransportChannelHashmap;

  explicit TransportAgent(::mysya::ioevent::EventLoop *network_event_loop,
      ::mysya::ioevent::EventLoop *app_event_loop);
  ~TransportAgent();

  void PushPending(::mysya::ioevent::TcpSocket *tcp_socket);
  bool SendMessage(int sockfd, const char *data, size_t size);

  void FlushReceiveQueue();
  void FlushSendQueue();

  ::mysya::ioevent::EventLoop *GetNetworkEventLoop() const;
  ::mysya::ioevent::EventLoop *GetAppEventLoop() const;

  void SetConnectCallback(const ConnectCallback &cb);
  void ResetConnectCallback();
  void SetReceiveCallback(const ReceiveCallback &cb);
  void ResetReceiveCallback();
  void SetCloseCallback(const CloseCallback &cb);
  void ResetCloseCallback();

 private:
  void SetNextFlushReceiveQueueTimer();
  void SetNextFlushSendQueueTimer();

  ::mysya::ioevent::TcpSocket *RemoveTcpSocket(int sockfd);

  void OnFlushReceiveQueue(int64_t timer_id);
  void OnFlushSendQueue(int64_t timer_id);

  void OnHandlePending(::mysya::ioevent::TcpSocket *tcp_socket);
  void OnRemoveCallback();

  ::mysya::ioevent::EventLoop *network_event_loop_;
  ::mysya::ioevent::EventLoop *app_event_loop_;
  TransportChannelHashmap channels_;

  MessageQueue receive_queue_;
  MessageQueue send_queue_;

  ReceiveCallback receive_cb_;

  int64_t flush_receive_queue_timer_id_;
  int64_t flush_send_queue_timer_id_;
};

}  // qservice
}  // namespace mysya 

#endif  // MYSYA_QSERVICE_TRANSPORT_AGENT_H
