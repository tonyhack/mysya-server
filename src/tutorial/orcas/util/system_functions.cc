#include "tutorial/orcas/util/system_functions.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

namespace tutorial {
namespace orcas {
namespace util {

bool create_pid_file(const char *file) {
  FILE *pid_file = fopen(file, "w");
  if (NULL == pid_file) {
    return false;
  }

  fprintf(pid_file, "%d\n", getpid());
  fclose(pid_file);

  return true;
}

}  // namespace util
}  // namespace orcas
}  // namespace tutorial
