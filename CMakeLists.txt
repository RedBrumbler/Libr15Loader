cmake_minimum_required(VERSION 3.22)

option(QUEST "Build for quest" ON)
option(WINDOWS "Build for windows" OFF)

if (QUEST)
    include (${CMAKE_CURRENT_LIST_DIR}/cmake/quest.cmake)
endif()

if (WINDOWS)
    include (${CMAKE_CURRENT_LIST_DIR}/cmake/windows.cmake)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/version.txt")
	file (STRINGS "${CMAKE_CURRENT_LIST_DIR}/version.txt" PROJECT_VERSION)
else()
    set(PROJECT_VERSION "0.1.0")
endif()

project(r15loader VERSION ${PROJECT_VERSION})

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(
    r15
    SHARED
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/modloader.cpp
    ${SOURCE_DIR}/fileutils.cpp
)

target_include_directories(r15 PRIVATE ${INCLUDE_DIR})

add_compile_definitions(LIB_ID="${CMAKE_PROJECT_NAME}")
add_compile_definitions(VERSION="${CMAKE_PROJECT_VERSION}")
add_compile_options(-fPIE -flto -fPIC -fno-rtti -fno-exceptions -O3 -Oz -fvisibility=hidden -Wall -Wextra -Wpedantic -Werror)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/git.cmake)

if (QUEST)
    add_compile_definitions(QUEST)
    target_include_directories(r15 PRIVATE ${CMAKE_ANDROID_NDK}/sources/)
    target_link_libraries(r15 log)
endif()

if (WINDOWS)
    add_compile_definitions(WINDOWS)
endif()
