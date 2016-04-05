#pragma once

#include "indexed_geometry.hpp"

class Cube : public IndexedGeometry
{
public:
    Cube(float size);

    // Initialize a cube with corners (0, 0, 0) and (size, size, size)
    void init(ShaderProgram& shaderProgram);

private:
    float size;
};
