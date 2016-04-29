#include "tutorial/orcas/combat/server/combat_message_handler.h"

#include <google/protobuf/message.h>

#include "tutorial/orcas/combat/transport_channel.h"
#include "tutorial/orcas/combat/protocol/cc/message.pb.h"
#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/app_session.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_role_field_manager.h"
#include "tutorial/orcas/combat/server/warrior_field.h"
#include "tutorial/orcas/combat/server/warrior_field_pool.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

using namespace ::tutorial::orcas::combat::protocol;

CombatMessageHandler::CombatMessageHandler(AppServer *app_server)
  : app_server_(app_server) {}
CombatMessageHandler::~CombatMessageHandler() {}

void CombatMessageHandler::SetMessageHandlers() {
  this->app_server_->GetMessageDispatcher()->SetMessageCalback(
      MessageCombatDeployRequest().GetTypeName(),
      std::bind(&CombatMessageHandler::OnMessageCombatDeployRequest,
        this, std::placeholders::_1, std::placeholders::_2));
  this->app_server_->GetMessageDispatcher()->SetMessageCalback(
      MessageCombatConnectArgentRequest().GetTypeName(),
      std::bind(&CombatMessageHandler::OnMessageCombatConnectArgentRequest,
        this, std::placeholders::_1, std::placeholders::_2));
  this->app_server_->GetMessageDispatcher()->SetMessageCalback(
      MessageCombatArgentRequest().GetTypeName(),
      std::bind(&CombatMessageHandler::OnMessageCombatArgentRequest,
        this, std::placeholders::_1, std::placeholders::_2));
}

void CombatMessageHandler::ResetMessageHandlers() {
  this->app_server_->GetMessageDispatcher()->ResetMessageCallback(
      MessageCombatDeployRequest().GetTypeName());
  this->app_server_->GetMessageDispatcher()->ResetMessageCallback(
      MessageCombatConnectArgentRequest().GetTypeName());
  this->app_server_->GetMessageDispatcher()->ResetMessageCallback(
      MessageCombatArgentRequest().GetTypeName());
}

static void SendMessageCombatDeployResponse(AppSession *session,
    int32_t host_id, int32_t result) {
  MessageCombatDeployResponse message;
  message.set_host_id(host_id);
  message.set_result_type(result);
  session->SendMessage(message);
}

void CombatMessageHandler::OnMessageCombatDeployRequest(
    ::tutorial::orcas::combat::TransportChannel *channel,
    const ::google::protobuf::Message *message_pb) {
  AppSession *session = (AppSession *)channel;
  const MessageCombatDeployRequest *message = (const MessageCombatDeployRequest *)message_pb;

  CombatField *combat_field = CombatFieldManager::GetInstance()->Allocate();
  if (combat_field == NULL) {
    SendMessageCombatDeployResponse(session, message->host_id(),
        COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
    return;
  }

  if (combat_field->Initialize(session) == false) {
    CombatFieldManager::GetInstance()->Deallocate(combat_field);
    SendMessageCombatDeployResponse(session, message->host_id(),
        COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
    return;
  }

  if (CombatFieldManager::GetInstance()->Add(combat_field) == false) {
    combat_field->Finalize();
    CombatFieldManager::GetInstance()->Deallocate(combat_field);
    SendMessageCombatDeployResponse(session, message->host_id(),
        COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
    return;
  }

  const CombatInitialData &initial_data = message->combat_initial_data();

  // combat camps.
  for (int i = 0; i < initial_data.camp_size(); ++i) {
    const CombatCampData &camp_data = initial_data.camp(i);

    // combat roles.
    for (int j = 0; j < camp_data.role_size(); ++j) {
      const CombatRoleData &role_data = camp_data.role(j);
      CombatRoleField *role_field = CombatRoleFieldManager::GetInstance()->Allocate();
      if (role_field == NULL) {
        CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
        combat_field->Finalize();
        CombatFieldManager::GetInstance()->Deallocate(combat_field);
        SendMessageCombatDeployResponse(session, message->host_id(),
            COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
        return;
      }

      if (role_field->Initialize(role_data.argent_id()) == false) {
        CombatRoleFieldManager::GetInstance()->Deallocate(role_field);
        CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
        combat_field->Finalize();
        CombatFieldManager::GetInstance()->Deallocate(combat_field);
        SendMessageCombatDeployResponse(session, message->host_id(),
            COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
        return;
      }

      if (CombatRoleFieldManager::GetInstance()->Add(role_field) == false) {
        role_field->Finalize();
        CombatRoleFieldManager::GetInstance()->Deallocate(role_field);
        CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
        combat_field->Finalize();
        CombatFieldManager::GetInstance()->Deallocate(combat_field);
        SendMessageCombatDeployResponse(session, message->host_id(),
            COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
        return;
      }

      // combat role's warroior.
      for (int k = 0; k < role_data.warrior_size(); ++k) {
        WarriorField *warrior = WarriorFieldPool::GetInstance()->Allocate();
        if (warrior == NULL) {
          CombatRoleFieldManager::GetInstance()->Remove(role_field->GetArgentId());
          role_field->Finalize();
          CombatRoleFieldManager::GetInstance()->Deallocate(role_field);
          CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
          combat_field->Finalize();
          CombatFieldManager::GetInstance()->Deallocate(combat_field);
          SendMessageCombatDeployResponse(session, message->host_id(),
              COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
          return;
        }
      }

      combat_field->AddRole(role_field->GetArgentId());
    }
  }

  MessageCombatDeployResponse message_response;
  message_response.set_host_id(message->host_id());
  message_response.set_result_type(COMBAT_DEPLOY_RESULT_TYPE_COMPLETE);
  message_response.set_combat_id(combat_field->GetId());
  session->SendMessage(message_response);
}

void CombatMessageHandler::OnMessageCombatArgentRequest(
    ::tutorial::orcas::combat::TransportChannel *channel,
    const ::google::protobuf::Message *message_pb) {
  // AppSession *session = (AppSession *)channel;
  // const MessageCombatArgentRequest *message = (const MessageCombatArgentRequest *)message_pb;
}

static void SendMessageCombatConnectArgentResponse(AppSession *session, int32_t ret_code) {
  MessageCombatConnectArgentResponse message;
  message.set_ret_code(ret_code);
  session->SendMessage(message);
}

void CombatMessageHandler::OnMessageCombatConnectArgentRequest(
    ::tutorial::orcas::combat::TransportChannel *channel,
    const ::google::protobuf::Message *message_pb) {
  AppSession *session = (AppSession *)channel;
  const MessageCombatConnectArgentRequest *message = (const MessageCombatConnectArgentRequest *)message_pb;

  CombatRoleField *role_field = CombatRoleFieldManager::GetInstance()->Get(message->argent_id());
  if (role_field == NULL) {
    SendMessageCombatConnectArgentResponse(session, MessageCombatConnectArgentResponse::ERROR_CODE_FAILURE);
    return;
  }

  role_field->SetAppSession(session);
  SendMessageCombatConnectArgentResponse(session, MessageCombatConnectArgentResponse::ERROR_CODE_COMPLETE);
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorialk
