#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_FORMULA_FORMULA_PROCESSOR_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_FORMULA_FORMULA_PROCESSOR_H

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace formula {

class FormulaProcessor {
 public:
  bool FormulaDamage(CombatWarriorField *active, CombatWarriorField *passive);
  bool FormulaDamage(CombatWarriorField *active, CombatBuildingField *passive);
};

}  // namespace formula
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_FORMULA_FORMULA_PROCESSOR_H
