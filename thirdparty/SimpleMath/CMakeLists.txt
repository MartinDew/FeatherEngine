
set(SIMPLEMATH_FILES SimpleMath.h SimpleMath.inl SimpleMath.cpp)

if (${STATIC_CPP})
    add_library(SimpleMath STATIC EXCLUDE_FROM_ALL ${SIMPLEMATH_FILES})
else()
    add_library(SimpleMath SHARED EXCLUDE_FROM_ALL ${SIMPLEMATH_FILES})
endif()

target_include_directories(SimpleMath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

RegisterThirdparty(SimpleMath)