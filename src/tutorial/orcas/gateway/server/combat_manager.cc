#include "tutorial/orcas/gateway/server/combat_manager.h"

#include <google/protobuf/message.h>
#include <mysya/ioevent/logger.h>

#include "tutorial/orcas/gateway/server/actor.h"
#include "tutorial/orcas/gateway/server/app_server.h"
#include "tutorial/orcas/gateway/server/combat_actor.h"
#include "tutorial/orcas/combat/client/combat_session.h"
#include "tutorial/orcas/combat/client/combat_sessions.h"
#include "tutorial/orcas/combat/protocol/cc/combat_message.pb.h"
#include "tutorial/orcas/gateway/server/map_config.h"

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

MYSYA_SINGLETON_IMPL(CombatManager);

Combat::Combat()
  : id_(0), combat_server_id_(0),
    combat_argent_id_(0), host_(NULL),
    al_(NULL), ar_(NULL),
    map_id_(0), connected_num_(0),
    max_time_(0) {}
Combat::~Combat() {
  if (this->al_ != NULL) { this->al_->SetCombat(NULL); }
  if (this->ar_ != NULL) { this->ar_->SetCombat(NULL); }
}

void Combat::SetId(int32_t id) {
  this->id_ = id;
}

int32_t Combat::GetId() const {
  return this->id_;
}

void Combat::SetHost(AppServer *host) {
  this->host_ = host;
}

void Combat::SetMapId(int32_t id) {
  this->map_id_ = id;
}

int32_t Combat::GetMapId() const {
  return this->map_id_;
}

void Combat::SetCombatServerId(int32_t id) {
  this->combat_server_id_ = id;
}

int32_t Combat::GetCombatServerId() const {
  return this->combat_server_id_;
}

void Combat::SetCombatArgentId(int32_t id) {
  this->combat_argent_id_ = id;
}

int32_t Combat::GetCombatArgentId() const {
  return this->combat_argent_id_;
}

void Combat::SetLeft(CombatActor *actor) {
  this->al_ = actor;
  this->al_->SetCombat(this);
}

CombatActor *Combat::GetLeft() {
  return this->al_;
}

void Combat::SetRight(CombatActor *actor) {
  this->ar_ = actor;
  this->ar_->SetCombat(this);
}

CombatActor *Combat::GetRight() {
  return this->ar_;
}

int32_t Combat::GetConnectedNum() const {
  return this->connected_num_;
}

void Combat::SetConnectedNum(int value) {
  this->connected_num_ = value;
}

int32_t Combat::GetMaxTime() const {
  return this->max_time_;
}

void Combat::SetMaxTime(int32_t value) {
  this->max_time_ = value;
}

CombatActor *Combat::GetActorByArgentId(uint64_t role_argent_id) {
  if (this->al_ != NULL && this->al_->GetCombatArgentId() == role_argent_id) {
    return this->al_;
  }
  if (this->ar_ != NULL && this->ar_->GetCombatArgentId() == role_argent_id) {
    return this->ar_;
  }

  return NULL;
}

Combat::CombatServerArgentKey Combat::GetArgentKey() const {
  return CombatServerArgentKey(this->GetCombatServerId(),
      this->GetCombatArgentId());
}

void Combat::SendMessage(const ::google::protobuf::Message &message) {
  ::tutorial::orcas::combat::client::CombatSession *session =
    this->host_->GetCombatClients().GetSessionByServerId(this->combat_server_id_);
  if (session == NULL) {
    MYSYA_ERROR("CombatSessions::GetSessionByServerId(%d) failed.",
        this->combat_server_id_);
    return;
  }

  session->SendMessage(message);
}

void Combat::BroadcastMessage(int type, const std::string &data) {
  if (this->al_ != NULL && this->al_->GetActor() != NULL) {
    this->al_->GetActor()->SendMessage(type, data);
  } else {
    MYSYA_ERROR("al actor is null.");
  }

  if (this->ar_ != NULL && this->ar_->GetActor() != NULL) {
    this->ar_->GetActor()->SendMessage(type, data);
  } else {
    MYSYA_ERROR("ar actor is null.");
  }
}

void Combat::BroadcastMessage(int type, const ::google::protobuf::Message &message) {
  if (this->al_ != NULL && this->al_->GetActor() != NULL) {
    this->al_->GetActor()->SendMessage(type, message);
  } else {
    MYSYA_ERROR("al actor is null.");
  }

  if (this->ar_ != NULL && this->ar_->GetActor() != NULL) {
    this->ar_->GetActor()->SendMessage(type, message);
  } else {
    MYSYA_ERROR("ar actor is null.");
  }
}

CombatManager::CombatManager()
  : id_allocator_(0) {}
CombatManager::~CombatManager() {}

void CombatManager::SetHost(AppServer *host) {
  this->host_ = host;
}

Combat *CombatManager::Allocate() {
  Combat *combat = new (std::nothrow) Combat();
  if (combat == NULL) {
    return NULL;
  }

  combat->SetId(++this->id_allocator_);
  combat->SetHost(this->host_);
  this->AddPendiong(combat);

  return combat;
}

void CombatManager::Deallocate(Combat *combat) {
  this->pending_combats_.erase(combat->GetId());
  this->combats_.erase(combat->GetArgentKey());
  delete combat;
}

void CombatManager::AddPendiong(Combat *combat) {
  this->pending_combats_.insert(std::make_pair(combat->GetId(), combat));
}

Combat *CombatManager::GetPending(int32_t id) {
  Combat *combat = NULL;

  CombatHashmap::iterator iter = this->pending_combats_.find(id);
  if (iter != this->pending_combats_.end()) {
    combat = iter->second;
  }

  return combat;
}

Combat *CombatManager::RemovePending(int32_t id) {
  Combat *combat = NULL;

  CombatHashmap::iterator iter = this->pending_combats_.find(id);
  if (iter != this->pending_combats_.end()) {
    combat = iter->second;
  }

  return combat;
}

void CombatManager::AddCombat(Combat *combat) {
  this->combats_.insert(std::make_pair(combat->GetArgentKey(), combat));
}

Combat *CombatManager::GetCombat(int32_t server_id, int32_t combat_argent_id) {
  Combat *combat = NULL;

  CombatArgentHashMap::iterator iter = this->combats_.find(
      CombatServerArgentKey(server_id, combat_argent_id));
  if (iter != this->combats_.end()) {
    combat = iter->second;
  }

  return combat;
}

Combat *CombatManager::RemoveCombat(int32_t server_id, int32_t combat_argent_id) {
  Combat *combat = NULL;

  CombatArgentHashMap::iterator iter = this->combats_.find(
      CombatServerArgentKey(server_id, combat_argent_id));
  if (iter != this->combats_.end()) {
    combat = iter->second;
    this->combats_.erase(iter);
  }

  return combat;
}

bool CombatManager::PushCombat(CombatActor *actor, int32_t map_id) {
  const MapConf *map_conf = MapConfig::GetInstance()->GetMapConf(map_id);
  if (map_conf == NULL) {
    MYSYA_ERROR("MapConfig::GetMapConf(%d) failed.", map_id);
    return false;
  }

  PendingCombatActorSet::iterator iter = this->pending_combat_actors_.begin();
  if (iter == this->pending_combat_actors_.end()) {
    this->AddPendingCombatActor(actor);
    return true;
  }

  CombatActor *left_actor = actor;
  CombatActor *right_actor = *iter;
  this->pending_combat_actors_.erase(iter);

  Combat *combat = this->Allocate();
  if (combat == NULL) {
    this->AddPendingCombatActor(right_actor);
    return false;
  }

  combat->SetMapId(map_id);
  combat->SetLeft(left_actor);
  combat->SetRight(right_actor);
  combat->SetMaxTime(CombatManager::kCombatMaxTime_);

  typedef CombatActor::WarriorDescriptionMap WarriorDescriptionMap;
  ::tutorial::orcas::combat::protocol::MessageCombatDeployRequest message;

  typedef MapConf::BuildingVectorMap BuildingVectorMap;
  typedef MapConf::BuildingVector BuildingVector;

  MYSYA_DEBUG("MessageCombatDeployRequest host_id(%d).", combat->GetId());

  message.set_host_id(combat->GetId());
  ::tutorial::orcas::combat::protocol::CombatInitialData *init_data =
    message.mutable_combat_initial_data();
  init_data->set_map_id(map_id);
  init_data->set_max_time(CombatManager::kCombatMaxTime_);
  init_data->set_combat_type(::tutorial::orcas::combat::protocol::COMBAT_TYPE_PVP);

  // 左方阵营
  left_actor->SetCampId(1);
  ::tutorial::orcas::combat::protocol::CombatCampData *l_camp_data =
    init_data->add_camp();
  l_camp_data->set_id(left_actor->GetCampId());
  // 左方角色
  ::tutorial::orcas::combat::protocol::CombatRoleData *l_role_data =
    l_camp_data->add_role();
  l_role_data->set_ai("");
  l_role_data->set_name(left_actor->GetName());
  l_role_data->set_argent_id(left_actor->GetCombatArgentId());
  // 左方初始兵种
  const WarriorDescriptionMap &l_warriors = left_actor->GetWarriors();
  for (WarriorDescriptionMap::const_iterator iter = l_warriors.begin();
      iter != l_warriors.end(); ++iter) {
    *l_role_data->add_warrior() = iter->second;
  }
  // 左方初始建筑
  BuildingVectorMap::const_iterator l_building_iter = map_conf->buildings_.find(
      left_actor->GetCampId());
  if (l_building_iter != map_conf->buildings_.end()) {
    for (BuildingVector::const_iterator iter = l_building_iter->second.begin();
        iter != l_building_iter->second.end(); ++iter) {
      const BuildingConf *building_conf = *iter;
      ::protocol::BuildingDescription *building = l_role_data->add_building();
      building->set_id(building_conf->id_);
      building->set_type(building_conf->type_);
      building->set_hp(building_conf->hp_);
      building->set_pos_x(building_conf->x_);
      building->set_pos_y(building_conf->y_);
    }
  }

  // 右方阵营
  right_actor->SetCampId(2);
  ::tutorial::orcas::combat::protocol::CombatCampData *r_camp_data =
    init_data->add_camp();
  r_camp_data->set_id(right_actor->GetCampId());
  // 右方角色
  ::tutorial::orcas::combat::protocol::CombatRoleData *r_role_data =
    r_camp_data->add_role();
  r_role_data->set_ai("");
  r_role_data->set_name(right_actor->GetName());
  r_role_data->set_argent_id(right_actor->GetCombatArgentId());
  // 右方初始兵种
  const WarriorDescriptionMap r_warriors = right_actor->GetWarriors();
  for (WarriorDescriptionMap::const_iterator iter = r_warriors.begin();
      iter != r_warriors.end(); ++iter) {
    *r_role_data->add_warrior() = iter->second;
  }
  // 右方初始建筑
  BuildingVectorMap::const_iterator r_building_iter = map_conf->buildings_.find(
      right_actor->GetCampId());
  if (r_building_iter != map_conf->buildings_.end()) {
    for (BuildingVector::const_iterator iter = r_building_iter->second.begin();
        iter != r_building_iter->second.end(); ++iter) {
      const BuildingConf *building_conf = *iter;
      ::protocol::BuildingDescription *building = r_role_data->add_building();
      building->set_id(building_conf->id_);
      building->set_type(building_conf->type_);
      building->set_hp(building_conf->hp_);
      building->set_pos_x(building_conf->x_);
      building->set_pos_y(building_conf->y_);
    }
  }

  // 获取战斗会话
  ::tutorial::orcas::combat::client::CombatSession *session =
    this->host_->GetCombatClients().GetSessionByServerId(1);
  if (session == NULL) {
    MYSYA_ERROR("CombatSessions::GetSessionByServerId(1) failed.");
    this->AddPendingCombatActor(right_actor);
    return false;
  }

  // 向战斗服务器请求部署战斗
  session->SendMessage(message);

  return true;
}

void CombatManager::Offline(CombatActor *actor) {
  this->RemovePendingCombatActor(actor);
}

void CombatManager::AddPendingCombatActor(CombatActor *actor) {
  this->pending_combat_actors_.insert(actor);
}

void CombatManager::RemovePendingCombatActor(CombatActor *actor) {
  this->pending_combat_actors_.erase(actor);
}

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial
