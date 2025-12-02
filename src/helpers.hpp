#pragma once

#include "../lib/glad.h"
#include <GLFW/glfw3.h>

#include <iostream>


bool init(GLFWwindow *&windowID, int width, int height)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    windowID = glfwCreateWindow(width, height, "First OpenGL", NULL, NULL);
    if (windowID == NULL)
    {
        std::cout << "(Initialization): Error: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(windowID);
    
    glfwSetInputMode(windowID, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "(Initialization): Error: Failed to initialize GLAD" << std::endl;
        return false;
    }

    return true;
}

// void setVertAttributes()
// {
//     // position vertex attribute
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
//     glEnableVertexAttribArray(0);
//     // color vertex attribute
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
//     glEnableVertexAttribArray(1);
//     // texture vertex attribute
//     glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));   
//     glEnableVertexAttribArray(2);
//     // normals vertex attribute
//     glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));   
//     glEnableVertexAttribArray(3);
// }

void setLightSourceVertAttribs()
{
    // position vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

