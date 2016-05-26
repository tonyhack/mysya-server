#include "tutorial/orcas/combat/server/ai/auto_status_search.h"

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace ai {

AutoStatusSearch::AutoStatusSearch(Auto *host)
  : AutoStatus(host), timer_id_search_(-1) {}
AutoStatusSearch::~AutoStatusSearch() {}

void AutoStatusSearch::Start() {
  this->timer_id_search_ =
    AiApp::GetInstance()->GetHost()->StartTimer(kSearchExpireMsec_,
      std::bind(&AutoStatusSearch::OnTimerSearch, this, std::placehoder::_1));
}

void AutoStatusSearch::Stop() {
  AiApp::GetInstance()->GetHost()->StopTimer(this->timer_id_search_);
  this->timer_id_search_ = -1;
}

void AutoStatusSearch::OnEvent(int type, ProtoMessage *data) {}

void AutoStatusSearch::OnTimerSearch(int64_t timer_id) {
  CombatWarriorField *combat_warrior_field = this->host_->GetHost();
  if (combat_warrior_field == NULL) {
    MYSYA_ERROR("[AI] Auto::GetHost() failed.");
    return;
  }

  CombatRoleField *combat_role_field = combat_warrior_field->GetRoleField();
  if (combat_role_field == NULL) {
    MYSYA_ERROR("[AI] CombatWarriorField::GetRoleField() failed.");
    return;
  }

  CombatField *combat_field = combat_role_field->GetCombatField();
  if (combat_field == NULL) {
    MYSYA_ERROR("[AI] CombatRoleField::GetCombatField() failed.");
    return;
  }

  const ::protocol::WarriorDescription *warrior_description =
    combat_role_field->GetWarriorDescription(
        combat_warrior_field->GetFields().conf_id());
  if (warrior_description == NULL) {
    MYSYA_ERROR("[AI] CombatRoleField::GetWarriorDescription(%d) failed.",
        combat_warrior_field->GetFields().conf_id());
    return;
  }

  require::RequireSceneFetch require_message;
  require_message.set_combat_id();
  require_message.set_except_camp_id();
  require_message.mutable_pos()->set_x(
      combat_warrior_field->GetFields().origin_pos_x());
  require_message.mutable_pos()->set_y(
      combat_warrior_field->GetFields().origin_pos_y());
  // TODO: search_range
  require_message.set_range(warrior_description->attack_range());

  if (AiApp::GetInstance()->GetRequireDispatcher()->Dispatch(
        REQUIRE_SCENE_FETCH, &require_message) == -1) {
    MYSYA_ERROR("[AI] RequireDispatcher::Dispatch(REQUIRE_SCENE_FETCH) failed.");
    return;
  }

  for (int i = 0; i < require_message.building_size(); ++i) {
    int32_t building_id = require_message.building(i);
    CombatBuildingField *combat_building_field = combat_field->GetBuilding(
        building_id);
    if (combat_building_field == NULL) {
      continue;
    }

    this->host_->SetTarget(::protocol::COMBAT_ENTITY_BUILDING, building_id);
    this->host_->GotoStatus(AutoStatus::MOVE);
    return;
  }

  for (int i = 0; i < require_message.warrior_size(); ++i) {
    int32_t warrior_id = require_message.warrior(i);
    CombatWarriorField *combat_warrior_field = combat_field-.GetWarrior(
        warrior_id);
    if (combat_warrior_field == NULL) {
      continue;
    }

    this->host_->SetTarget(::protocol::COMBAT_ACTION_TYPE_WARRIOR, warrior_id);
    this->host_->GotoStatus(AutoStatus::MOVE);
    return;
  }
}

}  // namespace ai
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
