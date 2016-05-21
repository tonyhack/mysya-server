#ifndef TUTORIAL_ORCAS_COMBAT_SERVER_APPS_H
#define TUTORIAL_ORCAS_COMBAT_SERVER_APPS_H

#include <mysya/util/class_util.h>

namespace tutorial {
namespace orcas {
namespace combat {
namespace server {

class AppServer;

class Apps {
 public:
  explicit Apps(AppServer *host);
  ~Apps();

  bool Initialize();
  void Finalize();

 private:
  AppServer *host_;

  MYSYA_DISALLOW_COPY_AND_ASSIGN(Apps);
};

}  // namespace server
}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_SERVER_APPS_H
