cmake_minimum_required(VERSION 3.10)
set(TARGET_NAME autorotator)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(${TARGET_NAME})

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/*.cpp")
add_executable(${TARGET_NAME} autorotator ${SOURCES})

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(UHD)

include_directories(${UHD_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${UHD_LIBRARIES})

