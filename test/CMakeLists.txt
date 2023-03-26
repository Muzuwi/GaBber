# This assumes that Catch2 was already found by CMake

add_executable(GaBberTests
    src/main.cpp)
target_link_libraries(GaBberTests PRIVATE
    Catch2::Catch2)
