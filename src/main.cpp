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
#include "textures.hpp"
#include "models.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>


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

    // imgui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    auto& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    const ImVec4 bgColor = ImVec4(0.1, 0.1, 0.1, 0.5);
    colors[ImGuiCol_WindowBg] = bgColor;
    colors[ImGuiCol_ChildBg] = bgColor;
    colors[ImGuiCol_TitleBg] = bgColor;

    
    // setup keybinds from json file
    std::string cur_actionName;
    std::vector<int> cur_keycodes;
    reloadConfigKeymaps();

    
    // rendering and shader stuff
    Shader objectShader("/home/jonah/Programming/Opengl/opengl-first-project/src/vertex.glsl", "/home/jonah/Programming/Opengl/opengl-first-project/src/fragment.glsl");
    Shader lightSourceShader("/home/jonah/Programming/Opengl/opengl-first-project/src/light_source_vertex.glsl", "/home/jonah/Programming/Opengl/opengl-first-project/src/light_source_fragment.glsl");


    // unsigned int VAO; //vertex array object
    // glGenVertexArrays(1, &VAO);

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
    
    // glBindVertexArray(VAO);
    // setVertAttributes();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBindVertexArray(lightVAO);
    setLightSourceVertAttribs();

    glBindBuffer(GL_ARRAY_BUFFER, 0); // unbinding VBO
    glBindVertexArray(0); // unbinding VAO

    // enable depth testing and face culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // texture stuff
    TextureManager::Get().GenerateTextureArray(4096, 4096, 10);
    
    // int texLayerGold = TextureManager::Get().LoadTexture("../images/gold_ore_stone.png");
    // int texLayerGold_spec = TextureManager::Get().LoadTexture("../images/gold_ore_stone_specular.png");

    GLuint texArrayID = TextureManager::Get().GetTexArrayID();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texArrayID);

    stbi_set_flip_vertically_on_load(true);

    objectShader.use();
    objectShader.setInt("texArray", 0); // tex array should use tex unit 0

    // objectShader.setInt("diffuseLayerCount", 1);
    // objectShader.setInt("specularLayerCount", 1);
    // objectShader.setInt("emissionLayerCount", 1);
    // objectShader.setInt("material.diffuseTexLayer[0]", texLayerGold);
    // objectShader.setInt("material.specularTexLayer[0]", texLayerGold_spec);
    // objectShader.setInt("material.emissionTexLayer[0]", texLayerGold_spec);
    objectShader.setFloat("material.emissionStrength", 1.0f);
    objectShader.setVec3("material.specular", glm::vec3(0.2f));
    objectShader.setFloat("material.shininess", 32.0f);

    // model loading
    Model backpack("../models/backpack/backpack.obj");

    TextureManager::Get().GenerateMipmaps(); // generate texture array mipmaps once all textures have been loaded in

    
    glm::vec3 pointLightPos[] = {
        glm::vec3( 0.7f,  5.0f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };
    

    // imgui stuff
    bool demoWindow = false;
    bool changingKeybind = false;

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
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Window"))
            {
                if (ImGui::MenuItem("Demo Window")) 
                {
                    demoWindow = !demoWindow;
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (demoWindow)
        {
            ImGui::ShowDemoWindow();
        }
        
        // info panel for fps/ms per frame and keymaps
        ImGui::Begin("Info", NULL, ImGuiWindowFlags_None);
        ImGui::Text("ms per frame: %f", msPerFrame);
        ImGui::Text("fps: %i", fps);
        
        ImGui::Separator();
        ImGui::Text("Keymaps");
        ImGui::BeginChild("Keymaps");
        auto& bindings = getConfigKeymaps();
        for (auto& [actionName, keycodes] : bindings)
        {
            if (ImGui::Button(actionName.c_str()))
            {
                cur_actionName = actionName;
                changingKeybind = true;
                ImGui::OpenPopup("Change Keymap?");
            }
        }
        if (changingKeybind)
        {
            cur_keycodes = bindings[cur_actionName];
        }

        if(ImGui::BeginPopupModal("Change Keymap?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Changing action: %s", cur_actionName.c_str());
            ImGui::Text("Press a key, then click 'add' or 'change' to assign the currently pressed key to that slot");
            
            ImGui::Separator();
            ImGui::Text("Press any key: %i", currentKeyPress);

            if(ImGui::BeginListBox("Current assigned keycodes"))
            {
                for (int i = 0; i < cur_keycodes.size(); i++)
                {
                    int key = cur_keycodes[i];
                    ImGui::Text("%i", key);
                    ImGui::SameLine();

                    ImGui::PushID(key + i);
                    if (ImGui::Button("Change"))
                    {
                        setConfigKeymap(cur_actionName, false, currentKeyPress, i);
                        reloadConfigKeymaps();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Remove"))
                    {
                        removeConfigKeymap(cur_actionName, i);
                        reloadConfigKeymaps();
                    }
                    ImGui::PopID();
                }
                if(ImGui::Button("Add"))
                {
                    setConfigKeymap(cur_actionName, true, currentKeyPress);
                    reloadConfigKeymaps();
                }
                ImGui::EndListBox();
            }

            if (ImGui::Button("Close")) 
            { 
                ImGui::CloseCurrentPopup(); 
                changingKeybind = false;
            }
            ImGui::EndPopup();
        }
        ImGui::EndChild();
        ImGui::End();
        
        // inspector window for modifying properties of objects
        ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_None);
        ImGui::Text("Entity: ");
        ImGui::End();
        

        // game loop stuff
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
        objectShader.setVec3("viewPos", camera.position);

        // directional light
        objectShader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        objectShader.setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        objectShader.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
        objectShader.setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));

        // point light
        objectShader.setVec3("pointLights[0].position", pointLightPos[0]);
        objectShader.setVec3("pointLights[0].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        objectShader.setVec3("pointLights[0].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        objectShader.setVec3("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        objectShader.setFloat("pointLights[0].constant", 1.0f);
        objectShader.setFloat("pointLights[0].linear", 0.09f);
        objectShader.setFloat("pointLights[0].quadratic", 0.032f);

        // spot light
        objectShader.setVec3("spotLight.position", camera.position);
        objectShader.setVec3("spotLight.direction", camera.front);
        objectShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        objectShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
        objectShader.setVec3("spotLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        objectShader.setVec3("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        objectShader.setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        objectShader.setFloat("spotLight.constant", 1.0f);
        objectShader.setFloat("spotLight.linear", 0.14f);
        objectShader.setFloat("spotLight.quadratic", 0.07f);

        
        glm::mat4 model = glm::mat4(1.0f);
        // for (unsigned int x = 0; x < 16; x++)
        // {
        //     for (unsigned int y = 0; y < 16; y++)
        //     {
        //         for (unsigned int z = 0; z < 16; z++)
        //         {
        //             model = glm::mat4(1.0f);
        //             model = glm::translate(model, glm::vec3(1.0f * x, -1.0f * y, -1.0f * z));
                    
        //             objectShader.setMat4("model", model);
                    
        //             glBindVertexArray(VAO);
        //             glDrawArrays(GL_TRIANGLES, 0, 36);
        //         }
        //     }
        // }

        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        objectShader.setMat4("model", model);
        backpack.Draw(objectShader);
        
        
        glBindVertexArray(lightVAO);
        lightSourceShader.use();
        lightSourceShader.setMat4("projection", projection);
        lightSourceShader.setMat4("view", view);
        
        // drawing all light object cube thingies
        model = glm::mat4(1.0f);
        model = glm::translate(model, pointLightPos[0]);
        lightSourceShader.setMat4("model", model);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        

        // imgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // end of process life
    // glDeleteVertexArrays(1, &VAO);
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

    if (focus)
    {
        float xOffset = xpos - lastMouseX;
        float yOffset = lastMouseY - ypos;
        lastMouseX = xpos;
        lastMouseY = ypos;
    
        camera.ProcessMouseMovement(xOffset, yOffset);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    processKeyEvent(key, action);
}