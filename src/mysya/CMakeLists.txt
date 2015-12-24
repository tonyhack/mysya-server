if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/mysya)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/mysya)

set(MYSYA_SOURCES
    ${SRC_MYSYA_PATH}/dynamic_buffer.cc
    ${SRC_MYSYA_PATH}/event_channel.cc
    ${SRC_MYSYA_PATH}/event_loop.cc
    ${SRC_MYSYA_PATH}/exception.cc
    ${SRC_MYSYA_PATH}/logger.cc
    ${SRC_MYSYA_PATH}/socket_address.cc
    ${SRC_MYSYA_PATH}/tcp_socket.cc
    ${SRC_MYSYA_PATH}/tcp_socket_app.cc
    ${SRC_MYSYA_PATH}/timestamp.cc
    ${SRC_MYSYA_PATH}/timing_wheel.cc
    ${SRC_MYSYA_PATH}/thread.cc)

add_library(mysya SHARED ${MYSYA_SOURCES})

set(MYSYA_LINK_LIBRARIES mysya pthread rt dl)

add_subdirectory(tests)
