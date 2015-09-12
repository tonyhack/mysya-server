#ifndef MYSYA_SOCKET_ADDRESS_H
#define MYSYA_SOCKET_ADDRESS_H

#include <netinet/in.h>

#include <string>

namespace mysya {

class SocketAddress {
  typedef NativeAddress struct sockaddr_in;

 public:
  SocketAddress();
  SocketAddress(const std::string &host, uint16_t port);
  SocketAddress(const SocketAddress &from);
  ~SocketAddress();

  const std::string &GetHost() const;
  void SetHost(const std::string &value);

  uint16_t GetPort() const;
  void SetPort(uint16_t value);

  const NativeAddress *GetNativeHandle() const;
  void SetNativeHandle(const NativeAddress &value);

  size_t GetNativeHandleSize() const;

  SocketAddress &operator=(const SocketAddress &from) {
    this->CopyFrom(from);
    return *this;
  }

 private:
  void CopyFrom(const SocketAddress &from);
  void GenerateNativeHandle();

  std::string host_;
  uint16_t port_;

  bool generated_;
  NativeAddress native_handle_;
};

}  // namespace mysya

#endif  // MYSYA_SOCKET_ADDRESS_H
