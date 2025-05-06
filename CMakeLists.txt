cmake_minimum_required(VERSION 3.20)
project(stx_secure_transfer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific warning flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

# Find OpenSSL (via vcpkg or system)
find_package(OpenSSL REQUIRED)

add_subdirectory(common)
add_subdirectory(stx-send)
add_subdirectory(stx-recv)


add_executable(crypto_tests tests/crypto_tests.cpp)

target_link_libraries(crypto_tests PRIVATE stx_common OpenSSL::SSL OpenSSL::Crypto)
target_include_directories(crypto_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/common)