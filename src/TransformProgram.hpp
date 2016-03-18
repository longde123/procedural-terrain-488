#pragma once

#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
// Shader program with transform feedback.
// i.e. store rendered vertices in a VBO instead of rasterizing.
class TransformProgram : public ShaderProgram
{
public:
    TransformProgram();
    virtual ~TransformProgram() {}

    virtual void link();

private:
};
