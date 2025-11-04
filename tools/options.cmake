option(STATIC_CPP "STATIC_CPP" ON)

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