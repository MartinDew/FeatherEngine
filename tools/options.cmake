option(STATIC_CPP "STATIC_CPP" ON)
if (${STATIC_CPP})
    set(FEATHER_BUILD_TYPE_SYMBOL STATIC CACHE INTERNAL "Build type symbol for libraries")
else ()
    set(FEATHER_BUILD_TYPE_SYMBOL SHARED CACHE INTERNAL "Build type symbol for libraries")
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

if (${USE_LLVM})
    if (WIN32 AND NOT MINGW)
        if (${CMAKE_GENERATOR} EQUAL "Visual Studio 17 2022")
            set(CMAKE_GENERATOR_TOOLSET "ClangCl")
        else ()
            set(CMAKE_C_COMPILER "clang-cl" CACHE STRING "The Cmake C Compiler")
            set(CMAKE_CXX_COMPILER "clang-cl" CACHE STRING "The Cmake CXX Compiler")
        endif ()
    else ()
        set(CMAKE_C_COMPILER "clang" CACHE STRING "The Cmake C Compiler")
        set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "The Cmake CXX Compiler")
    endif ()
endif ()

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