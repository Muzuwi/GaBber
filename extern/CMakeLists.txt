cmake_minimum_required(VERSION 3.14)

# External library imports
add_subdirectory(catch2/)

add_library(ImGui STATIC
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    )

set(DEPEND_CXX_FLAGS -g -O2)
target_compile_options(ImGui PRIVATE ${DEPEND_CXX_FLAGS})
target_compile_definitions(ImGui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_include_directories(ImGui
    PRIVATE ${SDL2_INCLUDE_DIRS}
    PUBLIC ${CMAKE_SOURCE_DIR}/extern/imgui/ ${CMAKE_SOURCE_DIR}/extern/imgui/backends/ ${CMAKE_SOURCE_DIR}/extern/imgui_club/imgui_memory_editor/
    )

if (UNIX)
    target_link_libraries(ImGui
        PUBLIC dl
        )
endif ()
