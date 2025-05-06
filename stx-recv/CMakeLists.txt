add_executable(stx-recv main.cpp)
target_include_directories(stx-recv PRIVATE ${PROJECT_SOURCE_DIR}/common)
target_link_libraries(stx-recv PRIVATE stx_common)
