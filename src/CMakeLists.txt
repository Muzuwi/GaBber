if(UNIX)
    set(GABBER_CXX_FLAGS
        -flto
        -fno-exceptions
        -fstrict-aliasing -Wstrict-aliasing=1
        -Wall -Wextra -Wdisabled-optimization
        -Werror=reorder -Werror=return-type -Werror=strict-aliasing
        -Wno-packed-bitfield-compat
        -std=c++20
        $<$<CONFIG:Debug>:-ggdb -g>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:RelWithDebInfo>:-O2 -ggdb -g>
        $<$<CONFIG:RelWithSanitizer>:-O2 -ggdb -g -fsanitize=address -fsanitize=undefined>
        )

    set(GABBER_LINK_LIBRARIES
        OpenGL::OpenGL
        GLEW::GLEW
        SDL2::SDL2
        ImGui
        fmt::fmt
        ${SOXR}
        )

    set(GABBER_INCLUDE_DIRS
        ${SOXR_HEADERS}
        .
        )
else()
    message(FATAL_ERROR "Compilation on non-Linux platforms currently unsupported")
endif()

file(GLOB_RECURSE GABBER_SOURCES *.cpp)
add_executable(GaBber ${GABBER_SOURCES})
target_compile_options(GaBber PRIVATE ${GABBER_CXX_FLAGS})
target_link_libraries(GaBber PRIVATE ${GABBER_LINK_LIBRARIES})
target_include_directories(GaBber PRIVATE ${GABBER_INCLUDE_DIRS})
