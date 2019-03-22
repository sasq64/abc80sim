cmake_minimum_required(VERSION 3.5)
project(abc80sim)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL REQUIRED)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FLAGS
    -U__STRICT_ANSI__
    -D_GNU_SOURCE=1
    -D_THREAD_SAFE
    -Werror=attributes
    -W
    -Wall
    -pedantic
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
    -DHAVE_CONFIG_H
    -fwrapv
    -O3
    -g
    -fno-common
    -fvisibility=hidden)

set(SOURCES
    src/abc80.cpp
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
    src/sdlscrn.c
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

add_executable(emu ${SOURCES})
target_link_libraries(emu PUBLIC png z ${SDL_LIBRARY})
target_include_directories(emu PRIVATE ${SDL_INCLUDE_DIR})
target_compile_options(emu PRIVATE ${FLAGS})

