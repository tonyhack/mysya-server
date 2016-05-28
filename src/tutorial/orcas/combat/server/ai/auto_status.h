#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_H

#include <map>
#include <functional>

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
    SEARCH = 1;           // 搜索目标
    CHASE = 2;            // 追击目标
    ATTACK = 3;           // 攻击目标
  };

  AutoStatus(Auto *host);
  virtual ~AutoStatus();

  virtual void Start();
  virtual void Stop();

  virtual type GetType() const = 0;

 protected:
  void AttachEvent(int type, const EventCallback &cb);
  void DetachEvent(int type);
  void DispatchEvent(int type, const ProtoMessage *data);

  Auto *host_;
  EventCallbackMap event_cbs_;
  EventTokenMap event_tokens_;
};

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_AI_AUTO_STATUS_H
