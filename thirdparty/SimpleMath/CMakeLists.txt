
set(SIMPLEMATH_FILES SimpleMath.cpp SimpleMath.h SimpleMath.inl)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)

add_library(SimpleMath STATIC EXCLUDE_FROM_ALL ${SIMPLEMATH_FILES})


target_include_directories(SimpleMath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(SimpleMath PUBLIC SAL_H DirectXMath)

#suppress all warnings for thirdparty
target_compile_options(SimpleMath PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W0>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)

target_compile_definitions(SimpleMath PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)

if (MSVC)
    target_compile_definitions(SimpleMath PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif ()

RegisterThirdparty(SimpleMath)