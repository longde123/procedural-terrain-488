#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "geometry.hpp"

class Grid : public Geometry
{
public:
    // Size of the grid in number of grid points.
    // So a 5x5 resolution => length of 4 units.
    Grid(size_t resolution);

    // Initialize a grid of size dim x dim on the x-z plane.
    void init(ShaderProgram& shaderProgram, glm::mat4 transform = glm::mat4());

private:
    size_t resolution;
};
