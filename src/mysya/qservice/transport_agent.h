#ifndef MYSYA_QSERVICE_TRANSPORT_AGENT_H
#define MYSYA_QSERVICE_TRANSPORT_AGENT_H

#include <functional>
#include <unordered_map>

#include <mysya/qservice/message_queue.h>
#include <mysya/qservice/tcp_service.h>
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
  class TransportChannel;
  typedef std::unordered_map<int, TransportChannel *> TransportChannelHashmap;

  explicit TransportAgent(TcpService *host, ::mysya::ioevent::EventLoop *network_event_loop,
      ::mysya::ioevent::EventLoop *app_event_loop);
  ~TransportAgent();

  ::mysya::ioevent::EventLoop *GetNetworkEventLoop() const;
  ::mysya::ioevent::EventLoop *GetAppEventLoop() const;

  // running in network event loop.
  bool AddTcpSocket(::mysya::ioevent::TcpSocket *tcp_socket);
  // running in app event loop.
  bool SendMessage(int sockfd, const char *data, size_t size);

  // running in ReceiveDecodeCallback.
  int DoReceive(int sockfd, const char *data, int size);

 private:
  ::mysya::ioevent::TcpSocket *RemoveTcpSocket(int sockfd);
  void CloseTcpSocket(int sockfd);

  void OnSocketRead(::mysya::ioevent::EventChannel *event_channel);
  void OnSocketWrite(::mysya::ioevent::EventChannel *event_channel);
  void OnSocketError(::mysya::ioevent::EventChannel *event_channel);

  void OnHandleConnected(int sockfd);
  void OnHandleClosed(int sockfd);
  void OnHandleError(int sockfd, int sys_errno);

  void OnReceiveQueueReady(int host, const char *data, int size);
  void OnSendQueueReady(int host, const char *data, int size);

  TcpService *host_;

  ::mysya::ioevent::EventLoop *network_event_loop_;
  ::mysya::ioevent::EventLoop *app_event_loop_;
  TransportChannelHashmap channels_;

  MessageQueue receive_queue_;
  MessageQueue send_queue_;
};

}  // qservice
}  // namespace mysya 

#endif  // MYSYA_QSERVICE_TRANSPORT_AGENT_H
