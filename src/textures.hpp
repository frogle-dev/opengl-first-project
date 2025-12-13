#pragma once

#include "../lib/glad.h"
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

#include "shader.hpp"

#include <iostream>


// singleton
class TextureManager
{
public:
    static TextureManager& Get()
    {
        static TextureManager instance;
        return instance;
    }

    GLuint GetTexArrayID() const 
    {
        return texArrayID; 
    }

    auto& GetTexSubTexResArray() const
    {
        return subTexRes;
    }

    void GenerateTextureArray(int _texWidth, int _texHeight, int _maxTextures)
    {
        maxTexLayers = _maxTextures;
        maxTexWidth = _texWidth;
        maxTexHeight = _texHeight;

        mipLevels = (int)std::floor(std::log2(maxTexWidth)) + 1;

        glGenTextures(1, &texArrayID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texArrayID);
    
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
        glTexStorage3D(GL_TEXTURE_2D_ARRAY,
            mipLevels,
            GL_RGBA8,
            maxTexWidth, maxTexHeight,
            maxTexLayers
        );
    }

    int LoadTexture(std::string path, std::string directoryPath)
    {
        if (nextTexLayer >= maxTexLayers)
        {
            std::cout << "(Texture Manager): Texture Array Error: cant have more textures than max specified for texture array" << std::endl;
            return -1;
        }

        int width, height, numChannels;
        unsigned char *data = stbi_load((directoryPath + "/" + path).c_str(), &width, &height, &numChannels, STBI_rgb_alpha);

        if (!data)
        {
            std::cout << "(Texture Manager): Texture Error: Failed to load texture" << std::endl;
            return -1;
        }
        
        if (width > maxTexWidth || height > maxTexHeight)
        {
            std::cout << "(Texture Manager): Texture Error: width and height are larger than texture array width and height" << std::endl;
            return -1;
        }

        if (width != maxTexWidth || height != maxTexHeight)
        {
            std::cout << "(Texture Manager): Texture Warning: width and height do not match texture array width and height, "
            "texture will still be inserted but will not take up the full resolution." << std::endl;
        }

        subTexRes.push_back(glm::vec2(width, height));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texArrayID);

        glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
            0, 
            0, 0, nextTexLayer,
            width, height, 1,
            GL_RGBA, GL_UNSIGNED_BYTE,
            data
        );

        stbi_image_free(data);

        int texLayerUsed = nextTexLayer; // set to the current layer used for the texture just initalized
        nextTexLayer++;

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        return texLayerUsed; // return the layer just used in order to be used in shaders
    }

    void GenerateMipmaps()
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texArrayID);

        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    void SendSubTexResArrayToShader(Shader &shader)
    {
        for (int i = 0; i < subTexRes.size(); i++)
        {
            shader.setIVec2("subTexRes[" + std::to_string(i) + "]", subTexRes[i].x, subTexRes[i].y);
        }
    }

private:
    // private constructor so other instances cant be made
    TextureManager() : texArrayID(0), maxTexWidth(0), maxTexHeight(0), maxTexLayers(0), nextTexLayer(0), mipLevels(0) {}

    GLuint texArrayID;
    std::vector<glm::vec2> subTexRes;
    int maxTexWidth;
    int maxTexHeight;

    int mipLevels;

    int maxTexLayers;
    int nextTexLayer;
};
