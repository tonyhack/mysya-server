#include "tutorial/orcas/gateway/server/user_message_handler.h"

#include <functional>

#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/client/combat_session.h"
#include "tutorial/orcas/combat/client/combat_sessions.h"
#include "tutorial/orcas/combat/protocol/cc/combat_message.pb.h"
#include "tutorial/orcas/gateway/server/actor.h"
#include "tutorial/orcas/gateway/server/app_server.h"
#include "tutorial/orcas/gateway/server/combat_actor.h"
#include "tutorial/orcas/gateway/server/combat_actor_manager.h"
#include "tutorial/orcas/gateway/server/combat_manager.h"
#include "tutorial/orcas/protocol/cc/message.pb.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

UserMessageHandler::UserMessageHandler(AppServer *host)
  : host_(host) {}
UserMessageHandler::~UserMessageHandler() {}

void UserMessageHandler::SetMessageHandlers() {
  this->host_->GetMessageDispatcher()->SetMessageCalback(::protocol::MESSAGE_LOGIN_REQUEST,
      std::bind(&UserMessageHandler::OnMessageLoginRequest, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  this->host_->GetMessageDispatcher()->SetMessageCalback(::protocol::MESSAGE_COMBAT_REQUEST,
      std::bind(&UserMessageHandler::OnMessageCombatRequest, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  this->host_->GetMessageDispatcher()->SetMessageCalback(::protocol::MESSAGE_COMBAT_ACTION_REQUEST,
      std::bind(&UserMessageHandler::OnMessageCombatActionRequest, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
}

void UserMessageHandler::ResetMessageHandlers() {
  this->host_->GetMessageDispatcher()->ResetMessageCallback(
      ::protocol::MESSAGE_LOGIN_REQUEST);
  this->host_->GetMessageDispatcher()->ResetMessageCallback(
      ::protocol::MESSAGE_COMBAT_REQUEST);
  this->host_->GetMessageDispatcher()->ResetMessageCallback(
      ::protocol::MESSAGE_COMBAT_ACTION_REQUEST);
}

static void SendMessageLoginResponse(Actor *actor, bool result) {
  ::protocol::MessageLoginResponse message;
  message.set_result(result);
  actor->SendMessage(::protocol::MESSAGE_LOGIN_RESPONSE, message);
}

void UserMessageHandler::OnMessageLoginRequest(Actor *actor,
    const char *data, int size) {
  ::protocol::MessageLoginRequest message;
  if (message.ParseFromArray(data, size) == false) {
    MYSYA_ERROR("MessageLoginRequest::ParseFromArray() failed.");
    SendMessageLoginResponse(actor, false);
    return;
  }

  if (actor->GetCombatActor() != NULL) {
    MYSYA_ERROR("Actor::GetCombatActor() exist.");
    SendMessageLoginResponse(actor, false);
    return;
  }

  CombatActor *combat_actor =
    CombatActorManager::GetInstance()->Get(message.name());
  if (combat_actor == NULL) {
    combat_actor = CombatActorManager::GetInstance()->Allocate(message.name());
    if (combat_actor == NULL) {
      MYSYA_ERROR("CombatActorManager::Allocate() exist.");
      SendMessageLoginResponse(actor, false);
      return;
    }
  }

  if (combat_actor->GetActor() != NULL) {
    MYSYA_ERROR("CombatActor::GetActor() exist.");
    SendMessageLoginResponse(actor, false);
    return;
  }

  combat_actor->SetActor(actor);
  actor->SetCombatActor(combat_actor);

  SendMessageLoginResponse(actor, true);
}

static void SendMessageCombatResponse(Actor *actor, bool result) {
  ::protocol::MessageCombatResponse message;
  message.set_result(result);
  actor->SendMessage(::protocol::MESSAGE_COMBAT_RESPONSE, message);
}

void UserMessageHandler::OnMessageCombatRequest(Actor *actor,
    const char *data, int size) {
  ::protocol::MessageCombatRequest message;
  if (message.ParseFromArray(data, size) == false) {
    MYSYA_ERROR("MessageCombatRequest::ParseFromArray() failed.");
    SendMessageCombatResponse(actor, false);
    return;
  }

  CombatActor *combat_actor = actor->GetCombatActor();
  if (combat_actor == NULL) {
    MYSYA_ERROR("Actor::GetCombatActor() failed.");
    SendMessageCombatResponse(actor, false);
    return;
  }

  // TODO: Magic number 1
  if (CombatManager::GetInstance()->PushCombat(combat_actor, 1) == false) {
    MYSYA_ERROR("CombatManager::Push() failed.");
    SendMessageCombatResponse(actor, false);
    return;
  }

  SendMessageCombatResponse(actor, true);
}

void UserMessageHandler::OnMessageCombatActionRequest(Actor *actor,
    const char *data, int size) {
  CombatActor *combat_actor = actor->GetCombatActor();
  if (combat_actor == NULL) {
    MYSYA_ERROR("Actor::GetCombatActor() failed.");
    return;
  }

  Combat *combat = combat_actor->GetCombat();
  if (combat == NULL) {
    MYSYA_ERROR("CombatActor::GetCombat() failed.");
    return;
  }

  ::tutorial::orcas::combat::client::CombatSession *session =
    this->host_->GetCombatClients().GetSessionByServerId(combat->GetCombatServerId());
  if (session == NULL) {
    MYSYA_ERROR("CombatSessions::GetSessionByServerId(%d) failed.",
        combat->GetCombatServerId());
    return;
  }

  ::tutorial::orcas::combat::protocol::MessageCombatArgentRequest message;
  message.set_argent_id(combat_actor->GetCombatArgentId());
  message.set_type(::protocol::MESSAGE_COMBAT_ACTION_REQUEST);
  message.set_data(data, size);
  session->SendMessage(message);
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
