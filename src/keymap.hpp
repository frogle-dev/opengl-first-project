#pragma once

#include <GLFW/glfw3.h>

#include "../lib/json.hpp"
#include <fstream>

#include <unordered_map>
#include <vector>
#include <iostream>


struct ActionState 
{
    bool pressed = false;
    bool justPressed = false;
    bool released = false;
};

std::unordered_map<std::string, std::vector<int>> bindings;
std::unordered_map<std::string, ActionState> actions;

void keysRefresh() 
{
    for (auto& [name, state] : actions) 
    {
        state.justPressed = false;
        state.released = false;
    }
}

void mapKey(const std::string& actionName, int key) 
{
    bindings[actionName].push_back(key);
}


bool isActionPressed(const std::string& actionName) 
{
    return actions[actionName].pressed;
}

bool isActionJustPressed(const std::string& actionName) 
{
    return actions[actionName].justPressed;
}

bool isActionReleased(const std::string& actionName) 
{
    return actions[actionName].released;
}

// called by glfw key callback thing
void processKeyEvent(int key, int action) 
{
    for (auto& [actionName, keys] : bindings) 
    {
        for (int boundKey : keys)
        {
            if (boundKey == key) 
            {
                auto& state = actions[actionName];

                if (action == GLFW_PRESS) 
                {
                    if (!state.pressed)
                        state.justPressed = true;
                    state.pressed = true;
                } 
                else if (action == GLFW_RELEASE)
                {
                    if (state.pressed)
                        state.released = true;
                    state.pressed = false;
                }
            }
        }
    }
}

void readConfigKeymaps()
{
    std::ifstream keymapJson("../game_config/keymaps.json");

    nlohmann::json data = nlohmann::json::parse(keymapJson);
}
