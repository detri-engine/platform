cmake_minimum_required(VERSION 4.1)
project(platform)

set(CMAKE_CXX_STANDARD 26)

add_library(detri_platform STATIC)
add_library(detri::platform ALIAS detri_platform)

include(FetchContent)

FetchContent_Declare(detri_except
    GIT_REPOSITORY https://github.com/detri-engine/except.git
    GIT_TAG main
)
FetchContent_Declare(mio
    GIT_REPOSITORY https://github.com/vimpunk/mio
)
FetchContent_MakeAvailable(detri_except mio)

if (MSVC)
    target_compile_definitions(detri_platform PUBLIC
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

target_sources(detri_platform PUBLIC FILE_SET HEADERS
    BASE_DIRS src
    FILES src/detri/window.hpp src/detri/platform_event.hpp src/detri/platform.hpp src/detri/platform_exceptions.hpp
)

if (MSVC)
    target_sources(detri_platform PRIVATE src/detri/window_win32.cpp src/detri/platform_win32.cpp)
endif()

target_link_libraries(detri_platform PRIVATE detri::except mio::mio)

add_executable(window_test src/test/window_integration_test.cpp)
target_link_libraries(window_test PRIVATE detri::platform detri::except)
