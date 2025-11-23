#include "../lib/glad.h"
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "../lib/imgui/imgui.h"
#include "../lib/imgui/backends/imgui_impl_glfw.h"
#include "../lib/imgui/backends/imgui_impl_opengl3.h"

#include "shader.hpp"
#include "cube.hpp"
#include "keymap.hpp"
#include "helpers.hpp"
#include "camera.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void update(GLFWwindow* window);


const unsigned int initWidth = 1280, initHeight = 720;
unsigned int viewWidth = initWidth, viewHeight = initHeight;


float deltaTime = 0.0f;
int fps;
float msPerFrame;

Camera camera;
float lastMouseX = initWidth/2, lastMouseY = initHeight/2;

glm::vec3 feetPos = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 feetVelocity = glm::vec3(0.0f, 0.0f, 0.0f);
const float bodyHeight = 1.0f;



int main()
{
    // initialization
    GLFWwindow* window;
    if (!init(window, initWidth, initHeight))
        return -1;
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // keybinds
    mapKey("forward", GLFW_KEY_W);
    mapKey("backward", GLFW_KEY_S);
    mapKey("left", GLFW_KEY_A);
    mapKey("right", GLFW_KEY_D);
    mapKey("jump", GLFW_KEY_SPACE);
    mapKey("focus", GLFW_KEY_ESCAPE);
    mapKey("wireframe", GLFW_KEY_C);
    readConfigKeymaps();


    // rendering stuff
    unsigned int texture1;
    generateTexture("../images/gold_ore_stone.png", texture1, false);
    unsigned int specularMap;
    generateTexture("../images/gold_ore_stone_specular.png", specularMap, true);
    
    Shader objectShader("/home/jonah/Programming/Opengl/opengl-first-project/src/vertex.glsl", "/home/jonah/Programming/Opengl/opengl-first-project/src/fragment.glsl");
    Shader lightSourceShader("/home/jonah/Programming/Opengl/opengl-first-project/src/light_source_vertex.glsl", "/home/jonah/Programming/Opengl/opengl-first-project/src/light_source_fragment.glsl");


    unsigned int VAO; //vertex array object
    glGenVertexArrays(1, &VAO);

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    
    unsigned int VBO; // vertex byffer object
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_cube), vertices_cube, GL_STATIC_DRAW);

    // unsigned int EBO; // element buffer object
    // glGenBuffers(1, &EBO);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_cube), indices_cube, GL_STATIC_DRAW);
    
    glBindVertexArray(VAO);
    setVertAttributes();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBindVertexArray(lightVAO);
    setLightSourceVertAttribs();

    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbinding VBO
    glBindVertexArray(0); // unbinding VAO

    // enable depth testing and face culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // setting texture samplers in frag shader to corresponding texture units
    objectShader.use();
    objectShader.setInt("cubemap", 0);

    objectShader.setInt("material.diffuse", 0); // tex unit 0
    objectShader.setInt("material.specular", 1); // tex unit 1
    objectShader.setInt("material.emission", 2); // tex unit 2
    objectShader.setFloat("material.emissionStrength", 0.0f);
    objectShader.setVec3("material.specular", glm::vec3(0.2f));
    objectShader.setFloat("material.shininess", 32.0f);

    objectShader.setVec3("light.ambient",  glm::vec3(0.2f));
    objectShader.setVec3("light.diffuse",  glm::vec3(1.0f));
    objectShader.setVec3("light.specular", glm::vec3(1.0f));

    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, specularMap);


    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<int> randr(0, 10);

    // render loop
    float lastFrame = 0.0f;
    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();


        update(window);

        glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear color + depth buffer

        objectShader.use();

        glm::mat4 view = camera.GetViewMatrix();
        objectShader.setMat4("view", view);
        
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(85.0f), (float)viewWidth / viewHeight, 0.1f, 100.0f);
        objectShader.setMat4("projection", projection);
        
        
        // lighting
        glm::vec3 lightPos(sin((float)glfwGetTime()) * 5, 5.0f, cos((float)glfwGetTime()) * 5);
        objectShader.setVec3("light.pos", lightPos);

        objectShader.setVec3("viewPos", camera.position);

        
        glm::mat4 model = glm::mat4(1.0f);
        for (unsigned int x = 0; x < 16; x++)
        {
            for (unsigned int y = 0; y < 16; y++)
            {
                for (unsigned int z = 0; z < 16; z++)
                {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(1.0f * x, -1.0f * y, -1.0f * z));
                    
                    objectShader.setMat4("model", model);
                    
                    glBindVertexArray(VAO);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
        }
        
        lightSourceShader.use();
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        lightSourceShader.setMat4("model", model);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // imgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // end of process life
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    objectShader.deleteProgram();
    lightSourceShader.deleteProgram();

    // imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}


// game loop stuff
bool grounded = false;
bool focus = true;
bool wireframe = false;
void update(GLFWwindow* window)
{
    // fps
    msPerFrame = deltaTime * 1000;
    fps = 1000 / msPerFrame;
    // std::cout << msPerFrame << ", " << fps << std::endl;

    // utility
    if (isActionJustPressed("focus"))
    {
        focus = !focus;

        if (focus)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (isActionJustPressed("wireframe"))
    {
        wireframe = !wireframe;

        if (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }


    // movement
    const float moveSpeed = 5.0f; 
    glm::vec3 moveDir(0.0f);
    feetVelocity = glm::vec3(0.0f, feetVelocity.y, 0.0f);
    if (isActionPressed("forward"))
    {
        moveDir -= camera.straightFront;
    }
    if (isActionPressed("backward"))
    {
        moveDir += camera.straightFront;
    }
    if (isActionPressed("left"))
    {
        moveDir -= camera.right;
    }
    if (isActionPressed("right"))
    {
        moveDir += camera.right;
    }

    if (glm::length(moveDir) > 0.0f)
    {
        moveDir = glm::normalize(moveDir);
        feetVelocity.x = moveDir.x * moveSpeed; 
        feetVelocity.z = moveDir.z * moveSpeed;
    }

    
    const float jumpSpeed = 7.0f; 
    if (isActionPressed("jump") && grounded)
    {
        grounded = false;
        feetVelocity.y = jumpSpeed;
    }

    const float gravity = -9.81f * 2.0f; 
    feetVelocity.y += gravity * deltaTime;

    feetPos += feetVelocity * deltaTime;

    if (feetPos.y <= 1.0f)
    {
        grounded = true;
        feetPos.y = 1.0f;
        feetVelocity.y = 0.0f;
    }
    else
    {
        grounded = false;
    }

    camera.position = glm::vec3(feetPos.x, feetPos.y + bodyHeight, feetPos.z);

    keysRefresh();
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    float aspect = (float)width / height;
    float targetAspect = (float)initWidth / initHeight;

    if (aspect > targetAspect)
    {
        viewHeight = height;
        viewWidth = (int)(height * targetAspect);
    }
    else
    {
        viewWidth = width;
        viewHeight = (int)(width / targetAspect);
    }

    int viewX = (width - viewWidth) / 2;
    int viewY = (height - viewHeight) / 2;

    glViewport(viewX,viewY,viewWidth,viewHeight);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    } // this is so when mouse initially moves, it doesnt make a large jkittery motion to that position

    
    float xOffset = xpos - lastMouseX;
    float yOffset = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    processKeyEvent(key, action);
}