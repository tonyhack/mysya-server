if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/mysya/qservice/tests)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/mysya/qservice/tests)

set(TCP_SERVICE_TEST_SOURCES
    ${SRC_MYSYA_QSERVICE_PATH}/tests/tcp_service_test.cc)
add_executable(tcp_service_test ${TCP_SERVICE_TEST_SOURCES})
target_link_libraries(tcp_service_test mysya-ioevent ${MYSYA_QSERVICE_LINK_LIBRARIES})
