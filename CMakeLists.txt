cmake_minimum_required (VERSION 3.7)

project(kiss)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


aux_source_directory(${PROJECT_SOURCE_DIR}/src DIR_SRCS)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/libs)

link_directories(${PROJECT_BINARY_DIR}/libs)


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/third/spdlog/include )

add_library(kiss  ${DIR_SRCS})

add_subdirectory(examples)




