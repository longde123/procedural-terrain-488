#include "plane.hpp"

#include <algorithm>

#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;

Plane::Plane(float size)
: size(size)
{
}

void Plane::init(ShaderProgram& shaderProgram, mat4 transform)
{
    float half_size = size / 2;

    vec3 plane_vertices[] = {
        vec3(transform * vec4(half_size, 0, half_size, 1)),
        vec3(transform * vec4(half_size, 0, -half_size, 1)),
        vec3(transform * vec4(-half_size, 0, -half_size, 1)),

        vec3(transform * vec4(-half_size, 0, -half_size, 1)),
        vec3(transform * vec4(-half_size, 0, half_size, 1)),
        vec3(transform * vec4(half_size, 0, half_size, 1))
    };

    initFromVertices(shaderProgram, (float*)&plane_vertices, 3 * 3 * 2);
}
