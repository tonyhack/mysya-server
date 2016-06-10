#include "tutorial/orcas/combat/server/combat_message_handler.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/combat/transport_channel.h"
#include "tutorial/orcas/combat/protocol/cc/combat_message.pb.h"
#include "tutorial/orcas/combat/server/app_server.h"
#include "tutorial/orcas/combat/server/app_session.h"
#include "tutorial/orcas/combat/server/combat_building_field.h"
#include "tutorial/orcas/combat/server/combat_building_field_pool.h"
#include "tutorial/orcas/combat/server/combat_field.h"
#include "tutorial/orcas/combat/server/combat_field_manager.h"
#include "tutorial/orcas/combat/server/combat_role_field.h"
#include "tutorial/orcas/combat/server/combat_role_field_manager.h"
#include "tutorial/orcas/combat/server/event/cc/event.pb.h"
#include "tutorial/orcas/combat/server/event/cc/event_combat.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

using namespace ::tutorial::orcas::combat::protocol;

CombatMessageHandler::CombatMessageHandler(AppServer *app_server)
  : app_server_(app_server) {}
CombatMessageHandler::~CombatMessageHandler() {}

void CombatMessageHandler::SetMessageHandlers() {
  this->app_server_->GetMessageDispatcher()->SetMessageCallback(
      MessageCombatDeployRequest().GetTypeName(),
      std::bind(&CombatMessageHandler::OnMessageCombatDeployRequest,
        this, std::placeholders::_1, std::placeholders::_2));
  this->app_server_->GetMessageDispatcher()->SetMessageCallback(
      MessageCombatConnectArgentRequest().GetTypeName(),
      std::bind(&CombatMessageHandler::OnMessageCombatConnectArgentRequest,
        this, std::placeholders::_1, std::placeholders::_2));
  this->app_server_->GetMessageDispatcher()->SetMessageCallback(
      MessageCombatReconnectRequest().GetTypeName(),
      std::bind(&CombatMessageHandler::OnMessageCombatReconnectRequest,
        this, std::placeholders::_1, std::placeholders::_2));
  this->app_server_->GetMessageDispatcher()->SetMessageCallback(
      MessageCombatBeginRequest().GetTypeName(),
      std::bind(&CombatMessageHandler::OnMessageCombatBeginRequest,
        this, std::placeholders::_1, std::placeholders::_2));
  this->app_server_->GetMessageDispatcher()->SetMessageCallback(
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
      MessageCombatReconnectRequest().GetTypeName());
  this->app_server_->GetMessageDispatcher()->ResetMessageCallback(
      MessageCombatBeginRequest().GetTypeName());
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

  MYSYA_DEBUG("MessageCombatDeployRequest host_id(%d).", message->host_id());

  CombatField *combat_field = CombatFieldManager::GetInstance()->Allocate();
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Allocate() failed.");
    SendMessageCombatDeployResponse(session, message->host_id(),
        COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
    return;
  }

  const CombatInitialData &initial_data = message->combat_initial_data();

  if (combat_field->Initialize(initial_data.map_id(), initial_data.max_time(),
        this->app_server_, session) == false) {
    MYSYA_ERROR("CombatField::Initialize() failed.");
    CombatFieldManager::GetInstance()->Deallocate(combat_field);
    SendMessageCombatDeployResponse(session, message->host_id(),
        COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
    return;
  }

  if (CombatFieldManager::GetInstance()->Add(combat_field) == false) {
    MYSYA_ERROR("CombatFieldManager::Add() failed.");
    combat_field->Finalize();
    CombatFieldManager::GetInstance()->Deallocate(combat_field);
    SendMessageCombatDeployResponse(session, message->host_id(),
        COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
    return;
  }

  // combat camps.
  for (int i = 0; i < initial_data.camp_size(); ++i) {
    const CombatCampData &camp_data = initial_data.camp(i);

    // combat roles.
    for (int j = 0; j < camp_data.role_size(); ++j) {
      const CombatRoleData &role_data = camp_data.role(j);
      CombatRoleField *role_field = CombatRoleFieldManager::GetInstance()->Allocate();
      if (role_field == NULL) {
        MYSYA_ERROR("CombatRoleFieldManager::Allocate() failed.");
        CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
        combat_field->Finalize();
        CombatFieldManager::GetInstance()->Deallocate(combat_field);
        SendMessageCombatDeployResponse(session, message->host_id(),
            COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
        return;
      }

      if (role_field->Initialize(role_data.argent_id(), role_data.name(),
            this->app_server_) == false) {
        MYSYA_ERROR("CombatRoleField::Initialize() failed.");
        CombatRoleFieldManager::GetInstance()->Deallocate(role_field);
        CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
        combat_field->Finalize();
        CombatFieldManager::GetInstance()->Deallocate(combat_field);
        SendMessageCombatDeployResponse(session, message->host_id(),
            COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
        return;
      }

      role_field->SetCampId(camp_data.id());
      role_field->SetFoodMax(camp_data.max_food());
      role_field->SetFood(camp_data.init_food());
      role_field->SetElixirMax(camp_data.max_elixir());
      role_field->SetElixir(camp_data.init_elixir());

      if (CombatRoleFieldManager::GetInstance()->Add(role_field) == false) {
        MYSYA_ERROR("CombatRoleFieldManager::Add() failed.");
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
        role_field->AddWarriorDescription(role_data.warrior(k));
      }

      // combat role's building.
      for (int k = 0; k < role_data.building_size(); ++k) {
        const ::protocol::BuildingDescription &building_description =
          role_data.building(k);

        CombatBuildingField *building = CombatBuildingFieldPool::GetInstance()->Allocate();
        if (building == NULL) {
          MYSYA_ERROR("CombatBuildingFieldPool::Allocate() failed.");
          role_field->Finalize();
          CombatRoleFieldManager::GetInstance()->Deallocate(role_field);
          CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
          combat_field->Finalize();
          CombatFieldManager::GetInstance()->Deallocate(combat_field);
          SendMessageCombatDeployResponse(session, message->host_id(),
              COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
          return;
        }

        if (building->Initialize(combat_field->AllocateId(), combat_field,
              role_field->GetArgentId(), camp_data.id(), building_description) == false) {
          MYSYA_ERROR("CombatBuildingFieldPool::Allocate() failed.");
          CombatBuildingFieldPool::GetInstance()->Deallocate(building);
          role_field->Finalize();
          CombatRoleFieldManager::GetInstance()->Deallocate(role_field);
          CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
          combat_field->Finalize();
          CombatFieldManager::GetInstance()->Deallocate(combat_field);
          SendMessageCombatDeployResponse(session, message->host_id(),
              COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
          return;
        }

        combat_field->AddBuilding(building);
        role_field->SetBuildingNum(role_field->GetBuildingNum() + 1);
      }

      combat_field->AddRole(role_field->GetArgentId());
    }

    // combat buidings.
    for (int j = 0; j < camp_data.building_size(); ++j) {
      const ::protocol::BuildingDescription &building_description =
        camp_data.building(j);

      CombatBuildingField *building = CombatBuildingFieldPool::GetInstance()->Allocate();
      if (building == NULL) {
        MYSYA_ERROR("CombatBuildingFieldPool::Allocate() failed.");
        CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
        combat_field->Finalize();
        CombatFieldManager::GetInstance()->Deallocate(combat_field);
        SendMessageCombatDeployResponse(session, message->host_id(),
            COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
        return;
      }

      if (building->Initialize(combat_field->AllocateId(), combat_field, 0,
            camp_data.id(), building_description) == false) {
        MYSYA_ERROR("CombatBuildingField::Initialize() failed.");
        CombatBuildingFieldPool::GetInstance()->Deallocate(building);
        CombatFieldManager::GetInstance()->Remove(combat_field->GetId());
        combat_field->Finalize();
        CombatFieldManager::GetInstance()->Deallocate(combat_field);
        SendMessageCombatDeployResponse(session, message->host_id(),
            COMBAT_DEPLOY_RESULT_TYPE_FAILURE);
        return;
      }

      combat_field->AddBuilding(building);
    }
  }

  MessageCombatDeployResponse message_response;
  message_response.set_host_id(message->host_id());
  message_response.set_result_type(COMBAT_DEPLOY_RESULT_TYPE_COMPLETE);
  message_response.set_combat_id(combat_field->GetId());
  session->SendMessage(message_response);
}

void CombatMessageHandler::OnMessageCombatConnectArgentRequest(
    ::tutorial::orcas::combat::TransportChannel *channel,
    const ::google::protobuf::Message *message_pb) {
  AppSession *session = (AppSession *)channel;
  const MessageCombatConnectArgentRequest *message =
    (const MessageCombatConnectArgentRequest *)message_pb;

  MessageCombatConnectArgentResponse response_message;
  response_message.set_role_argent_id(message->role_argent_id());
  response_message.set_combat_id(message->combat_id());

  CombatRoleField *role_field =
    CombatRoleFieldManager::GetInstance()->Get(message->role_argent_id());
  if (role_field == NULL) {
    MYSYA_ERROR("CombatRoleFieldManager::Get(%lu) failed.", message->role_argent_id());
    response_message.set_ret_code(MessageCombatConnectArgentResponse::ERROR_CODE_FAILURE);
    session->SendMessage(response_message);
    return;
  }

  role_field->SetAppSession(session);

  response_message.set_ret_code(MessageCombatConnectArgentResponse::ERROR_CODE_COMPLETE);
  session->SendMessage(response_message);
}

void CombatMessageHandler::OnMessageCombatReconnectRequest(
    ::tutorial::orcas::combat::TransportChannel *channel,
    const ::google::protobuf::Message *message_pb) {
  AppSession *session = (AppSession *)channel;
  const MessageCombatReconnectRequest *message =
    (const MessageCombatReconnectRequest *)message_pb;

  MessageCombatReconnectResponse response_message;
  response_message.set_role_argent_id(message->role_argent_id());
  response_message.set_combat_id(message->combat_id());

  CombatRoleField *role_field = 
    CombatRoleFieldManager::GetInstance()->Get(message->role_argent_id());
  if (role_field == NULL) {
    MYSYA_ERROR("CombatRoleFieldManager::Get(%lu) failed.", message->role_argent_id());
    response_message.set_ret_code(MessageCombatReconnectResponse::ERROR_CODE_FAILURE);
    return;
  }

  CombatField *combat_field = CombatFieldManager::GetInstance()->Get(message->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", message->combat_id());
    response_message.set_ret_code(MessageCombatReconnectResponse::ERROR_CODE_FAILURE);
    return;
  }

  role_field->SetAppSession(session);

  response_message.set_ret_code(MessageCombatReconnectResponse::ERROR_CODE_COMPLETE);
  combat_field->ExportStatusImage(*response_message.mutable_status_image());
  session->SendMessage(response_message);
}

static void SendMessageCombatBeginResponse(AppSession *session,
    int32_t combat_id, int32_t ret_code) {
  MessageCombatBeginResponse message;
  message.set_ret_code(ret_code);
  message.set_combat_id(combat_id);
  session->SendMessage(message);
}

void CombatMessageHandler::OnMessageCombatBeginRequest(
    ::tutorial::orcas::combat::TransportChannel *channel,
    const ::google::protobuf::Message *message_pb) {
  AppSession *session = (AppSession *)channel;
  const MessageCombatBeginRequest *message = (const MessageCombatBeginRequest *)message_pb;

  CombatField *combat_field = CombatFieldManager::GetInstance()->Get(message->combat_id());
  if (combat_field == NULL) {
    MYSYA_ERROR("CombatFieldManager::Get(%d) failed.", message->combat_id());
    SendMessageCombatBeginResponse(session, message->combat_id(),
        MessageCombatBeginResponse::ERROR_CODE_FAILURE);
    return;
  }

  combat_field->SetSettleTimer();
  combat_field->SetResourceRecoverTimer();
  combat_field->SetBeginTimestamp(this->app_server_->GetTimestamp());

  combat_field->AllocateBuildingSupply();

  event::EventCombatBegin combat_event;
  combat_event.set_combat_id(combat_field->GetId());
  this->app_server_->GetEventDispatcher()->Dispatch(event::EVENT_COMBAT_BEGIN, &combat_event);

  // response status image.
  MessageCombatBeginResponse response_message;
  response_message.set_ret_code(MessageCombatBeginResponse::ERROR_CODE_SUCCESS);
  response_message.set_combat_id(message->combat_id());
  combat_field->ExportStatusImage(*response_message.mutable_status_image());
  session->SendMessage(response_message);
}

void CombatMessageHandler::OnMessageCombatArgentRequest(
    ::tutorial::orcas::combat::TransportChannel *channel,
    const ::google::protobuf::Message *message_pb) {
  const MessageCombatArgentRequest *message = (const MessageCombatArgentRequest *)message_pb;

  CombatRoleField *role = CombatRoleFieldManager::GetInstance()->Get(
      message->role_argent_id());
  if (role == NULL) {
    MYSYA_ERROR("CombatRoleFieldManager::Get(%lu) failed.", message->role_argent_id());
    return;
  }

  this->app_server_->GetUserMessageDispatcher()->Dispatch(message->type(),
      role, message->data().data(), message->data().size());
}

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
