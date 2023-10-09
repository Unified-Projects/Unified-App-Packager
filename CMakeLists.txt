project(unified-app-packager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options($<$<C_COMPILER_ID:Clang>:-fcolor-diagnostics>)

add_compile_options(-Werror -Wall -Wextra -Wno-write-strings -Wno-missing-braces)
add_compile_options(-march=x86-64-v2)
# add_compile_options(-ffreestanding -nostdlib)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c*")

add_executable(Packager ${SOURCES})
target_include_directories(Packager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(Packager PRIVATE stdc++) # winmm.lib may be windows only!