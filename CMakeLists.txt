project("mysya")

cmake_minimum_required(VERSION 0.1)

set(MY_CXX_FLAGS_WARNING "-Wall -Werror -Wno-deprecated")
set(MY_CXX_FLAGS_MACRO "")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -g -std=c++11 ${MY_CXX_FLAGS_WARNING} -rdynamic ${MY_CXX_FLAGS_MACRO}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories("${PROJECT_SOURCE_DIR}/src")

add_subdirectory(src)
