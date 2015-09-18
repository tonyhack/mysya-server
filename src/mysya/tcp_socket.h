#ifndef MYSYA_TCP_SOCKET_H
#define MYSYA_TCP_SOCKET_H

#include <mysya/class_util.h>
#include <mysya/event_channel.h>
#include <mysya/socket_address.h>

namespace mysya {

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

  bool GetLocalAddress(SocketAddress *addr) const;
  bool GetPeerAddress(SocketAddress *addr) const;

  EventChannel *GetEventChannel() { return &this->event_channel_; }

  int GetFileDescriptor() const { return this->event_channel_.GetFileDescriptor(); }
  void SetFileDescriptor(int value) { this->event_channel_.SetFileDescriptor(value); }

 private:
  EventChannel event_channel_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(TcpSocket);
};

}  // namespace mysya

#endif  // MYSYA_TCP_SOCKET_H
