
set(SIMPLEMATH_FILES SimpleMath.cpp SimpleMath.h SimpleMath.inl)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)

if (${STATIC_CPP})
    add_library(SimpleMath STATIC EXCLUDE_FROM_ALL ${SIMPLEMATH_FILES})
else ()
    add_library(SimpleMath SHARED EXCLUDE_FROM_ALL ${SIMPLEMATH_FILES})
endif ()

target_include_directories(SimpleMath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(SimpleMath PUBLIC SAL_H DirectXMath)

#suppress all warnings for thirdparty
target_compile_options(SimpleMath PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)

RegisterThirdparty(SimpleMath)