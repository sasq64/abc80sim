cmake_minimum_required(VERSION 3.15)
project(abc80sim)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS Off)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    add_compile_options(-W -Wall -Wextra -Wpedantic -Werror)
    add_compile_definitions(_GNU_SOURCE=1)
endif()
add_compile_definitions(
    _THREAD_SAFE
    HAVE_CONFIG_H)

list(APPEND FLAGS
    # -Werror=unknown-warning-option
    # -Werror=implicit
    # -Werror=missing-braces
    # -Werror=return-type
    # -Werror=trigraphs
    # -Werror=pointer-arith
    # -Werror=strict-prototypes
    # -Werror=missing-prototypes
    # -Werror=missing-declarations
    # -Werror=comment
    # -Werror=vla
    -fwrapv
    -fno-common
    -fvisibility=hidden)

set(SOURCES
    src/abcfile.c
    src/abcfont.c
    src/abcio.c
    src/abcmem.c
    src/cas.c
    src/clock.c
    src/console.c
    src/disk.c
    src/filelist.c
    src/fileop.c
    src/hostfile.c
    src/nstime.c
    src/print.c
    src/rtc.c
    src/screenshot.c
    src/sdlscrn.cpp
    src/simprint.c
    src/trace.c
    src/z80.c
    src/z80dis.c
    src/z80irq.c
    src/roms/abc802rom.c
    src/roms/abc80_devs.c
    src/roms/abc80bas40n.c
    src/roms/abc80bas40o.c
    src/roms/abc80bas80n.c
    src/roms/abc80bas80o.c)

add_library(emu_lib STATIC ${SOURCES})
target_link_libraries(emu_lib PUBLIC ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} SDL2_image::SDL2_image)
target_include_directories(emu_lib PRIVATE ${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIRS})
target_compile_options(emu_lib PRIVATE ${FLAGS})

add_executable(emu src/abc80.cpp)
target_link_libraries(emu PUBLIC emu_lib ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} SDL2_image::SDL2_image)
target_include_directories(emu PRIVATE ${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIRS})
target_compile_options(emu PRIVATE ${FLAGS})

add_custom_target(copy_examples
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/abcdir/"
        "$<TARGET_FILE_DIR:emu_lib>/abcdir"
    COMMENT "Copying examples"
)
add_dependencies(emu copy_examples)
