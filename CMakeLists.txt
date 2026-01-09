cmake_minimum_required(VERSION 3.10)
project(SimpleRDBMS C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Compiler options
if(MSVC)
    add_compile_options(/W4)
    # Static runtime for MSVC (MultiThreaded = /MT, MultiThreadedDLL = /MD)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    # Force static linking
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Static linking for GCC/MinGW - prefer static libraries
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")
    # For Release builds, use full static linking
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    endif()
endif()

# Release-specific optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2 /DNDEBUG)
        # Remove debug symbols for smaller size
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG:NONE")
    else()
        add_compile_options(-O3 -DNDEBUG)
        # Link-time optimization for smaller, faster binaries
        add_compile_options(-flto)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto -s")
        # Strip symbols for smaller binary size
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--strip-all")
    endif()
endif()

include_directories(src)

set(COMMON_SOURCES
    src/storage.c
    src/catalog.c
    src/table.c
    src/row.c
    src/index.c
    src/util.c
)

add_executable(db
    src/main.c
    src/repl.c
    src/lexer.c
    src/parser.c
    src/sql.c
    ${COMMON_SOURCES}
)

# Set output directory for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(db PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/release"
    )
endif()

# Installation configuration
install(TARGETS db
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# Install data directory structure (optional - creates empty data dir)
# install(DIRECTORY data
#     DESTINATION .
#     FILES_MATCHING PATTERN ".gitkeep"
# )
