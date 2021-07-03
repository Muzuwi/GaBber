cmake_minimum_required(VERSION 3.14)


add_library(gl3w
    gl3w.c
)

add_library(ImGui
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_sdl.cpp
    imgui/imgui_impl_opengl3.cpp
)

target_compile_definitions(ImGui PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GL3W
)