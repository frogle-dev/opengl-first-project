cmake_minimum_required(VERSION 3.10.0)
project(first_opengl VERSION 0.1.0 LANGUAGES C CXX)
cmake_policy(SET CMP0072 NEW) #make sure "new" opengl is used

find_package(OpenGL REQUIRED)

add_executable(first_opengl 
src/main.cpp
lib/glad.c
src/shader.hpp
src/vertex.glsl
src/fragment.glsl
lib/stb_image.h
)

target_link_libraries(first_opengl 
glfw
OpenGL::GL
)