if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/combat/test)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/tutorial/orcas/combat/test)

set(BLOCK_FILE_SOURCES
    ${TUTORIAL_SOURCE_DIR}/orcas/combat/test/block_file.cc
    )

add_executable(block_file ${UTIL_SOURCES} ${COMBAT_SOURCES} ${BLOCK_FILE_SOURCES})
target_link_libraries(block_file mysya-ioevent mysya-codec mysya-orcas-deps protobuf z pthread rt dl)
