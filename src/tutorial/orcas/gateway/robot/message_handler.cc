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
  RobotApp::GetInstance()->SetMessageCallback(::protocol::MESSAGE_LOGIN_RESPONSE,
      std::bind(&MessageHandler::OnMessageLoginResponse, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  RobotApp::GetInstance()->SetMessageCallback(::protocol::MESSAGE_COMBAT_RESPONSE,
      std::bind(&MessageHandler::OnMessageCombatResponse, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  RobotApp::GetInstance()->SetMessageCallback(::protocol::MESSAGE_COMBAT_BEGIN_SYNC,
      std::bind(&MessageHandler::OnMessageCombatBeginSync, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  RobotApp::GetInstance()->SetMessageCallback(::protocol::MESSAGE_COMBAT_ACTION_REQUEST,
      std::bind(&MessageHandler::OnMessageCombatActionResponse, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  RobotApp::GetInstance()->SetMessageCallback(::protocol::MESSAGE_COMBAT_ACTION_SYNC,
      std::bind(&MessageHandler::OnMessageCombatActionSync, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  RobotApp::GetInstance()->SetMessageCallback(::protocol::MESSAGE_COMBAT_SETTLEMENT_SYNC,
      std::bind(&MessageHandler::OnMessageCombatSettlementSync, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
}

void MessageHandler::ResetHandlers() {
  RobotApp::GetInstance()->ResetMessageCallback(::protocol::MESSAGE_LOGIN_RESPONSE);
  RobotApp::GetInstance()->ResetMessageCallback(::protocol::MESSAGE_COMBAT_RESPONSE);
  RobotApp::GetInstance()->ResetMessageCallback(::protocol::MESSAGE_COMBAT_BEGIN_SYNC);
  RobotApp::GetInstance()->ResetMessageCallback(::protocol::MESSAGE_COMBAT_ACTION_RESPONSE);
  RobotApp::GetInstance()->ResetMessageCallback(::protocol::MESSAGE_COMBAT_ACTION_SYNC);
  RobotApp::GetInstance()->ResetMessageCallback(::protocol::MESSAGE_COMBAT_SETTLEMENT_SYNC);
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

void MessageHandler::OnMessageCombatBeginSync(Actor *actor,
    const char *data, int size) {
  ::protocol::MessageCombatBeginSync message;
  if (message.ParseFromArray(data, size) == false) {
    MYSYA_ERROR("MessageCombatBeginSync::ParseFromArray() failed.");
    return;
  }

  MYSYA_DEBUG("<<<<<<< MessageCombatBeginSync -------");
  message.PrintDebugString();
  MYSYA_DEBUG("------- MessageCombatBeginSync >>>>>>>");
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

void MessageHandler::OnMessageCombatActionSync(Actor *actor, const char *data, int size) {
  ::protocol::MessageCombatActionSync message;
  if (message.ParseFromArray(data, size) == false) {
    MYSYA_ERROR("MessageCombatActionSync::ParseFromArray() failed.");
    return;
  }

  MYSYA_DEBUG("<<<<<<< MessageCombatActionSync -------");
  message.PrintDebugString();
  MYSYA_DEBUG("------- MessageCombatActionSync >>>>>>>");
}

void MessageHandler::OnMessageCombatSettlementSync(Actor *actor, const char *data, int size) {
  ::protocol::MessageCombatSettlementSync message;
  if (message.ParseFromArray(data, size) == false) {
    MYSYA_ERROR("MessageCombatSettlementSync::ParseFromArray() failed.");
    return;
  }

  MYSYA_DEBUG("<<<<<<< MessageCombatSettlementSync -------");
  message.PrintDebugString();
  MYSYA_DEBUG("------- MessageCombatSettlementSync >>>>>>>");
}

}  // namespace robot
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
