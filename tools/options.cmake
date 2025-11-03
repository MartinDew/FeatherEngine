option(STATIC_CPP "STATIC_CPP" ON)


if (WIN32)
    option(USE_LLVM "USE_LLVM" ON)
else()
    option(USE_LLVM "USE_LLVM" OFF)
endif()

if (${USE_LLVM})
    if (MSVC)
        set(CMAKE_C_COMPILER "clang-cl" CACHE STRING "The Cmake C Compiler")
        set(CMAKE_CXX_COMPILER "clang-cl" CACHE STRING "The Cmake CXX Compiler")
    else()
        set(CMAKE_C_COMPILER "clang" CACHE STRING "The Cmake C Compiler")
        set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "The Cmake CXX Compiler")
    endif()
endif()