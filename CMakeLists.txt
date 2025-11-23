cmake_minimum_required(VERSION 3.10.0)
project(first_opengl VERSION 0.1.0 LANGUAGES C CXX)
cmake_policy(SET CMP0072 NEW) #make sure "new" opengl is used

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

add_library(imgui STATIC
    # main imgui stuff
    lib/imgui/imgui.cpp
    lib/imgui/imgui.h
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui_demo.cpp
    # glfw + opengl backends
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_glfw.h
    lib/imgui/backends/imgui_impl_opengl3.cpp
    lib/imgui/backends/imgui_impl_opengl3.h
)

target_link_libraries(imgui
    PUBLIC
    glfw
    OpenGL::GL
)
target_include_directories(imgui PUBLIC lib/imgui/ lib/imgui/backends/)


add_executable(first_opengl 
src/main.cpp
lib/glad.c
src/vertex.glsl
src/fragment.glsl
)

target_link_libraries(first_opengl 
glfw
OpenGL::GL
imgui
X11
)