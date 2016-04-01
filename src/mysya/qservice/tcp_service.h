#ifndef MYSYA_QSERVICE_TCP_SERVICE_H
#define MYSYA_QSERVICE_TCP_SERVICE_H

#include <functional>
#include <map>
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
  friend class TransportAgent;

 public:
  typedef std::function<void (int, TransportAgent *)> ConnectCallback;
  typedef std::function<void (int, const char *, int)> ReceiveCallback;
  typedef std::function<void (int)> CloseCallback;
  typedef std::function<void (int, int)> ErrorCallback;
  typedef std::function<void (int, ::mysya::ioevent::DynamicBuffer *)> ReceiveDecodeCallback;

  typedef std::function<void (int)> ListenedCallback;
  typedef std::function<void (int, int)> ListenErrorCallback;

  typedef std::function<void (int, TransportAgent *)> AsyncConnectedCallback;
  typedef std::function<void (int, int)> AsyncConnectErroCallback;

  typedef std::vector<TransportAgent *> TransportAgentVector;
  typedef std::map< ::mysya::ioevent::TcpSocket *, TransportAgent *> TcpSocketMap;

  explicit TcpService(::mysya::ioevent::EventLoop *app_event_loop,
    EventLoopThreadPool *thread_pool);
  ~TcpService();

  // running in app event loop.
  int AsyncConnect(const ::mysya::ioevent::SocketAddress &addr, int timeout_ms);
  int Listen(const ::mysya::ioevent::SocketAddress &addr);

  // callback running in app event loop.
  ListenedCallback GetListenedCallback();
  void SetListenedCallback(const ListenedCallback &cb);
  void ResetListenedCallback();

  // callback running in app event loop.
  ListenErrorCallback GetListenErrorCallback();
  void SetListenErrorCallback(const ListenErrorCallback &cb);
  void ResetListenErrorCallback();

  // callback running in app event loop.
  AsyncConnectedCallback GetAsyncConnectedCallback() const;
  void SetAsyncConnectedCallback(const AsyncConnectedCallback &cb);
  void ResetAsyncConnectedCallback();

  // callback running in app event loop.
  AsyncConnectErroCallback GetAsyncConnectErroCallback() const;
  void SetAsyncConnectErroCallback(const AsyncConnectErroCallback &cb);
  void ResetAsyncConnectErroCallback();

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
  bool BuildListenSocket(TransportAgent *transport_agent,
      ::mysya::ioevent::TcpSocket *listen_socket,
      const ::mysya::ioevent::SocketAddress &listen_addr,
      int backlog = 256);
  bool BuildConnectedSocket(::mysya::ioevent::TcpSocket *socket);
  TransportAgent *AllocateTransportAgent();

  void OnListenRead(::mysya::ioevent::EventChannel *event_channel);
  void OnListenError(::mysya::ioevent::EventChannel *event_channel);

  // ::mysya::ioevent::SocketAddress listen_addr_;
  ::mysya::ioevent::EventLoop *app_event_loop_;
  EventLoopThreadPool *thread_pool_;

  size_t next_agent_;
  TransportAgentVector transport_agents_;
  // ::mysya::ioevent::TcpSocket listen_socket_;

  TcpSocketMap listen_sockets_;

  ListenedCallback listened_cb_;
  ListenErrorCallback listen_error_cb_;

  AsyncConnectedCallback async_connected_cb_;
  AsyncConnectErroCallback async_connect_error_cb_;

  ConnectCallback connect_cb_;
  ReceiveCallback receive_cb_;
  CloseCallback close_cb_;
  ErrorCallback error_cb_;

  ReceiveDecodeCallback receive_decode_cb_;
};

}  // namespace qservice
}  // namespace mysya

#endif  // MYSYA_QSERVICE_TCP_SERVICE_H
