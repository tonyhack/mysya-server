if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

set(SRC_MYSYA_IOEVENT_PATH ${SRC_MYSYA_PATH}/ioevent)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/mysya/ioevent)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/mysya/ioevent)

set(MYSYA_IOEVENT_SOURCES
    ${SRC_MYSYA_IOEVENT_PATH}/dynamic_buffer.cc
    ${SRC_MYSYA_IOEVENT_PATH}/event_channel.cc
    ${SRC_MYSYA_IOEVENT_PATH}/event_loop.cc
    ${SRC_MYSYA_IOEVENT_PATH}/logger.cc
    ${SRC_MYSYA_IOEVENT_PATH}/socket_address.cc
    ${SRC_MYSYA_IOEVENT_PATH}/tcp_socket.cc
    ${SRC_MYSYA_IOEVENT_PATH}/tcp_socket_app.cc
    ${SRC_MYSYA_IOEVENT_PATH}/timing_wheel.cc
    ${SRC_MYSYA_IOEVENT_PATH}/thread.cc)

add_library(mysya-ioevent SHARED ${MYSYA_IOEVENT_SOURCES} ${MYSYA_UTIL_SOURCES})

set(MYSYA_IOEVENT_LINK_LIBRARIES mysya-ioevent pthread rt dl)

add_subdirectory(tests)
