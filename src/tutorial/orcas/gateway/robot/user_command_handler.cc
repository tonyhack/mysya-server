#include "tutorial/orcas/gateway/robot/user_command_handler.h"

#include <string.h>
#include <ctype.h>

#include <algorithm>

#include <mysya/ioevent/logger.h>
#include <mysya/ioevent/socket_address.h>

#include "tutorial/orcas/gateway/robot/actor.h"
#include "tutorial/orcas/gateway/robot/robot_app.h"
#include "tutorial/orcas/protocol/cc/message.pb.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace robot {

void StringSplit(const char *str, const char *sep,
    std::vector<std::string> *result, int max_split = -1) {
  size_t sep_len = strlen(sep);
  if (0 == sep_len) {
    return;
  }

  int count = 0;
  const char *last = str;
  for (;;) {
    if (max_split > 0 && ++count >= max_split) {
      result->push_back(std::string(last));
      break;
    }

    const char *next = ::strstr(last, sep);
    if (NULL == next) {
      result->push_back(std::string(last));
      break;
    }

    result->push_back(std::string(last, next - last));

    last = next + sep_len;
  }
}

static void UsageActorLogin() {
  ::printf("actor_login(al) <server_ip> <server_port>\n");
}

static void UsageRunCommand() {
  ::printf("run <actor_no> <command>\n");
}

static void CommandHelp(const UserCommandHandler::ArgsVector &args) {
  UsageActorLogin();
  UsageRunCommand();
}

static void CommandActorLogin(const UserCommandHandler::ArgsVector &args) {
  if (args.size() < 3) {
    ::printf("argument num is invalid.\n");
    return;
  }

  ::mysya::ioevent::SocketAddress server_addr(args[1], atoi(args[2].data()));
  if (RobotApp::GetInstance()->Connect(server_addr, 5000) == false) {
    ::printf("error server addr(%s:%s).\n", args[1].data(), args[2].data());
    return;
  }
}

static void OnCommandLogin(Actor *actor, const UserCommandHandler::ArgsVector &args) {
  ::protocol::MessageLoginRequest message;
  message.set_name(args[3]);
  actor->SendMessage(::protocol::MESSAGE_LOGIN_REQUEST, message);

  MYSYA_DEBUG("MessageLoginRequest");
}

static void OnCommandCombat(Actor *actor, const UserCommandHandler::ArgsVector &args) {
  ::protocol::MessageCombatRequest message;
  message.add_warrior_id(atoi(args[3].data()));
  actor->SendMessage(::protocol::MESSAGE_COMBAT_REQUEST, message);
}

static void OnCommandBuild(Actor *actor, const UserCommandHandler::ArgsVector &args) {
  ::protocol::MessageCombatActionRequest message;

  ::protocol::CombatAction *action = message.mutable_action();
  action->set_type(::protocol::COMBAT_ACTION_TYPE_BUILD);
  action->set_timestamp(0);

  ::protocol::CombatBuildAction *build_action = action->mutable_build_action();
  build_action->set_building_id(atoi(args[3].data()));
  build_action->set_warrior_conf_id(atoi(args[4].data()));

  actor->SendMessage(::protocol::MESSAGE_COMBAT_ACTION_REQUEST, message);
}

static void OnCommandMove(Actor *actor, const UserCommandHandler::ArgsVector &args) {
}

static void CommandRunCommand(const UserCommandHandler::ArgsVector &args) {
  if (args.size() < 3) {
    ::printf("argument num is invalid.\n");
    return;
  }

  int64_t actor_id = ::atol(args[1].c_str());

  Actor *actor = RobotApp::GetInstance()->GetActor(actor_id);
  if (actor == NULL) {
    printf("arguments <actor_no> is invalid.\n");
    return;
  }

  if (args[2] == "login") {
    OnCommandLogin(actor, args);
  } else if (args[2] == "combat") {
    OnCommandCombat(actor, args);
  } else if (args[2] == "build") {
    OnCommandBuild(actor, args);
  } else if (args[2] == "move") {
    OnCommandMove(actor, args);
  } else {
  }
}

UserCommandHandler::UserCommandHandler() {
  this->cbs_.insert(std::make_pair("help", &CommandHelp));
  this->cbs_.insert(std::make_pair("h", &CommandHelp));

  this->cbs_.insert(std::make_pair("actor_login", &CommandActorLogin));
  this->cbs_.insert(std::make_pair("al", &CommandActorLogin));

  this->cbs_.insert(std::make_pair("run", &CommandRunCommand));
}

UserCommandHandler::~UserCommandHandler() {}

bool UserCommandHandler::Parse(const std::string &command) {
  if (command.empty() == true) {
    return true;
  }

  std::vector<std::string> args;

  StringSplit(command.c_str(), " ", &args);

  if (args.empty() == true) {
    return true;
  }

  CommandCallbackMap::const_iterator iter = this->cbs_.find(args[0]);
  if (iter == this->cbs_.end()) {
    return false;
  }

  iter->second(args);

  return true;
}

}  // namespace robot
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
