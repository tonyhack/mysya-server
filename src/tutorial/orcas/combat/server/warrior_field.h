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

  void SetDescription(const ::protocol::WarriorDescription &value);
  const ::protocol::WarriorDescription &GetDescription() const;

 private:
  uint64_t host_id_;
  ::protocol::WarriorDescription description_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(WarriorField);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_WARRIOR_FIELD_H
