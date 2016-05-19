#include <stdio.h>

#include "tutorial/orcas/gateway/robot/robot_app.h"

int main(int argc, char *argv[]) {
  ::tutorial::orcas::gateway::robot::RobotApp::GetInstance()->Start();

  return 0;
}
