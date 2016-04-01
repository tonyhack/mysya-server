#ifndef MYSYA_QSERVICE_ERRNO_H
#define MYSYA_QSERVICE_ERRNO_H

namespace mysya {
namespace qservice {

struct SocketErrno {
  enum type {
    UNKNOWN = -1,
    CONNECT_TIMEOUT = -2,
    ATTACH_EVENT_LOOP = -3,
    BUILD_CONNECTED_SOCKET = -4,
    DUPLICATE_SOCKET = -5,
  };
};

}  // namespace qservice
}  // namespace mysya

#endif  // MYSYA_QSERVICE_ERRNO_H
