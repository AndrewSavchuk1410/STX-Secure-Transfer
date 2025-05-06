add_library(stx_common
    common.h
    crypto.cpp crypto.h
    socket.cpp socket.h
    transfer.cpp transfer.h
)

target_include_directories(stx_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(stx_common PUBLIC OpenSSL::SSL OpenSSL::Crypto)
