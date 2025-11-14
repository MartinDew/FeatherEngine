option(STATIC_CPP "STATIC_CPP" ON)

set(STATIC_CPP ${STATIC_CPP} CACHE BOOL "Build static C++ libraries")

if (${STATIC_CPP})
    set(FEATHER_BUILD_TYPE STATIC CACHE INTERNAL "Build type symbol for libraries")
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build shared libraries")
    else ()
    set(FEATHER_BUILD_TYPE SHARED CACHE INTERNAL "Build type symbol for libraries")
    set(BUILD_SHARED_LIBS ON CACHE INTERNAL "Build shared libraries")
endif ()

if (NOT WIN32)
    option(USE_MINGW "USE_MINGW" OFF)

    if (${USE_MINGW})
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/tools/toolchain-mingw64.cmake" CACHE STRING "Toolchain file for MinGW-w64 cross-compilation")
    endif ()
endif ()

option(USE_ANDROID "USE_ANDROID" OFF)
if (${USE_ANDROID})
    # get ANDROID_NDK_HOME from environment variable
    if (NOT DEFINED ENV{ANDROID_NDK_HOME})
        message(FATAL_ERROR "ANDROID_NDK_HOME environment variable not set")
    endif ()
    #    set(ANDROID_NDK_HOME $ENV{ANDROID_NDK_HOME} PATH "Path to Android NDK")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake" CACHE STRING "Toolchain file for Android NDK")
    set(ANDROID_ABI "arm64-v8a" CACHE STRING "Android ABI")
    set(ANDROID_PLATFORM 21 CACHE STRING "Android platform version")
endif ()

option(USE_LLVM "USE_LLVM" OFF)

if(${USE_LLVM})
    # Check if we're using Visual Studio generator with toolset support
    if(CMAKE_GENERATOR MATCHES "Visual Studio" AND WIN32)
        # For Visual Studio generators, use toolset instead of toolchain file
        set(CMAKE_GENERATOR_TOOLSET "ClangCl" CACHE STRING "Visual Studio toolset" FORCE)
        message(STATUS "Using ClangCl toolset for Visual Studio generator")
    else()
        # For all other generators, use the toolchain file
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/tools/toolchain-llvm.cmake" CACHE FILEPATH "LLVM toolchain file" FORCE)
        message(STATUS "Using LLVM toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
endif()

# Clang-Tidy integration
option(ENABLE_CLANG_TIDY "Enable clang-tidy checks during compilation" OFF)

if (ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)
    if (CLANG_TIDY_EXE)
        set(CMAKE_CXX_CLANG_TIDY
                ${CLANG_TIDY_EXE};
                -header-filter=.*;
        )
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    else ()
        message(WARNING "clang-tidy not found!")
    endif ()
endif ()