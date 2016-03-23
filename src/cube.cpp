#include "cube.hpp"

#include <algorithm>

using namespace glm;

Cube::Cube(float size)
: size(size)
{
}

void Cube::init(ShaderProgram& shaderProgram)
{
    vec3 cubeVertices[] = {
        // Front of cube
        vec3(size, size, size),
        vec3(size, 0.0, size),
        vec3(0.0, 0.0, size),
        vec3(0.0, size, size),
        // Back of cube
        vec3(size, size, 0.0),
        vec3(size, 0.0, 0.0),
        vec3(0.0, 0.0, 0.0),
        vec3(0.0, size, 0.0),
    };

    indices = {
        // Face (front)
        0, 2, 1,
        0, 3, 2,
        // Face (back)
        4, 5, 6,
        4, 6, 7,
        // Face (top)
        0, 4, 7,
        0, 7, 3,
        // Face (bottom)
        1, 2, 6,
        1, 6, 5,
        // Face (left)
        2, 3, 6,
        3, 7, 6,
        // Face (right)
        0, 1, 5,
        0, 5, 4,
    };

    initFromVertices(shaderProgram, (float*)&cubeVertices, 3 * 4 * 2);
}
