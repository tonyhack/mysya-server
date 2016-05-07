#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_ROLE_FIELD_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_ROLE_FIELD_H

#include <map>

#include <mysya/util/class_util.h>

#include "tutorial/orcas/protocol/cc/warrior.pb.h"

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppSession;
class CombatField;

class CombatRoleField {
 public:
  typedef std::map<int32_t, ::protocol::WarriorDescription> WarriorDescriptionMap;

  CombatRoleField();
  ~CombatRoleField();

  bool Initialize(uint64_t argent_id);
  void Finalize();

  uint64_t GetArgentId() const;
  void SetArgentId(uint64_t value);

  int32_t GetCampId() const;
  void SetCampId(int32_t value);

  CombatField *GetCombatField();
  void SetCombatField(CombatField *value);

  void AddWarriorDescription(const ::protocol::WarriorDescription &warroir);
  const ::protocol::WarriorDescription *GetWarriorDescription(int32_t id) const;

  void SetAppSession(AppSession *session);
  void ResetAppSession();
  int SendMessage(const ::google::protobuf::Message &message);

 private:
  uint64_t argent_id_;
  int32_t camp_id_;
  CombatField *combat_field_;
  WarriorDescriptionMap warrior_descriptions_;

  AppSession *app_session_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatRoleField);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_ROLE_FIELD_H
