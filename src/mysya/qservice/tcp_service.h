#ifndef MYSYA_QSERVICE_TCP_SERVICE_H
#define MYSYA_QSERVICE_TCP_SERVICE_H

#include <functional>
#include <memory>
#include <vector>

#include <mysya/ioevent/dynamic_buffer.h>
#include <mysya/ioevent/socket_address.h>
#include <mysya/ioevent/tcp_socket.h>

namespace mysya {
namespace ioevent {

class EventChannel;
class EventLoop;
class EventLoopThreadPool;

}  // namespace ioevent
}  // namespace mysya

namespace mysya {
namespace qservice {

class EventLoopThreadPool;
class TransportAgent;

class TcpService {
 public:
  typedef std::function<void (int, TransportAgent *)> ConnectCallback;
  typedef std::function<void (int, const char *, int)> ReceiveCallback;
  typedef std::function<void (int)> CloseCallback;
  typedef std::function<void (int, int)> ErrorCallback;
  typedef std::function<void (int, ::mysya::ioevent::DynamicBuffer *)> ReceiveDecodeCallback;

  typedef std::vector<TransportAgent *> TransportAgentVector;

  explicit TcpService(const ::mysya::ioevent::SocketAddress &listen_addr,
      ::mysya::ioevent::EventLoop *app_event_loop, EventLoopThreadPool *thread_pool);
  ~TcpService();

  // callback running in app event loop.
  ConnectCallback GetConnectCallback() const;
  void SetConnectCallback(const ConnectCallback &cb);
  void ResetConnectCallback();

  // callback running in app event loop.
  ReceiveCallback GetReceiveCallback() const;
  void SetReceiveCallback(const ReceiveCallback &cb);
  void ResetReceiveCallback();

  // callback running in app event loop.
  CloseCallback GetCloseCallback() const;
  void SetCloseCallback(const CloseCallback &cb);
  void ResetCloseCallback();

  // callback running in app event loop.
  ErrorCallback GetErrorCallback() const;
  void SetErrorCallback(const ErrorCallback &cb);
  void ResetErrorCallback();

  // callback running in network event loop.
  ReceiveDecodeCallback GetReceiveDecodeCallback() const;
  void SetReceiveDecodeCallback(const ReceiveDecodeCallback &cb);
  void ResetReceiveDecodeCallback();

 private:
  bool BuildListenSocket(const ::mysya::ioevent::SocketAddress &listen_addr);
  bool BuildConnectedSocket(std::unique_ptr< ::mysya::ioevent::TcpSocket> &socket);
  TransportAgent *AllocateTransportAgent();

  void OnListenRead(::mysya::ioevent::EventChannel *event_channel);
  void OnListenError(::mysya::ioevent::EventChannel *event_channel);

  ::mysya::ioevent::SocketAddress listen_addr_;
  ::mysya::ioevent::EventLoop *app_event_loop_;
  EventLoopThreadPool *thread_pool_;

  size_t next_agent_;
  TransportAgentVector transport_agents_;
  ::mysya::ioevent::TcpSocket listen_socket_;

  ConnectCallback connect_cb_;
  ReceiveCallback receive_cb_;
  CloseCallback close_cb_;
  ErrorCallback error_cb_;

  ReceiveDecodeCallback receive_decode_cb_;
};

}  // namespace qservice
}  // namespace mysya

#endif  // MYSYA_QSERVICE_TCP_SERVICE_H
