if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/mysya/qservice/tests)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/mysya/qservice/tests)

set(ECHO_SERVER_TEST_SOURCES
    ${SRC_MYSYA_QSERVICE_PATH}/tests/echo_server_test.cc)
add_executable(echo_server_test ${ECHO_SERVER_TEST_SOURCES})
target_link_libraries(echo_server_test mysya-ioevent ${MYSYA_QSERVICE_LINK_LIBRARIES})

set(ECHO_CLIENT_TEST_SOURCES ${SRC_MYSYA_QSERVICE_PATH}/tests/echo_client_test.cc)
add_executable(echo_client_test ${ECHO_CLIENT_TEST_SOURCES})
target_link_libraries(echo_client_test mysya-ioevent ${MYSYA_QSERVICE_LINK_LIBRARIES})

set(BROADCAST_SERVER_TEST_SOURCES
    ${SRC_MYSYA_QSERVICE_PATH}/tests/broadcast_server_test.cc)
add_executable(broadcast_server_test ${BROADCAST_SERVER_TEST_SOURCES})
target_link_libraries(broadcast_server_test mysya-ioevent ${MYSYA_QSERVICE_LINK_LIBRARIES})

set(BROADCAST_CLIENT_TEST_SOURCES ${SRC_MYSYA_QSERVICE_PATH}/tests/broadcast_client_test.cc)
add_executable(broadcast_client_test ${BROADCAST_CLIENT_TEST_SOURCES})
target_link_libraries(broadcast_client_test mysya-ioevent ${MYSYA_QSERVICE_LINK_LIBRARIES})
