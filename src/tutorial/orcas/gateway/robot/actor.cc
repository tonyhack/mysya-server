#include "tutorial/orcas/gateway/robot/actor.h"

#include <google/protobuf/message.h>

#include "tutorial/orcas/gateway/robot/robot_app.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace robot {

Actor::Actor(int sockfd)
  : sockfd_(sockfd) {}
Actor::~Actor() {}

int Actor::GetSockfd() const {
  return this->sockfd_;
}

int Actor::GetId() const {
  return this->sockfd_;
}

void Actor::SendMessage(int type, ::google::protobuf::Message &data) {
  RobotApp::GetInstance()->SendMessage(this->sockfd_, type, data);
}

}  // namespace robot
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
