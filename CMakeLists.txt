cmake_minimum_required(VERSION 3.0)
project(elkhound LANGUAGES C CXX)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(BUILD_TESTING)
    enable_testing()
endif()

if(MSVC)
    # Use local FLEX and BISON for MSVC builds
    set(BISON_EXECUTABLE win_bison)
    set(FLEX_EXECUTABLE win_flex --wincompat)
    set(FLEX_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/winflexbison/flex/src")

    add_subdirectory(thirdparty/winflexbison)
endif()

add_subdirectory(thirdparty/fmt)
add_subdirectory(src)
