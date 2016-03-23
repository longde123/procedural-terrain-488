#pragma once

#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

// Shader program with transform feedback.
// i.e. store rendered vertices in a VBO instead of rasterizing.
class TransformProgram : public ShaderProgram
{
public:
    TransformProgram(const GLchar** varyings, int count);
    virtual ~TransformProgram() {}

    virtual void link();

private:
    const GLchar** varyings;
    int count;
};
