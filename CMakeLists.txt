cmake_minimum_required(VERSION 4.1)
project(platform LANGUAGES CXX)

include(cmake/DetriDependencies.cmake)

option(DETRI_PLATFORM_BUILD_TESTS "Whether to build platform integration tests" ${PROJECT_IS_TOP_LEVEL})

add_library(detri_platform STATIC)
add_library(detri::platform ALIAS detri_platform)

set_target_properties(detri_platform PROPERTIES
    CXX_STANDARD 26
    CXX_EXTENSIONS OFF
)

detri_resolve_dependency(
    TARGET detri::except
    PACKAGE detri_except
    FETCHCONTENT_NAME detri_except
    FETCHCONTENT_DECLARE_ARGS
        GIT_REPOSITORY https://github.com/detri-engine/except.git
        GIT_TAG main
)

detri_resolve_dependency(
    TARGET mio::mio
    PACKAGE mio
    FETCHCONTENT_NAME mio
    FETCHCONTENT_DECLARE_ARGS
        GIT_REPOSITORY https://github.com/vimpunk/mio
)

if (MSVC)
    target_compile_definitions(detri_platform PUBLIC
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

target_sources(detri_platform
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS src
        FILES
            src/detri/window.hpp
            src/detri/platform_event.hpp
            src/detri/platform.hpp
            src/detri/platform_exceptions.hpp
)

if (MSVC)
    target_sources(detri_platform PRIVATE
        src/detri/window_win32.cpp
        src/detri/platform_win32.cpp
    )
endif()

target_link_libraries(detri_platform PRIVATE detri::except mio::mio)

if (PROJECT_IS_TOP_LEVEL AND DETRI_PLATFORM_BUILD_TESTS)
    add_executable(window_test src/test/window_integration_test.cpp)
    target_link_libraries(window_test PRIVATE detri::platform detri::except)
endif()