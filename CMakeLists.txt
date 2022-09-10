cmake_minimum_required(VERSION 3.10)
set(TARGET_NAME autorotator)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(EXEC rx_tx_samples_to_file_autorotate)

project(${TARGET_NAME})

find_package(Boost COMPONENTS program_options filesystem REQUIRED)
find_package(UHD)

include_directories(${UHD_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB SOURCES "${PROJECT_SOURCE_DIR}/stm23ip.cpp")
add_executable(${TARGET_NAME} "${TARGET_NAME}.cpp" ${SOURCES})
add_executable(${EXEC} "${EXEC}.cpp" ${SOURCES})

target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${UHD_LIBRARIES})

target_link_libraries(${EXEC} ${Boost_LIBRARIES})
target_link_libraries(${EXEC} ${UHD_LIBRARIES})

