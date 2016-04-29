#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_WARRIOR_FIELD_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_WARRIOR_FIELD_H

#include <mysya/util/class_util.h>

#include "tutorial/orcas/protocol/cc/warrior.pb.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class WarriorField {
 public:
  WarriorField();
  ~WarriorField();

  bool Initialize(uint64_t host_id, int32_t camp_id,
      const ::protocol::WarriorDescription &description);
  void Finalize();

  int32_t GetId() const;

  uint64_t GetHostId() const;
  void SetHostId(uint64_t value);

  int32_t GetCampId() const;
  void SetCampId(int32_t value);

  ::protocol::WarriorFields &GetFields();
  ::protocol::WarriorJuniorFields &GetJuniorFields();
  ::protocol::WarriorSeniorFields &GetSeniorFields();

 private:
  void GenerateFields();

  ::protocol::WarriorFields fields_;
  ::protocol::WarriorJuniorFields junior_fields_;
  ::protocol::WarriorSeniorFields senior_fields_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(WarriorField);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_WARRIOR_FIELD_H
