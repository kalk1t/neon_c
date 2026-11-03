cmake_minimum_required(VERSION 3.24)
project(RoadstersRaylibDependencyCheck LANGUAGES C)

if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This dependency check uses the pinned Windows x64 MSVC package.")
endif()

set(RAYLIB_PACKAGE "${CMAKE_CURRENT_LIST_DIR}/../../third_party/raylib-6.2-msvc-x64/raylib-6.2_win64_msvc16" CACHE PATH "Verified raylib package directory")
if(NOT EXISTS "${RAYLIB_PACKAGE}/include/raylib.h" OR NOT EXISTS "${RAYLIB_PACKAGE}/lib/raylib.lib")
    message(FATAL_ERROR "Install the pinned raylib archive with scripts/Install-Raylib.ps1 first.")
endif()

add_library(raylib_verified STATIC IMPORTED)
set_target_properties(raylib_verified PROPERTIES
    IMPORTED_LOCATION "${RAYLIB_PACKAGE}/lib/raylib.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_PACKAGE}/include")

add_executable(raylib_dependency_check main.c)
set_target_properties(raylib_dependency_check PROPERTIES
    C_STANDARD 17
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
target_compile_options(raylib_dependency_check PRIVATE /W4 /WX)
target_link_libraries(raylib_dependency_check PRIVATE raylib_verified opengl32 gdi32 winmm shell32 user32)

enable_testing()
add_test(NAME raylib_link_and_runtime COMMAND raylib_dependency_check)

