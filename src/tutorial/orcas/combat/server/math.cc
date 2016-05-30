#include "tutorial/orcas/combat/server/math.h"

#include <math.h>
#include <stdlib.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {
namespace math {

int distance(int start_x, int start_y, int end_x, int end_y) {
  int a = abs(end_x - start_x);
  int b = abs(end_y - start_y);
  return (int)(sqrt(a*a + b*b) + 0.5f);
}

}  // namespace math
}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial
