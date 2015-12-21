if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/mysya/tests)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/mysya/tests)

set(THREAD_TEST_SOURCES
    ${SRC_MYSYA_PATH}/tests/thread_test.cc)
add_executable(thread_test ${THREAD_TEST_SOURCES})
target_link_libraries(thread_test ${MYSYA_LINK_LIBRARIES})

set(LOGGER_TEST_SOURCES
    ${SRC_MYSYA_PATH}/tests/logger_test.cc)
add_executable(logger_test ${LOGGER_TEST_SOURCES})
target_link_libraries(logger_test ${MYSYA_LINK_LIBRARIES})

set(EVENT_LOOP_TEST_SOURCES
    ${SRC_MYSYA_PATH}/tests/event_loop_test.cc)
add_executable(event_loop_test ${EVENT_LOOP_TEST_SOURCES})
target_link_libraries(event_loop_test ${MYSYA_LINK_LIBRARIES})

set(TCP_SOCKET_TEST_SOURCES
    ${SRC_MYSYA_PATH}/tests/tcp_socket_test.cc)
add_executable(tcp_socket_test ${TCP_SOCKET_TEST_SOURCES})
target_link_libraries(tcp_socket_test ${MYSYA_LINK_LIBRARIES})

set(TIMING_WHEEL_TEST_SOURCES
    ${SRC_MYSYA_PATH}/tests/timing_wheel_test.cc)
add_executable(timing_wheel_test ${TIMING_WHEEL_TEST_SOURCES})
target_link_libraries(timing_wheel_test ${MYSYA_LINK_LIBRARIES})
