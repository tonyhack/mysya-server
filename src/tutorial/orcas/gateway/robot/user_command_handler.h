#ifndef TUTORIAL_ORCAS_GATEWAY_ROBOT_USER_COMMAND_HANDLER_H
#define TUTORIAL_ORCAS_GATEWAY_ROBOT_USER_COMMAND_HANDLER_H

#include <map>
#include <string>
#include <vector>

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace gateway {
namespace robot {

class UserCommandHandler {
 public:
  typedef std::vector<std::string> ArgsVector;
  typedef void (*CommandCallback)(const ArgsVector &);
  typedef std::map<std::string, CommandCallback> CommandCallbackMap;

  UserCommandHandler();
  ~UserCommandHandler();

  bool Parse(const std::string &command);

 private:
  CommandCallbackMap cbs_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(UserCommandHandler);
};

}  // namespace robot
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_ROBOT_USER_COMMAND_HANDLER_H
