#pragma once

#include "../lib/glad.h"
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

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

    void GenerateTextureArray(int _texWidth, int _texHeight, int _maxTextures)
    {
        maxTexLayers = _maxTextures;
        texWidth = _texWidth;
        texHeight = _texHeight;

        glGenTextures(1, &texArrayID);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texArrayID);
    
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
        glTexStorage3D(GL_TEXTURE_2D_ARRAY,
            1,
            GL_RGBA,
            texWidth, texHeight,
            maxTexLayers
        );
    }
    
    int LoadTexture(const char* absolutePath)
    {
        if (nextTexLayer >= maxTexLayers)
        {
            std::cout << "(Texture Manager): Texture Array Error: cant have more textures than max specified for texture array" << std::endl;
            return -1;
        }

        
        int width, height, numChannels;
        unsigned char *data = stbi_load(absolutePath, &width, &height, &numChannels, 0);
        if (!data)
        {
            std::cout << "(Texture Manager): Texture Error: Failed to load texture" << std::endl;
            return -1;
        }
        
        if (width != texWidth, height != texHeight)
        {
            std::cout << "(Texture Manager): Texture Error: width and height do not match texture array width and height";
            return -1;
        }
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texArrayID);

        glTextureSubImage3D(GL_TEXTURE_2D_ARRAY,
            0, 
            0, 0, nextTexLayer,
            texWidth, texHeight, 1,
            GL_RGBA, GL_UNSIGNED_BYTE,
            data
        );
        
        stbi_image_free(data);

        int texLayerUsed = nextTexLayer; // set to the current layer used for the texture just initalized
        nextTexLayer++;

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

        return texLayerUsed; // return the layer just used in order to be used in shaders
    }

    int LoadTexture(std::string path, std::string directoryPath)
    {
        if (nextTexLayer >= maxTexLayers)
        {
            std::cout << "(Texture Manager): Texture Array Error: cant have more textures than max specified for texture array" << std::endl;
            return -1;
        }


        int width, height, numChannels;
        unsigned char *data = stbi_load((directoryPath + "/" + path).c_str(), &width, &height, &numChannels, 0);
        if (!data)
        {
            std::cout << "(Texture Manager): Texture Error: Failed to load texture" << std::endl;
            return -1;
        }
        
        if (width != texWidth, height != texHeight)
        {
            std::cout << "(Texture Manager): Texture Error: width and height do not match texture array width and height" << std::endl;
        }
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, texArrayID);

        glTextureSubImage3D(GL_TEXTURE_2D_ARRAY,
            0, 
            0, 0, nextTexLayer,
            texWidth, texHeight, 1,
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
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

private:
    // private constructor so other instances cant be made
    TextureManager() {}

    GLuint texArrayID;
    int texWidth;
    int texHeight;

    int maxTexLayers;
    int nextTexLayer;
};
