cmake_minimum_required(VERSION 3.14)

set(DEPEND_CXX_FLAGS -g -O2)


add_library(gl3w
    gl3w.c
)
target_compile_options(gl3w PRIVATE ${DEPEND_CXX_FLAGS})

add_library(ImGui
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_impl_sdl.cpp
    imgui/imgui_impl_opengl3.cpp
)
target_compile_options(ImGui PRIVATE ${DEPEND_CXX_FLAGS})
target_compile_definitions(ImGui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GL3W)
