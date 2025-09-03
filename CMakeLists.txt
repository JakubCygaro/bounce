cmake_minimum_required(VERSION 3.80)

project(bounce)

include(CMakeOptions.txt)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(BUILD_TYPES Debug Release)
set(PLATFORMS Web Desktop)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(NOT PLATFORM)
    set(PLATFORM "Desktop")
endif()

if(NOT (${CMAKE_BUILD_TYPE} IN_LIST BUILD_TYPES))
    message(FATAL_ERROR "Unknown build type " ${CMAKE_BUILD_TYPE})
endif()

if(NOT (${PLATFORM} IN_LIST PLATFORMS))
    message(FATAL_ERROR "Unknown platform type " ${PLATFORM})
endif()

set(RAYLIB_VERSION 5.5)
set(raylib_VERBOSE YES)

if(STATIC_RAYLIB)
    set(raylib_USE_STATIC_LIBS YES)
    set(PLATFORM "Desktop")
endif()

if(STATIC_RAYLIB AND NOT RAYLIB_STATIC_INSTALL)
    message(WARNING "Path to raylib installation directory not set")
    set(raylib_FOUND NO)
elseif(STATIC_RAYLIB AND RAYLIB_STATIC_INSTALL)
    message(STATUS "Building with statically linked raylib from provided path")
    find_package(raylib ${RAYLIB_VERSION} QUIET PATHS ${RAYLIB_STATIC_INSTALL} NO_DEFAULT_PATH)
else()
    find_package(raylib ${RAYLIB_VERSION} QUIET)
endif()


if (NOT raylib_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
  endif()
endif()

add_executable(bounce
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ball.c
)
target_link_libraries(bounce PRIVATE raylib)
target_link_libraries(bounce PRIVATE -lm)
target_include_directories(bounce PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_compile_options(bounce
    PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)
if(PLATFORM STREQUAL "Web")
    target_compile_definitions(bounce
        PRIVATE -DPLATFORM_WEB
    )
endif()

if (${PLATFORM} STREQUAL "Web")
    set_target_properties(bounce PROPERTIES SUFFIX ".html")
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -s GL_ENABLE_GET_PROC_ADDRESS=1 -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_STACK=64MB -s INITIAL_MEMORY=128MB -DPLATFORM_WEB --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html --post-js ${CMAKE_CURRENT_SOURCE_DIR}/postfix.js --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/resources"
    )
    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/postfix.js
    )
    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shell.html
    )
    add_custom_target(copy_styles ALL
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/styles.css
    )
    add_custom_command(TARGET copy_styles
        PRE_BUILD
        COMMAND cmake
        ARGS -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/styles.css ${CMAKE_BINARY_DIR}/styles.css
    )
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    add_custom_target(copy_resources ALL
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/qubodup-cfork-ccby3-jump.ogg
    )
    add_custom_command(TARGET copy_resources
        PRE_BUILD
        COMMAND cmake
        ARGS -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    )
endif()
