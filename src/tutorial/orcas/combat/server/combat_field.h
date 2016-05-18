#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_FIELD_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_FIELD_H

#include <stdint.h>

#include <map>
#include <set>
#include <unordered_map>

#include <mysya/util/class_util.h>
#include <mysya/util/timestamp.h>

#include "tutorial/orcas/protocol/cc/combat.pb.h"

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppServer;
class AppSession;
class CombatWarriorField;
class CombatBuildingField;

class CombatField {
 public:
  typedef std::set<uint64_t> CombatRoleFieldSet;
  typedef std::unordered_map<int32_t, CombatWarriorField *> WarriorFieldHashmap;
  typedef std::map<int32_t, CombatBuildingField *> BuildingFieldMap;

  CombatField();
  ~CombatField();

  bool Initialize(int32_t map_id, AppServer *app_server,
      AppSession *session);
  void Finalize();

  int32_t GetId() const;
  void SetId(int32_t value);

  int32_t GetMapId() const;
  int32_t AllocateId();

  const ::mysya::util::Timestamp &GetBeginTimestamp() const;
  void SetBeginTimestamp(const ::mysya::util::Timestamp &value);

  void AddRole(uint64_t role_argent_id);
  void RemoveRole(uint64_t role_argent_id);
  const CombatRoleFieldSet &GetRoles() const;

  void AddBuilding(CombatBuildingField *building);
  CombatBuildingField *RemoveBuilding(int32_t building_id);
  CombatBuildingField *GetBuilding(int32_t building_id);
  const BuildingFieldMap &GetBuildings() const;

  void AddWarrior(CombatWarriorField *warrior);
  CombatWarriorField *RemoveWarrior(int32_t warrior_id);
  CombatWarriorField *GetWarrior(int32_t warrior_id);
  const WarriorFieldHashmap &GetWarriors() const;

  void ResetAppSession();
  int SendMessage(const ::google::protobuf::Message &message);

  int BroadcastMessage(int type, const ::google::protobuf::Message &message);

  void PushAction(const ::protocol::CombatAction &action);
  const ::protocol::CombatActionSequence &GetActions() const;

  void ExportStatusImage(::protocol::CombatStatusImage &image) const;

 private:
  int32_t id_;
  int32_t map_id_;
  int32_t id_alloctor_;
  ::mysya::util::Timestamp begin_timestamp_;

  CombatRoleFieldSet roles_;
  CombatRoleFieldSet argent_roles_;

  BuildingFieldMap buildings_;
  WarriorFieldHashmap warriors_;

  ::protocol::CombatActionSequence actions_;

  AppSession *app_session_;
  AppServer *app_server_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatField);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_FIELD_H
