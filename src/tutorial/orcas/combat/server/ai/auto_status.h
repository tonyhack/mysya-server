#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_H

#include <stdint.h>

#include <map>
#include <functional>

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
namespace ai {

class Auto;

class AutoStatus {
 public:
  typedef ::google::protobuf::Message ProtoMessage;
  typedef std::function<void (const ProtoMessage *data)> EventCallback;
  typedef std::map<int, EventCallback> EventCallbackMap;
  typedef std::map<int, uint64_t> EventTokenMap;

  enum type {
    SEARCH = 1,           // 搜索目标
    CHASE = 2,            // 追击目标
    ATTACK = 3,           // 攻击目标
  };

  AutoStatus(Auto *host);
  virtual ~AutoStatus();

  virtual void Start();
  virtual void Stop();

  virtual type GetType() const = 0;

  void DispatchEvent(int type, const ProtoMessage *data);

 protected:
  bool GotoStatus(int status);
  void AttachEvent(int type, const EventCallback &cb);
  void DetachEvent(int type);

  Auto *host_;
  EventCallbackMap event_cbs_;
  EventTokenMap event_tokens_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(AutoStatus);
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_H
