#ifndef MYSYA_IOEVENT_TCP_SOCKET_H
#define MYSYA_IOEVENT_TCP_SOCKET_H

#include <mysya/ioevent/event_channel.h>
#include <mysya/ioevent/socket_address.h>
#include <mysya/util/class_util.h>

namespace mysya {
namespace ioevent {

class TcpSocket {
 public:
  TcpSocket();
  ~TcpSocket();

  bool Open();
  void Close();

  bool Connect(const SocketAddress &peer_addr);
  bool AsyncConnect(const SocketAddress &peer_addr);

  bool Bind(const SocketAddress &addr);
  bool Listen(int backlog);
  bool Accept(TcpSocket *peer_socket);

  int ReadableSize() const;
  int Read(char *data, size_t size);
  int Write(const char *data, size_t size);

  bool SetReuseAddr();
  bool SetTcpNoDelay();
  bool SetNonblock();
  bool SetCloseExec();

  bool GetLocalAddress(SocketAddress *addr) const;
  bool GetPeerAddress(SocketAddress *addr) const;

  EventChannel *GetEventChannel() { return &this->event_channel_; }

  int GetFileDescriptor() const { return this->event_channel_.GetFileDescriptor(); }
  void SetFileDescriptor(int value) { this->event_channel_.SetFileDescriptor(value); }

  void SetAppHandle(void *handle) { this->app_handle_ = handle; }
  void *GetAppHandle() const { return this->app_handle_; }

 private:
  EventChannel event_channel_;
  void *app_handle_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(TcpSocket);
};

}  // namespace ioevent
}  // namespace mysya

#endif  // MYSYA_IOEVENT_TCP_SOCKET_H
