#ifndef TUTORIAL_ORCAS_GATEWAY_ROBOT_ACTOR_H
#define TUTORIAL_ORCAS_GATEWAY_ROBOT_ACTOR_H

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace gateway {
namespace robot {

class Actor {
 public:
  Actor(int sockfd);
  ~Actor();

  int GetSockfd() const;
  int GetId() const;

  void SendMessage(int type, ::google::protobuf::Message &data);

 private:
  int sockfd_;
};

}  // namespace robot
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_ROBOT_ACTOR_H
