#ifndef MYSYA_TCP_SOCKET_APP_H
#define MYSYA_TCP_SOCKET_APP_H

namespace mysya {

class TcpConnection;

class TcpSocketApp {
 public:
  typedef std::unordered_map<int, TcpSocket *> TcpSocketHashmap;
  typedef std::unordered_map<int, TcpConnection *> TcpConnectionHashmap;

  typedef std::function<void (TcpSocketApp *, int)> ConnectionCallback;
  typedef std::function<void (TcpSocketApp *, int, DynamicBuffer *)> ReceiveCallback;
  typedef std::function<void (TcpSocketApp *, int)> SendCompleteCallback;
  typedef std::function<void (TcpSocketApp *, int)> CloseCallback;
  typedef std::function<void (TcpSocketApp *, int)> ErrorCallback;

  explicit TcpSocketApp(EventLoop *event_loop);
  ~TcpSocketApp();

  void SetConnectionCallback(const ConnectionCallback &cb);
  void ResetConnectionCallback();

  void SetReceiveCallback(const ReceiveCallback &cb);
  void ResetReceiveCallback();

  void SetSendCompleteCallback(const SendCompleteCallback &cb);
  void ResetetSendCompleteCallback();

  void SetCloseCallback(const CloseCallback &cb);
  void ResetCloseCallback();

  void SetErrorCallback(const ErrorCallback &cb);
  void ResetErrorCallback();

  bool Listen(const SocketAddress &addr);
  void Close(int sockfd);

  bool Connect(const SocketAddress &addr);
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

  void OnListenRead(EventChannel *event_channel);
  void OnListenError(EventChannel *event_channel);

  void OnConnectWrite(EventChannel *event_channel);
  void OnConnectError(EventChannel *event_channel);

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

  int listen_backlog_;
  int socket_buffer_init_size_;
  int socket_buffer_extend_size_;
};

}  // namespace mysya

#endif  // MYSYA_TCP_SOCKET_APP_H
