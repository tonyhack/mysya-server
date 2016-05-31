if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/combat/server/scene/test)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/combat/server/scene/test)

set(SCENE_TEST_SOURCES
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/server/scene/test/scene_test.cc
    )

add_executable(scene_test ${UTIL_SOURCES} ${COMBAT_SOURCES} ${ORCAS_PROTOCOL_SOURCES}
    ${COMBAT_PROTOCOL_SOURCES} ${VOTE_SOURCES} ${EVENT_SOURCES} ${REQUIRE_SOURCES}
    ${COMBAT_SERVER_SOURCES} ${COMBAT_SERVER_APPS_SOURCES} ${SCENE_TEST_SOURCES})
target_link_libraries(scene_test mysya-ioevent mysya-codec mysya-orcas-deps protobuf z pthread rt dl)

