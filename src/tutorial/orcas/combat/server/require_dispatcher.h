#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_REQUIRE_DISPATCHER_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_REQUIRE_DISPATCHER_H

#include <stdint.h>

#include <functional>
#include <list>
#include <unordered_map>

#include <mysya/util/class_util.h>

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class RequireDispatcher {
  typedef ::google::protobuf::Message ProtoMessage;
  typedef std::function<int (ProtoMessage *)> RequireCallback;
  typedef std::unordered_map<int32_t, RequireCallback> RequireCallbackHashmap;

 public:
  RequireDispatcher();
  ~RequireDispatcher();

  void Attach(int32_t type, const RequireCallback &cb);
  void Detach(int32_t type);
  int Dispatch(int32_t type, ProtoMessage *data);

 private:
  RequireCallbackHashmap cbs_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(RequireDispatcher);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_REQUIRE_DISPATCHER_H
