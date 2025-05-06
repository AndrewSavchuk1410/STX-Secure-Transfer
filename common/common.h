#pragma once

#include <cstdint>
#include <string>
#include <iostream>

namespace stx {

constexpr size_t BLOCK_SIZE = 4096;       
constexpr size_t MAX_FILENAME_LEN = 256;

enum ExitCode {
    SUCCESS = 0,
    NETWORK_ERROR = 1,
    AUTH_ERROR = 2,
    IO_ERROR = 3
};

inline void log_error(const std::string& msg) noexcept {
    (void)msg; 
    std::cerr << "[ERROR] " << msg << "\n";
}


inline void log_debug(const std::string& msg) {
    (void)msg;
#ifdef DEBUG
    std::cerr << "[DEBUG] " << msg << std::endl;
#endif
}

}
