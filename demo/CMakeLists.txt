cmake_minimum_required(VERSION 3.20)
project(demo)

set(CMAKE_CXX_FLAGS "-mthumb-interwork -mthumb -O2 -Wall -fno-exceptions")
set(CMAKE_EXE_LINKER_FLAGS "-mthumb-interwork -mthumb -specs=gba.specs")

add_executable(demo demo.cpp)
target_link_libraries(demo gba)

target_include_directories(demo PRIVATE "${DEVKITPRO}/libgba/include")
target_link_directories(demo PRIVATE "${DEVKITPRO}/libgba/lib/")

add_custom_command(
	OUTPUT demo.gba
	COMMAND ${DEVKITARM}/bin/arm-none-eabi-objcopy -v -O binary $<TARGET_FILE:demo> demo.gba
	DEPENDS demo
	BYPRODUCTS demo.gba
	)
add_custom_target(demo_gba
	DEPENDS demo.gba)
