# Advanced LLVM/Clang Toolchain File with additional optimizations
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=tools/toolchain-llvm-advanced.cmake ..
#        cmake -DCMAKE_TOOLCHAIN_FILE=tools/toolchain-llvm-advanced.cmake -DLLVM_ROOT=/path/to/llvm ..

# Allow specifying LLVM installation directory
set(LLVM_ROOT "" CACHE PATH "LLVM installation root directory")

# Helper function to find LLVM tools
function(find_llvm_tool VAR_NAME TOOL_NAME)
    if(LLVM_ROOT)
        find_program(${VAR_NAME} 
            NAMES ${TOOL_NAME}
            PATHS "${LLVM_ROOT}/bin"
            NO_DEFAULT_PATH
        )
    endif()
    
    if(NOT ${VAR_NAME})
        find_program(${VAR_NAME} NAMES ${TOOL_NAME})
    endif()
endfunction()

# Detect platform and set compilers
if(WIN32)
    # Check if we're in a MinGW environment
    if(MINGW OR CMAKE_SYSTEM_NAME STREQUAL "MinGW")
        # MinGW environment: Use Unix-style Clang
        find_llvm_tool(CLANG_COMPILER "clang")
        find_llvm_tool(CLANGXX_COMPILER "clang++")
        
        if(CLANG_COMPILER)
            set(CMAKE_C_COMPILER "${CLANG_COMPILER}" CACHE STRING "C compiler")
        else()
            set(CMAKE_C_COMPILER "clang" CACHE STRING "C compiler")
        endif()
        
        if(CLANGXX_COMPILER)
            set(CMAKE_CXX_COMPILER "${CLANGXX_COMPILER}" CACHE STRING "C++ compiler")
        else()
            set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler")
        endif()
        
        # Use LLD linker in Unix mode
        find_llvm_tool(LLD_LINKER "ld.lld")
        if(NOT LLD_LINKER)
            find_llvm_tool(LLD_LINKER "lld")
        endif()
        
        if(LLD_LINKER)
            set(CMAKE_LINKER "${LLD_LINKER}" CACHE STRING "Linker")
            set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld" CACHE STRING "Executable linker flags")
            set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld" CACHE STRING "Shared linker flags")
            set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld" CACHE STRING "Module linker flags")
            message(STATUS "LLVM Advanced: Using LLD linker")
        endif()
        
        # Use LLVM tools
        find_llvm_tool(LLVM_AR "llvm-ar")
        if(LLVM_AR)
            set(CMAKE_AR "${LLVM_AR}" CACHE STRING "Archiver")
        endif()
        
        find_llvm_tool(LLVM_RANLIB "llvm-ranlib")
        if(LLVM_RANLIB)
            set(CMAKE_RANLIB "${LLVM_RANLIB}" CACHE STRING "Ranlib")
        endif()
        
        find_llvm_tool(LLVM_NM "llvm-nm")
        if(LLVM_NM)
            set(CMAKE_NM "${LLVM_NM}" CACHE STRING "nm tool")
        endif()
        
        find_llvm_tool(LLVM_OBJDUMP "llvm-objdump")
        if(LLVM_OBJDUMP)
            set(CMAKE_OBJDUMP "${LLVM_OBJDUMP}" CACHE STRING "objdump tool")
        endif()
        
        # MinGW uses GCC-style flags
        set(CMAKE_C_FLAGS_INIT "" CACHE STRING "C flags")
        set(CMAKE_CXX_FLAGS_INIT "" CACHE STRING "C++ flags")
        
        message(STATUS "LLVM Advanced: Using Clang (MinGW mode) on Windows")
    else()
        # Native Windows: Use clang-cl (MSVC-compatible)
        find_llvm_tool(CLANG_CL_COMPILER "clang-cl")
        
        if(CLANG_CL_COMPILER)
            set(CMAKE_C_COMPILER "${CLANG_CL_COMPILER}" CACHE STRING "C compiler")
            set(CMAKE_CXX_COMPILER "${CLANG_CL_COMPILER}" CACHE STRING "C++ compiler")
        else()
            set(CMAKE_C_COMPILER "clang-cl" CACHE STRING "C compiler")
            set(CMAKE_CXX_COMPILER "clang-cl" CACHE STRING "C++ compiler")
        endif()
        
        # Use LLVM linker
        find_llvm_tool(LLD_LINK "lld-link")
        if(LLD_LINK)
            set(CMAKE_LINKER "${LLD_LINK}" CACHE STRING "Linker")
        endif()
        
        # Use LLVM archiver
        find_llvm_tool(LLVM_LIB "llvm-lib")
        if(LLVM_LIB)
            set(CMAKE_AR "${LLVM_LIB}" CACHE STRING "Archiver")
        endif()
        
        # MSVC-compatible flags
        set(CMAKE_C_FLAGS_INIT "/EHsc" CACHE STRING "C flags")
        set(CMAKE_CXX_FLAGS_INIT "/EHsc /GR" CACHE STRING "C++ flags")
        
        message(STATUS "LLVM Advanced: Using clang-cl on Windows")
    endif()
    
elseif(APPLE)
    # macOS
    find_llvm_tool(CLANG_COMPILER "clang")
    find_llvm_tool(CLANGXX_COMPILER "clang++")
    
    if(CLANG_COMPILER)
        set(CMAKE_C_COMPILER "${CLANG_COMPILER}" CACHE STRING "C compiler")
    else()
        set(CMAKE_C_COMPILER "clang" CACHE STRING "C compiler")
    endif()
    
    if(CLANGXX_COMPILER)
        set(CMAKE_CXX_COMPILER "${CLANGXX_COMPILER}" CACHE STRING "C++ compiler")
    else()
        set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler")
    endif()
    
    # Try to use LLVM linker
    find_llvm_tool(LLD_LINKER "ld64.lld")
    if(NOT LLD_LINKER)
        find_llvm_tool(LLD_LINKER "lld")
    endif()
    
    if(LLD_LINKER)
        set(CMAKE_LINKER "${LLD_LINKER}" CACHE STRING "Linker")
    endif()
    
    # Use LLVM archiver
    find_llvm_tool(LLVM_AR "llvm-ar")
    if(LLVM_AR)
        set(CMAKE_AR "${LLVM_AR}" CACHE STRING "Archiver")
    endif()
    
    find_llvm_tool(LLVM_RANLIB "llvm-ranlib")
    if(LLVM_RANLIB)
        set(CMAKE_RANLIB "${LLVM_RANLIB}" CACHE STRING "Ranlib")
    endif()
    
    message(STATUS "LLVM Advanced: Using Clang on macOS")
    
else()
    # Linux and other Unix-like systems
    find_llvm_tool(CLANG_COMPILER "clang")
    find_llvm_tool(CLANGXX_COMPILER "clang++")
    
    if(CLANG_COMPILER)
        set(CMAKE_C_COMPILER "${CLANG_COMPILER}" CACHE STRING "C compiler")
    else()
        set(CMAKE_C_COMPILER "clang" CACHE STRING "C compiler")
    endif()
    
    if(CLANGXX_COMPILER)
        set(CMAKE_CXX_COMPILER "${CLANGXX_COMPILER}" CACHE STRING "C++ compiler")
    else()
        set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "C++ compiler")
    endif()
    
    # Use LLD linker for faster linking
    find_llvm_tool(LLD_LINKER "ld.lld")
    if(NOT LLD_LINKER)
        find_llvm_tool(LLD_LINKER "lld")
    endif()
    
    if(LLD_LINKER)
        set(CMAKE_LINKER "${LLD_LINKER}" CACHE STRING "Linker")
        set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld" CACHE STRING "Executable linker flags")
        set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld" CACHE STRING "Shared linker flags")
        set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld" CACHE STRING "Module linker flags")
        message(STATUS "LLVM Advanced: Using LLD linker")
    endif()
    
    # Use LLVM tools
    find_llvm_tool(LLVM_AR "llvm-ar")
    if(LLVM_AR)
        set(CMAKE_AR "${LLVM_AR}" CACHE STRING "Archiver")
    endif()
    
    find_llvm_tool(LLVM_RANLIB "llvm-ranlib")
    if(LLVM_RANLIB)
        set(CMAKE_RANLIB "${LLVM_RANLIB}" CACHE STRING "Ranlib")
    endif()
    
    find_llvm_tool(LLVM_NM "llvm-nm")
    if(LLVM_NM)
        set(CMAKE_NM "${LLVM_NM}" CACHE STRING "nm tool")
    endif()
    
    find_llvm_tool(LLVM_OBJDUMP "llvm-objdump")
    if(LLVM_OBJDUMP)
        set(CMAKE_OBJDUMP "${LLVM_OBJDUMP}" CACHE STRING "objdump tool")
    endif()
    
    message(STATUS "LLVM Advanced: Using Clang on Linux")
endif()

# Optimized flags for different build types
if(NOT WIN32)
    set(CMAKE_C_FLAGS_DEBUG "-g -O0" CACHE STRING "C debug flags")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0" CACHE STRING "C++ debug flags")
    
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "C release flags")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE STRING "C++ release flags")
    
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG" CACHE STRING "C release with debug info flags")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG" CACHE STRING "C++ release with debug info flags")
endif()

# Advanced optimization options
option(LLVM_ENABLE_LTO "Enable Link Time Optimization" OFF)
option(LLVM_ENABLE_PGO "Enable Profile Guided Optimization" OFF)
option(LLVM_USE_LIBCXX "Use LLVM's libc++ standard library" OFF)
option(LLVM_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers in Debug builds" OFF)
option(LLVM_ENABLE_POLLY "Enable Polly loop optimizer" OFF)

# Link Time Optimization
if(LLVM_ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    if(NOT WIN32)
        # Use ThinLTO for faster incremental builds
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto=thin")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=thin")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto=thin")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto=thin")
    endif()
    message(STATUS "LLVM Advanced: ThinLTO enabled")
endif()

# Profile Guided Optimization
if(LLVM_ENABLE_PGO)
    if(NOT WIN32)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate")
        message(STATUS "LLVM Advanced: PGO instrumentation enabled (remember to run with profdata)")
    endif()
endif()

# Use libc++ (LLVM's C++ standard library)
if(LLVM_USE_LIBCXX AND NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
    message(STATUS "LLVM Advanced: Using libc++")
endif()

# Sanitizers for debug builds
if(LLVM_ENABLE_SANITIZERS AND NOT WIN32)
    set(SANITIZER_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    message(STATUS "LLVM Advanced: Sanitizers enabled for Debug builds")
endif()

# Polly loop optimizer
if(LLVM_ENABLE_POLLY AND NOT WIN32)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mllvm -polly")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mllvm -polly")
    message(STATUS "LLVM Advanced: Polly optimizer enabled")
endif()

# Verify compilers exist
find_program(C_COMPILER_CHECK ${CMAKE_C_COMPILER})
find_program(CXX_COMPILER_CHECK ${CMAKE_CXX_COMPILER})

if(NOT C_COMPILER_CHECK)
    message(FATAL_ERROR "LLVM Toolchain: C compiler '${CMAKE_C_COMPILER}' not found")
endif()

if(NOT CXX_COMPILER_CHECK)
    message(FATAL_ERROR "LLVM Toolchain: C++ compiler '${CMAKE_CXX_COMPILER}' not found")
endif()

# Print configuration summary
message(STATUS "========== LLVM Toolchain Configuration ==========")
message(STATUS "C Compiler:       ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler:     ${CMAKE_CXX_COMPILER}")
if(CMAKE_LINKER)
    message(STATUS "Linker:           ${CMAKE_LINKER}")
endif()
if(CMAKE_AR)
    message(STATUS "Archiver:         ${CMAKE_AR}")
endif()
message(STATUS "LTO:              ${LLVM_ENABLE_LTO}")
message(STATUS "PGO:              ${LLVM_ENABLE_PGO}")
message(STATUS "libc++:           ${LLVM_USE_LIBCXX}")
message(STATUS "Sanitizers:       ${LLVM_ENABLE_SANITIZERS}")
message(STATUS "Polly:            ${LLVM_ENABLE_POLLY}")
message(STATUS "=================================================")

set(CMAKE_TOOLCHAIN_FILE_PROCESSED TRUE)