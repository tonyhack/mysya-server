if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

set(SRC_MYSYA_CODEC_PATH ${SRC_MYSYA_PATH}/codec)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/mysya/codec)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/mysya/codec)

set(MYSYA_CODEC_SOURCES
    ${SRC_MYSYA_CODEC_PATH}/protobuf_codec.cc)

add_library(mysya-codec SHARED ${MYSYA_CODEC_SOURCES} ${MYSYA_UTIL_SOURCES})

set(MYSYA_CODEC_LINK_LIBRARIES mysya-codec mysya-ioevent protobuf pthread rt dl)

add_subdirectory(tests)
