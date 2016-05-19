#include "tutorial/orcas/gateway/robot/message_handler.h"

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/gateway/robot/actor.h"
#include "tutorial/orcas/gateway/robot/robot_app.h"
#include "tutorial/orcas/protocol/cc/message.pb.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace robot {

MessageHandler::MessageHandler() {}
MessageHandler::~MessageHandler() {}

void MessageHandler::SetHandlers() {
  RobotApp::GetInstance()->SetMessageCalback(::protocol::MESSAGE_LOGIN_RESPONSE,
      std::bind(&MessageHandler::OnMessageLoginResponse, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  RobotApp::GetInstance()->SetMessageCalback(::protocol::MESSAGE_COMBAT_RESPONSE,
      std::bind(&MessageHandler::OnMessageCombatResponse, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  RobotApp::GetInstance()->SetMessageCalback(::protocol::MESSAGE_COMBAT_ACTION_REQUEST,
      std::bind(&MessageHandler::OnMessageCombatActionResponse, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
}

void MessageHandler::ResetHandlers() {
  RobotApp::GetInstance()->ResetMessageCallback(::protocol::MESSAGE_LOGIN_RESPONSE);
  RobotApp::GetInstance()->ResetMessageCallback(::protocol::MESSAGE_COMBAT_RESPONSE);
  RobotApp::GetInstance()->ResetMessageCallback(::protocol::MESSAGE_COMBAT_ACTION_REQUEST);
}

void MessageHandler::OnMessageLoginResponse(Actor *actor,
    const char *data, int size) {
  ::protocol::MessageLoginResponse message;
  if (message.ParseFromArray(data, size) == false) {
    MYSYA_ERROR("MessageLoginResponse::ParseFromArray() failed.");
    return;
  }

  MYSYA_DEBUG("<<<<<<< MessageLoginResponse -------");
  message.PrintDebugString();
  MYSYA_DEBUG("------- MessageLoginResponse >>>>>>>");
}

void MessageHandler::OnMessageCombatResponse(Actor *actor,
    const char *data, int size) {
  ::protocol::MessageCombatResponse message;
  if (message.ParseFromArray(data, size) == false) {
    MYSYA_ERROR("MessageCombatResponse::ParseFromArray() failed.");
    return;
  }

  MYSYA_DEBUG("<<<<<<< MessageCombatResponse -------");
  message.PrintDebugString();
  MYSYA_DEBUG("------- MessageCombatResponse >>>>>>>");
}

void MessageHandler::OnMessageCombatActionResponse(Actor *actor,
    const char *data, int size) {
  ::protocol::MessageCombatActionResponse message;
  if (message.ParseFromArray(data, size) == false) {
    MYSYA_ERROR("MessageCombatActionResponse::ParseFromArray() failed.");
    return;
  }

  MYSYA_DEBUG("<<<<<<< MessageCombatActionResponse -------");
  message.PrintDebugString();
  MYSYA_DEBUG("------- MessageCombatActionResponse >>>>>>>");
}

}  // namespace robot
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
