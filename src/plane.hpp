#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "geometry.hpp"

class Plane : public Geometry
{
public:
    Plane(float dim);

    // Initialize a grid of size dim x dim on the x-z plane.
    void init(ShaderProgram& shaderProgram, glm::mat4 transform = glm::mat4());

private:
    float size;
};
