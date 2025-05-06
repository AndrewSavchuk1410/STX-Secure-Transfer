add_executable(stx-send main.cpp)
target_include_directories(stx-send PRIVATE ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(stx-send PRIVATE stx_common)
