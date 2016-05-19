if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/gateway/robot)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/gateway/robot)

set(GATEWAY_ROBOT_SOURCES
    ${GATEWAY_SOURCE_DIR}/robot/actor.cc
    ${GATEWAY_SOURCE_DIR}/robot/main.cc
    ${GATEWAY_SOURCE_DIR}/robot/message_handler.cc
    ${GATEWAY_SOURCE_DIR}/robot/robot_app.cc
    ${GATEWAY_SOURCE_DIR}/robot/user_command_handler.cc
    )

add_executable(gateway_robot ${UTIL_SOURCES} ${GATEWAY_SOURCES} ${GATEWAY_ROBOT_SOURCES} ${ORCAS_PROTOCOL_SOURCES})
target_link_libraries(gateway_robot mysya-ioevent mysya-codec mysya-orcas-deps protobuf pthread rt dl)
