#include "texture.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/glm.hpp>
#include "soil/soil.h"
#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

Texture::Texture(string path, GLenum binding)
: path(path)
, width(0)
, height(0)
, texture(-1)
, binding(binding)
{
    ifstream file(path);
    if (!file) {
        stringstream errorMessage;
        errorMessage << "Texture file not found: " << path;
        throw errorMessage.str();
    }
}

void Texture::init()
{
    glGenTextures(1, &texture);
    glActiveTexture(binding);
    glBindTexture(GL_TEXTURE_2D, texture);
    {
        // Note: first pixel is loaded as top-left corner whereas OpenGL expects lower-left
        // corner. This is not handled here so the shader code will need to be aware.
        unsigned char* image = SOIL_load_image(path.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
        assert(width > 0 && height > 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        SOIL_free_image_data(image);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // TODO: Compare GL_NEAREST_MIPMAP_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

	CHECK_GL_ERRORS;
}

void Texture::reload(string new_path)
{
    path = new_path;

    init();
    ifstream file(path);
    if (!file) {
        stringstream errorMessage;
        errorMessage << "Texture file not found: " << path;
        throw errorMessage.str();
    }
}

void Texture::rebind()
{
    assert(width > 0 && height > 0);
    glActiveTexture(binding);
    glBindTexture(GL_TEXTURE_2D, texture);
}
