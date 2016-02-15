cmake_minimum_required (VERSION 2.8.11)
project(ipa)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

add_definitions(-D_GNU_SOURCE -std=c11)

file(GLOB sources "*.c")
add_executable(${CMAKE_PROJECT_NAME} ${sources})

find_package(libimobiledevice REQUIRED)
include_directories(${libimobiledevice_INCLUDE_DIRS})
target_link_libraries (${CMAKE_PROJECT_NAME} ${libimobiledevice_LIBRARIES})

find_package(libplist REQUIRED)
include_directories(${libplist_INCLUDE_DIRS})
target_link_libraries (${CMAKE_PROJECT_NAME} ${libplist_LIBRARIES})
