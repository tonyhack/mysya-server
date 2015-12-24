#ifndef MYSYA_TCP_SOCKET_APP_H
#define MYSYA_TCP_SOCKET_APP_H

#include <stdint.h>

#include <functional>
#include <memory>
#include <unordered_map>

#include <mysya/event_loop.h>

namespace mysya {

class DynamicBuffer;
class SocketAddress;
class TcpSocket;

class TcpSocketApp {
 public:
  class TcpConnection;

  typedef std::unordered_map<int, TcpSocket *> TcpSocketHashmap;
  typedef std::unordered_map<int, TcpConnection *> TcpConnectionHashmap;

  typedef std::unordered_map<int64_t, int> TimerSocketHashmap;
  typedef std::unordered_map<int, int64_t> SocketTimerHashmap;

  typedef EventLoop::ExpireCallback ExpireCallback;

  typedef std::function<void (TcpSocketApp *, int)> ConnectionCallback;
  typedef std::function<void (TcpSocketApp *, int, DynamicBuffer *)> ReceiveCallback;
  typedef std::function<void (TcpSocketApp *, int)> SendCompleteCallback;
  typedef std::function<void (TcpSocketApp *, int)> CloseCallback;
  typedef std::function<void (TcpSocketApp *, int, int)> ErrorCallback;

  explicit TcpSocketApp(EventLoop *event_loop);
  ~TcpSocketApp();

  bool GetPeerAddress(int sockfd, SocketAddress *addr);
  bool GetLocalAddress(int sockfd, SocketAddress *addr);

  void SetConnectionCallback(const ConnectionCallback &cb);
  void ResetConnectionCallback();
  const ConnectionCallback &GetConnectionCallback() const;

  void SetReceiveCallback(const ReceiveCallback &cb);
  void ResetReceiveCallback();
  const ReceiveCallback &GetReceiveCallback() const;

  void SetSendCompleteCallback(const SendCompleteCallback &cb);
  void ResetetSendCompleteCallback();
  const SendCompleteCallback &GetSendCompleteCallback() const;

  void SetCloseCallback(const CloseCallback &cb);
  void ResetCloseCallback();
  const CloseCallback &GetCloseCallback() const;

  void SetErrorCallback(const ErrorCallback &cb);
  void ResetErrorCallback();
  const ErrorCallback &GetErrorCallback() const;

  bool Listen(const SocketAddress &addr);
  void Close(int sockfd);

  int Connect(const SocketAddress &addr);
  bool AsyncConnect(const SocketAddress &addr, int timeout_ms);

  bool SendMessage(int sockfd, const char *data, size_t size);

  void SetListenBacklog(int value) { this->listen_backlog_ = value; }
  int GetListenBacklog() const { return this->listen_backlog_; }

  void SetSocketBufferInitSize(int value) { this->socket_buffer_init_size_ = value; }
  int GetSocketBufferInitSize() const { return this->socket_buffer_init_size_; }

  void SetSocketBufferExtendSize(int value) { this->socket_buffer_extend_size_ = value; }
  int GetSocketBufferExtendSize() const { return this->socket_buffer_extend_size_; }

 private:
  bool BuildListenSocket(std::unique_ptr<TcpSocket> &socket, const SocketAddress &addr);
  bool BuildConnectedSocket(std::unique_ptr<TcpSocket> &socket);
  bool BuildAsyncConnectSocket(std::unique_ptr<TcpSocket> &socket, int timeout_ms = 0);

  void AddSocketTimer(int sockfd, int expire_ms, const ExpireCallback &cb);
  void RemoveSocketTimer(int sockfd);

  void OnListenRead(EventChannel *event_channel);
  void OnListenError(EventChannel *event_channel);

  void OnConnectWrite(EventChannel *event_channel);
  void OnConnectError(EventChannel *event_channel);
  void OnConnectTimeout(int64_t timer_id);

  void OnSocketRead(EventChannel *event_channel);
  void OnSocketWrite(EventChannel *event_channel);
  void OnSocketError(EventChannel *event_channel);

  EventLoop *event_loop_;

  TcpSocketHashmap sockets_;
  TcpConnectionHashmap connections_;

  ConnectionCallback connection_cb_;
  ReceiveCallback receive_cb_;
  SendCompleteCallback send_complete_cb_;
  CloseCallback close_cb_;
  ErrorCallback error_cb_;

  TimerSocketHashmap timer_socket_ids_;
  SocketTimerHashmap socket_timer_ids_;

  int listen_backlog_;
  int socket_buffer_init_size_;
  int socket_buffer_extend_size_;
};

}  // namespace mysya

#endif  // MYSYA_TCP_SOCKET_APP_H
