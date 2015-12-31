if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}/mysya/codec/tests)
set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/mysya/codec/tests)

set(PROTOBUF_CODEC_TEST_SOURCES
    ${SRC_MYSYA_CODEC_PATH}/tests/protobuf_codec_test.cc)
add_executable(protobuf_codec_test ${PROTOBUF_CODEC_TEST_SOURCES})
target_link_libraries(protobuf_codec_test ${MYSYA_CODEC_LINK_LIBRARIES})
