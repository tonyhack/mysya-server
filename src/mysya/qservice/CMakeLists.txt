if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

set(SRC_MYSYA_QSERVICE_PATH ${SRC_MYSYA_PATH}/qservice)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/mysya/qservice)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/mysya/qservice)

set(MYSYA_QSERVICE_SOURCES
    ${SRC_MYSYA_QSERVICE_PATH}/event_loop_thread_pool.cc
    ${SRC_MYSYA_QSERVICE_PATH}/message_queue.cc 
    ${SRC_MYSYA_QSERVICE_PATH}/tcp_service.cc
    ${SRC_MYSYA_QSERVICE_PATH}/transport_agent.cc)

add_library(mysya-qservice SHARED ${MYSYA_QSERVICE_SOURCES} ${MYSYA_UTIL_SOURCES})

set(MYSYA_QSERVICE_LINK_LIBRARIES mysya-qservice pthread rt dl)

add_subdirectory(tests)
