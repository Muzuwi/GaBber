add_library(disarmv4t INTERFACE)
add_library(disarmv4t::disarmv4t ALIAS disarmv4t)
target_include_directories(disarmv4t INTERFACE ./include/)
