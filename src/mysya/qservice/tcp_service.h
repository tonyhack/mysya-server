#ifndef MYSYA_QSERVICE_TCP_SERVICE_H
#define MYSYA_QSERVICE_TCP_SERVICE_H

#include <functional>
#include <vector>

#include <mysya/ioevent/socket_address.h>

namespace mysya {
namespace ioevent {

class EventLoop;
class EventLoopThreadPool;

}  // namespace ioevent
}  // namespace mysya

namespace mysya {
namespace qservice {

class EventLoopThreadPool;

class TcpService {
 public:
  typedef std::vector<TransportAgent *> TransportAgentVector;

  explicit TcpService(const ::mysya::ioevent::SocketAddress &listen_addr,
      ::mysya::ioevent::EventLoop *app_event_loop, EventLoopThreadPool *thread_pool);
  ~TcpService();

 private:
  bool BuildListenSocket(const ::mysya::ioevent::SocketAddress &listen_addr);
  bool BuildConnectedSocket(std::unique_ptr<::mysya::ioevent::TcpSocket> &socket);
  TransportAgent *AllocateTransportAgent();

  void OnListenRead(::mysya::ioevent::EventChannel *event_channel);
  void OnListenError(::mysya::ioevent::EventChannel *event_channel);

  ::mysya::ioevent::SocketAddress listen_addr_;
  ::mysya::ioevent::EventLoop *app_event_loop_;
  EventLoopThreadPool *thread_pool_;

  size_t next_agent_;
  TransportAgentVector transport_agents_;
  ::mysya::ioevent::TcpSocket listen_socket_;
};

}  // namespace qservice
}  // namespace mysya

#endif  // MYSYA_QSERVICE_TCP_SERVICE_H
