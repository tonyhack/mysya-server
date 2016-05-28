if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

add_subdirectory(event)
add_subdirectory(require)
add_subdirectory(vote)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/combat/server)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/combat/server)

set(VOTE_SOURCES
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/vote/cc/vote.pb.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/vote/cc/vote_combat.pb.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/vote/cc/vote_scene.pb.cc
    )

set(EVENT_SOURCES
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/event/cc/event.pb.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/event/cc/event_combat.pb.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/event/cc/event_scene.pb.cc
    )

set(REQUIRE_SOURCES
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/require/cc/require.pb.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/require/cc/require_scene.pb.cc
    )

set(COMBAT_SERVER_SCENE_SOURCES
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/building.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/entity_builder.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/combat_event_handler.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/grid.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/move_action.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/require_handler.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/scene_app.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/scene.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/scene_manager.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/warrior.cc
    )

set(COMBAT_SERVER_SOURCES
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/apps.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/app_server.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/app_session.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_building_field.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_building_field_pool.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_event_handler.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_field.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_field_manager.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_message_handler.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_role_field.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_role_field_manager.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_warrior_field.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/combat_warrior_field_pool.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/configs.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/event_dispatcher.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/require_dispatcher.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/vote_dispatcher.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/user_combat_message_handler.cc
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/user_message_dispatcher.cc
    )

set(COMBAT_SERVER_MAIN_SOURCES
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/main.cc
    )

add_subdirectory(scene/test)

add_executable(combat_server ${UTIL_SOURCES} ${COMBAT_SOURCES} ${ORCAS_PROTOCOL_SOURCES}
    ${COMBAT_PROTOCOL_SOURCES} ${VOTE_SOURCES} ${EVENT_SOURCES} ${REQUIRE_SOURCES}
    ${COMBAT_SERVER_SOURCES} ${COMBAT_SERVER_SCENE_SOURCES} ${COMBAT_SERVER_MAIN_SOURCES})
target_link_libraries(combat_server mysya-ioevent mysya-codec mysya-orcas-deps protobuf pthread rt dl)
