if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/mysya/tests)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/mysya/tests)

set(THREAD_TEST_SOURCES
    ${SRC_MYSYA_PATH}/thread.cc
    ${SRC_MYSYA_PATH}/tests/thread_test.cc)
add_executable(thread_test ${THREAD_TEST_SOURCES})
target_link_libraries(thread_test pthread rt dl)

set(LOGGER_TEST_SOURCES
    ${SRC_MYSYA_PATH}/thread.cc
    ${SRC_MYSYA_PATH}/logger.cc
    ${SRC_MYSYA_PATH}/tests/logger_test.cc)
add_executable(logger_test ${LOGGER_TEST_SOURCES})
target_link_libraries(logger_test rt pthread dl)

set(EVENT_LOOP_TEST_SOURCES
    ${SRC_MYSYA_PATH}/dynamic_buffer.cc
    ${SRC_MYSYA_PATH}/event_channel.cc
    ${SRC_MYSYA_PATH}/event_loop.cc
    ${SRC_MYSYA_PATH}/logger.cc
    ${SRC_MYSYA_PATH}/tests/event_loop_test.cc)
add_executable(event_loop_test ${EVENT_LOOP_TEST_SOURCES})
target_link_libraries(event_loop_test rt dl)
