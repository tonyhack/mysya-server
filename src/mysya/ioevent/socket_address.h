#ifndef MYSYA_IOEVENT_SOCKET_ADDRESS_H
#define MYSYA_IOEVENT_SOCKET_ADDRESS_H

#include <netinet/in.h>

#include <string>

#include <mysya/util/exception.h>

namespace mysya {
namespace ioevent {

class SocketAddress {
 public:
  typedef struct sockaddr_in NativeAddress;

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
  void GenerateNativeHandle() const;

  std::string host_;
  uint16_t port_;

  mutable bool generated_;
  mutable NativeAddress native_handle_;
};

}  // namespace ioevent
}  // namespace mysya

#endif  // MYSYA_IOEVENT_SOCKET_ADDRESS_H
