#include "tutorial/orcas/gateway/server/combat_message_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/transport_channel.h"
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

using namespace ::tutorial::orcas::combat;

CombatMessageHandler::CombatMessageHandler(AppServer *host)
  : host_(host) {}
CombatMessageHandler::~CombatMessageHandler() {}

void CombatMessageHandler::SetMessageHandlers() {
  this->host_->GetCombatClients().GetMessageDispatcher()->SetMessageCalback(
      protocol::MessageCombatDeployResponse().GetTypeName(), std::bind(
        &CombatMessageHandler::OnMessageCombatDeployResponse, this,
        std::placeholders::_1, std::placeholders::_2));
  this->host_->GetCombatClients().GetMessageDispatcher()->SetMessageCalback(
      protocol::MessageCombatConnectArgentResponse().GetTypeName(), std::bind(
        &CombatMessageHandler::OnMessageCombatConnectArgentResponse, this,
        std::placeholders::_1, std::placeholders::_2));
  this->host_->GetCombatClients().GetMessageDispatcher()->SetMessageCalback(
      protocol::MessageCombatBeginResponse().GetTypeName(), std::bind(
        &CombatMessageHandler::OnMessageCombatBeginResponse, this,
        std::placeholders::_1, std::placeholders::_2));
}

void CombatMessageHandler::ResetMessageHandlers() {
  this->host_->GetCombatClients().GetMessageDispatcher()->ResetMessageCallback(
      protocol::MessageCombatDeployResponse().GetTypeName());
  this->host_->GetCombatClients().GetMessageDispatcher()->ResetMessageCallback(
      protocol::MessageCombatConnectArgentResponse().GetTypeName());
  this->host_->GetCombatClients().GetMessageDispatcher()->ResetMessageCallback(
      protocol::MessageCombatBeginResponse().GetTypeName());
}

void CombatMessageHandler::OnMessageCombatDeployResponse(
    TransportChannel *channel, const ProtoMessage *data) {
  client::CombatSession *session = (client::CombatSession *)channel;
  protocol::MessageCombatDeployResponse *message =
    (protocol::MessageCombatDeployResponse *)data;

  Combat *combat = CombatManager::GetInstance()->GetPending(message->host_id());
  if (combat == NULL) {
    MYSYA_ERROR("CombatManager::GetPending(%d) failed.", message->host_id());
    return;
  }

  CombatActor *left_combat_actor = combat->GetLeft();
  CombatActor *right_combat_actor = combat->GetRight();

  if (left_combat_actor == NULL || right_combat_actor == NULL) {
    MYSYA_ERROR("left_combat_actor/right_combat_actor is null.");
    return;
  }

  if (message->result_type() != protocol::COMBAT_DEPLOY_RESULT_TYPE_COMPLETE) {
    ::protocol::MessageCombatResponse response;
    response.set_result(false);

    if (left_combat_actor->GetActor() != NULL) {
      left_combat_actor->GetActor()->SendMessage(
          ::protocol::MESSAGE_COMBAT_RESPONSE, response);
    }

    if (right_combat_actor->GetActor() != NULL) {
      right_combat_actor->GetActor()->SendMessage(
          ::protocol::MESSAGE_COMBAT_RESPONSE, response);
    }

    CombatManager::GetInstance()->Deallocate(combat);

    return;
  }

  combat->SetCombatArgentId(message->combat_id());
  combat->SetCombatServerId(session->GetServerId());

  CombatManager::GetInstance()->RemovePending(combat->GetId());
  CombatManager::GetInstance()->AddCombat(combat);

  // 发送连接请求
  protocol::MessageCombatConnectArgentRequest connect_message;
  connect_message.set_combat_id(message->combat_id());
  connect_message.set_role_argent_id(left_combat_actor->GetCombatArgentId());
  session->SendMessage(connect_message);
  connect_message.set_role_argent_id(right_combat_actor->GetCombatArgentId());
  session->SendMessage(connect_message);

  MYSYA_DEBUG("MessageCombatDeployResponse success.");
}

void CombatMessageHandler::OnMessageCombatConnectArgentResponse(
    TransportChannel *channel, const ProtoMessage *data) {
  client::CombatSession *session = (client::CombatSession *)channel;
  protocol::MessageCombatConnectArgentResponse *message =
    (protocol::MessageCombatConnectArgentResponse *)data;

  Combat *combat = CombatManager::GetInstance()->GetCombat(
      session->GetServerId(), message->combat_id());
  if (combat == NULL) {
    MYSYA_ERROR("CombatManager::GetCombat(%d, %d) failed.",
        session->GetServerId(), message->combat_id());
    return;
  }

  CombatActor *left_combat_actor = combat->GetLeft();
  CombatActor *right_combat_actor = combat->GetRight();

  if (left_combat_actor == NULL || right_combat_actor == NULL) {
    MYSYA_ERROR("left_combat_actor/right_combat_actor is null.");
    return;
  }

  if (message->ret_code() != protocol::MessageCombatConnectArgentResponse::ERROR_CODE_COMPLETE) {
    ::protocol::MessageCombatResponse response;
    response.set_result(false);

    if (left_combat_actor->GetActor() != NULL) {
      left_combat_actor->GetActor()->SendMessage(
          ::protocol::MESSAGE_COMBAT_RESPONSE, response);
    }

    if (right_combat_actor->GetActor() != NULL) {
      right_combat_actor->GetActor()->SendMessage(
          ::protocol::MESSAGE_COMBAT_RESPONSE, response);
    }

    CombatManager::GetInstance()->Deallocate(combat);

    return;
  }

  combat->SetConnectedNum(combat->GetConnectedNum() + 1);
  if (combat->GetConnectedNum() >= 2) {
    protocol::MessageCombatBeginRequest begin_message;
    begin_message.set_combat_id(message->combat_id());
    session->SendMessage(begin_message);
  }
}

void CombatMessageHandler::OnMessageCombatBeginResponse(
    TransportChannel *channel, const ProtoMessage *data) {
  client::CombatSession *session = (client::CombatSession *)channel;
  protocol::MessageCombatBeginResponse *message =
    (protocol::MessageCombatBeginResponse *)data;

  Combat *combat = CombatManager::GetInstance()->GetCombat(
      session->GetServerId(), message->combat_id());
  if (combat == NULL) {
    MYSYA_ERROR("CombatManager::GetCombat(%d, %d) failed.",
        session->GetServerId(), message->combat_id());
    return;
  }

  CombatActor *left_combat_actor = combat->GetLeft();
  CombatActor *right_combat_actor = combat->GetRight();

  if (left_combat_actor == NULL || right_combat_actor == NULL) {
    MYSYA_ERROR("left_combat_actor/right_combat_actor is null.");
    return;
  }

  ::protocol::MessageCombatResponse response;

  if (message->ret_code() != protocol::MessageCombatConnectArgentResponse::ERROR_CODE_COMPLETE) {
    response.set_result(false);

    if (left_combat_actor->GetActor() != NULL) {
      left_combat_actor->GetActor()->SendMessage(
          ::protocol::MESSAGE_COMBAT_RESPONSE, response);
    }

    if (right_combat_actor->GetActor() != NULL) {
      right_combat_actor->GetActor()->SendMessage(
          ::protocol::MESSAGE_COMBAT_RESPONSE, response);
    }

    CombatManager::GetInstance()->Deallocate(combat);
  } else {
    response.set_result(true);
    response.set_map_id(combat->GetMapId());
    *response.mutable_status_image() = message->status_image();

    response.set_host_id(left_combat_actor->GetCombatArgentId());
    response.set_camp_id(left_combat_actor->GetCampId());
    if (left_combat_actor->GetActor() != NULL) {
      left_combat_actor->GetActor()->SendMessage(
          ::protocol::MESSAGE_COMBAT_RESPONSE, response);
    }

    response.set_host_id(right_combat_actor->GetCombatArgentId());
    response.set_camp_id(right_combat_actor->GetCampId());
    if (right_combat_actor->GetActor() != NULL) {
      right_combat_actor->GetActor()->SendMessage(
          ::protocol::MESSAGE_COMBAT_RESPONSE, response);
    }
  }
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
