if (COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif (COMMAND cmake_policy)

set(GENERATE_PATH_CONFIG_PATH ${TUTORIAL_SOURCE_DIR}/tools/generate_path_config)

set(GENERATE_PATH_CONFIG_SOURCES
    ${GENERATE_PATH_CONFIG_PATH}/generate_path_config.cc
    ${GENERATE_PATH_CONFIG_PATH}/tinyxml/tinystr.cpp
    ${GENERATE_PATH_CONFIG_PATH}/tinyxml/tinyxml.cpp
    ${GENERATE_PATH_CONFIG_PATH}/tinyxml/tinyxmlerror.cpp
    ${GENERATE_PATH_CONFIG_PATH}/tinyxml/tinyxmlparser.cpp
    )

set(EXECUTABLE_OUTPUT_PATH ${GENERATE_PATH_CONFIG_PATH})
add_executable(generate_path_config ${GENERATE_PATH_CONFIG_SOURCES})
target_link_libraries(generate_path_config z rt dl)
