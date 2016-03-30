#pragma once

#include <string>

#include "cs488-framework/OpenGLImport.hpp"

class Texture
{
public:
    Texture(std::string path, GLenum binding);

    void init();
    void rebind();
    void reload(std::string newpath);

    GLuint texture;
private:
    std::string path;

    int width;
    int height;

    GLenum binding;
};
