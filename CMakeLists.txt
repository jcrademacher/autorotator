cmake_minimum_required(VERSION 3.10)
set(TARGET_NAME autorotator)

project(${TARGET_NAME})

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
add_executable(${TARGET_NAME} autorotator ${SOURCES})

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

