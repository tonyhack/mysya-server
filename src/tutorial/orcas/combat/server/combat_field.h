#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_FIELD_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_FIELD_H

#include <stdint.h>

#include <set>

#include <mysya/util/class_util.h>

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

class CombatField {
 public:
  typedef std::set<uint64_t> CombatRoleFieldSet;

  CombatField();
  ~CombatField();

  bool Initialize(AppSession *session);
  void Finalize();

  int32_t GetId() const;
  void SetId(int32_t value);

  void AddRole(uint64_t role_argent_id);
  void RemoveRole(uint64_t role_argent_id);
  const CombatRoleFieldSet &GetRoles() const;

  void ResetAppSession();
  int SendMessage(const ::google::protobuf::Message &message);

 private:
  int32_t id_;
  CombatRoleFieldSet roles_;

  AppSession *app_session_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(CombatField);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_COMBAT_FIELD_H
