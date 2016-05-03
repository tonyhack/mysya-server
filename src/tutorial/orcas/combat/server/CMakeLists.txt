if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/combat/server)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/combat/server)

set(COMBAT_SERVER_SOURCES
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/app_server.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/app_session.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_field.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_field_manager.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_message_handler.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_role_field.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_role_field_manager.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/configs.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/event_dispatcher.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/main.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/warrior_field.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/warrior_field_pool.cc
    )

add_executable(combat_server ${UTIL_SOURCES} ${COMBAT_SOURCES} ${ORCAS_PROTOCOL_SOURCES} ${COMBAT_PROTOCOL_SOURCES} ${COMBAT_SERVER_SOURCES})
target_link_libraries(combat_server mysya-ioevent mysya-codec mysya-orcas-deps protobuf pthread rt dl)
